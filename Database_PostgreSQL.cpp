#include "Database_PostgreSQL.h"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "DataFilter.h"
#include "LinesReader.h"
#include "SafeWriter.h"
#include "SqlParsing_Condition.h"
#include "exceptions.h"
#include "util.h"

const std::string Database_PostgreSQL::versionTableName = "versioninfo";

Database_PostgreSQL::Database_PostgreSQL(const Config_Db &config, Temp &temp)
	:
	  config(config),
	  temp(temp)
{
}

std::set<std::string> Database_PostgreSQL::getTables() {
	std::set<std::string> tables = getTables_internal();
	tables.erase(versionTableName);
	return tables;
}

void Database_PostgreSQL::exportTable(const std::string &tableName, const boost::filesystem::path &file) {
	Command command("pg_dump");

	appendConnectionParamsAndVars_pgdump(command);

	command
		.appendArgument("-t")
		.appendArgument(quoteName(tableName))
		.appendArgument("--schema-only")
		.appendRedirectTo(file)
		.execute()
	;
}

void Database_PostgreSQL::exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) {
	TempFile tableDump = temp.createFile();

	Command command("pg_dump");

	appendConnectionParamsAndVars_pgdump(command);

	command
		.appendArgument("-t")
		.appendArgument(quoteName(tableName))
		.appendArgument("--data-only")
		.appendArgument("--column-inserts")
		.appendRedirectTo(tableDump.path())
		.execute()
	;

	sql::condition::Condition condition;
	if (! ignoreWhere.empty()) {
		condition = sql_parsing::parseCondition(ignoreWhere);
	}

	SafeWriter writer(file);
	LinesReader reader(tableDump.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		if (! ignoreWhere.empty()) {
			if (::isInsertAndMatchesWhere(*line, condition)) {
				continue;
			}
		}

		writer.writeLine(*line);
	}
	writer.close();
}

void Database_PostgreSQL::printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) {
	SafeWriter writer(file);
	writer.writeLine("DROP TABLE " + quoteName(tableName) + ";");
	writer.close();
}

void Database_PostgreSQL::import(const boost::filesystem::path &file) {
	import_internal(file);
}

void Database_PostgreSQL::clear() {
	for (const std::string &tableName : getTables_internal()) {
		Command command("psql");

		appendConnectionParamsAndVars_psql(command);

		command
			.appendArgument("--quiet")
			.appendArgument("-c")
			.appendArgument("DROP TABLE IF EXISTS " + quoteName(tableName) + " CASCADE;")
			.execute();
	}
}

std::set<std::string> Database_PostgreSQL::getTableDependencies(const std::string &tableName) {
	TempFile tableDump = temp.createFile();

	Command command("pg_dump");

	appendConnectionParamsAndVars_pgdump(command);

	command
		.appendArgument("-t")
		.appendArgument(quoteName(tableName))
		.appendArgument("--schema-only")
		.appendRedirectTo(tableDump.path())
		.execute()
	;

	std::set<std::string> result;
	LinesReader reader(tableDump.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		std::string::size_type referencesBegin = line->find("REFERENCES");
		if (referencesBegin != std::string::npos) {
			std::string::size_type space = (line->find(' ', referencesBegin + 1));
			if (space != std::string::npos) {
				std::string::size_type parent = (line->find('(', space + 1));
				if (parent != std::string::npos) {
					std::string referencedTableName_maybeInQuotes = line->substr(space + 1, parent - space - 1);
					if ((referencedTableName_maybeInQuotes.size() >= 2) && (referencedTableName_maybeInQuotes.front() == '"') && (referencedTableName_maybeInQuotes.back() == '"')) {
						result.insert(referencedTableName_maybeInQuotes.substr(1, referencedTableName_maybeInQuotes.size() - 2));
					} else {
						result.insert(std::move(referencedTableName_maybeInQuotes));
					}
				}
			}
		}
	}

	return result;
}

bool Database_PostgreSQL::isVersioned() {
	std::set<std::string> tables = getTables_internal();
	return tables.find(versionTableName) != tables.end();
}

void Database_PostgreSQL::makeVersioned() {
	TempFile versionTableCreation = temp.createFile();

	SafeWriter writer(versionTableCreation.path());
	writer.writeLine("CREATE TABLE " + quoteName(versionTableName) + " (");
	writer.writeLine("value integer NOT NULL");
	writer.writeLine(");");
	writer.writeLine("INSERT INTO " + quoteName(versionTableName) + "(value) VALUES(0);");
	writer.close();

	import_internal(versionTableCreation.path());
}

void Database_PostgreSQL::makeNotVersioned() {
	deleteTable_internal(versionTableName);
}

uint32_t Database_PostgreSQL::getVersion() {
	TempFile version = temp.createFile();

	Command command("pg_dump");

	appendConnectionParamsAndVars_pgdump(command);

	command
		.appendArgument("-t")
		.appendArgument(quoteName(versionTableName))
		.appendArgument("--data-only")
		.appendArgument("--inserts")
		.appendRedirectTo(version.path())
		.execute()
	;

	LinesReader reader(version.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		if (line->find("INSERT INTO " + versionTableName + " VALUES (") == 0) {
			std::string::size_type firstPar = (line->find('('));
			std::string::size_type lastPar = (line->rfind(')'));
			if ((firstPar != std::string::npos) && (lastPar != std::string::npos) && (firstPar < lastPar)) {
				std::string numStr = line->substr(firstPar + 1, lastPar - firstPar - 1);
				try {
					return ::stringToNumber(numStr);
				} HANDLE_IGNORE;
			}
		}
	}

	THROW("Unable to get version info.");
}

void Database_PostgreSQL::setVersion(const uint32_t version) {
	Command command("psql");

	appendConnectionParamsAndVars_psql(command);

	command
		.appendArgument("--quiet")
		.appendArgument("-c")
		.appendArgument("UPDATE " + quoteName(versionTableName) + " SET value=" + std::to_string(version))
		.execute()
	;
}

std::set<std::string> Database_PostgreSQL::getTables_internal() {
	TempFile tables = temp.createFile();

	Command command("psql");

	appendConnectionParamsAndVars_psql(command);
	appendFormattingParams_psql(command);

	command
		.appendArgument("--quiet")
		.appendArgument("-c")
		.appendArgument("\\dt")
		.appendRedirectTo(tables.path())
		.execute()
	;

	std::set<std::string> result;
	LinesReader reader(tables.path());
	boost::optional<std::string> line;
	while ((line = reader.readLine())) {
		std::vector<std::string> fields;
		boost::split(fields, *line, boost::is_any_of("|"));

		if (fields.size() == 4) {
			result.insert(fields[1]);
		} else {
			bool canBeNoRelationsFoundMessage = (fields.size() == 1);
			if (! canBeNoRelationsFoundMessage) {
				THROW(boost::format("Invalid line format: %1%") % line);
			}
		}
	}

	return result;
}

void Database_PostgreSQL::import_internal(const boost::filesystem::path &file) {
	Command command("psql");

	appendConnectionParamsAndVars_psql(command);

	command
		.appendArgument("--quiet")
		.appendArgument("-f")
		.appendArgument(file.string())
		.execute()
	;
}

void Database_PostgreSQL::deleteTable_internal(const std::string &tableName) {
	Command command("psql");

	appendConnectionParamsAndVars_psql(command);

	command
		.appendArgument("--quiet")
		.appendArgument("-c")
		.appendArgument("DROP TABLE " + quoteName(tableName) + ";")
		.execute();
}

void Database_PostgreSQL::appendConnectionParamsAndVars_psql(Command &command) {
	command
		.appendArgument("-h")
		.appendArgument(config.host)

		.appendArgument("--username")
		.appendArgument(config.user)

		.appendArgument("-d")
		.appendArgument(config.database)
	;

	setPasswordVariable(command);
}

void Database_PostgreSQL::appendConnectionParamsAndVars_pgdump(Command &command) {
	command
		.appendArgument("--host")
		.appendArgument(config.host)

		.appendArgument("--username")
		.appendArgument(config.user)

		.appendArgument(config.database)
	;

	setPasswordVariable(command);
}

void Database_PostgreSQL::setPasswordVariable(Command &command) {
	command.setVariable("PGPASSWORD", config.password);
}

void Database_PostgreSQL::appendFormattingParams_psql(Command &command) {
	command
		.appendArgument("-A")
		.appendArgument("-t")
	;
}

std::string Database_PostgreSQL::quoteName(const std::string &name) {
	return '"' + name + '"';
}
