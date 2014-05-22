#include "Database_SQLite.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "DataFilter.h"
#include "LinesReader.h"
#include "SafeWriter.h"
#include "SqlParsing_Condition.h"
#include "exceptions.h"
#include "util.h"

const std::string Database_SQLite::versionTableName = "versioninfo";

Database_SQLite::Database_SQLite(const Config_Db &config, Temp &temp)
	:
	  config(config),
	  temp(temp)
{
}

std::set<std::string> Database_SQLite::getTables() {
	std::set<std::string> tables = getTables_internal();
	tables.erase(versionTableName);
	return tables;
}

void Database_SQLite::exportTable(const std::string &tableName, const boost::filesystem::path &file) {
	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument(".schema " + tableName)
		.appendRedirectTo(file)
		.execute()
	;
}

void Database_SQLite::exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) {
	TempFile tableDump = temp.createFile();

	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument(".dump " + tableName)
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
		if (line->find("INSERT INTO") != 0) {
			continue;
		}

		if (! ignoreWhere.empty()) {
			if (::isInsertAndMatchesWhere(*line, condition)) {
				continue;
			}
		}

		writer.writeLine(*line);
	}
	writer.close();
}

void Database_SQLite::printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) {
	SafeWriter writer(file);
	writer.writeLine("DROP TABLE " + tableName + ";");
	writer.close();
}

void Database_SQLite::import(const boost::filesystem::path &file) {
	import_internal(file);
}

void Database_SQLite::deleteTable(const std::string &tableName) {
	deleteTable_internal(tableName);
}

std::set<std::string> Database_SQLite::getTableDependencies(const std::string &tableName) {
	TempFile tableDump = temp.createFile();

	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument(".schema " + tableName)
		.appendRedirectTo(tableDump.path())
		.execute()
	;

	std::set<std::string> result;
	LinesReader reader(tableDump.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		std::string::size_type referencesBegin = line->find("REFERENCES");
		if (referencesBegin != std::string::npos) {
			std::string::size_type firstApos = (line->find('`', referencesBegin));
			if (firstApos != std::string::npos) {
				std::string::size_type secondApos = (line->find('`', firstApos + 1));
				if (secondApos != std::string::npos) {
					std::string referencedTableName = line->substr(firstApos + 1, secondApos - firstApos - 1);
					result.insert(std::move(referencedTableName));
				}
			}
		}
	}

	return result;
}

bool Database_SQLite::isVersioned() {
	std::set<std::string> tables = getTables_internal();
	return tables.find(versionTableName) != tables.end();
}

void Database_SQLite::makeVersioned() {
	TempFile versionTableCreation = temp.createFile();

	SafeWriter writer(versionTableCreation.path());
	writer.writeLine("CREATE TABLE `" + versionTableName + "` (");
	writer.writeLine("`value` int(11) NOT NULL");
	writer.writeLine(");");
	writer.writeLine("INSERT INTO " + versionTableName + "(value) VALUES(0);");
	writer.close();

	import_internal(versionTableCreation.path());
}

void Database_SQLite::makeNotVersioned() {
	deleteTable_internal(versionTableName);
}

uint32_t Database_SQLite::getVersion() {
	TempFile version = temp.createFile();

	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument(".dump " + versionTableName)
		.appendRedirectTo(version.path())
		.execute()
	;

	LinesReader reader(version.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		if (line->find("INSERT INTO \"" + versionTableName + "\" VALUES(") == 0) {
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

void Database_SQLite::setVersion(const uint32_t version) {
	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument("UPDATE " + versionTableName + " SET value=" + std::to_string(version))
		.execute()
	;
}

std::set<std::string> Database_SQLite::getTables_internal() {
	TempFile tables = temp.createFile();

	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument(".tables")
		.appendRedirectTo(tables.path())
		.execute()
	;

	std::set<std::string> result;

	LinesReader reader(tables.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		std::vector<std::string> words;
		boost::split(words, *line, boost::is_any_of("\t "), boost::token_compress_on);
		result.insert(words.begin(), words.end());
	}

	return result;
}

void Database_SQLite::import_internal(const boost::filesystem::path &file) {
	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendRedirectFrom(file)
		.execute()
	;
}

void Database_SQLite::deleteTable_internal(const std::string &tableName) {
	Command command("sqlite3");

	command
		.appendArgument(config.file)
		.appendArgument("DROP TABLE " + tableName + ";") // TODO: disable foreign keys
		.execute()
	;
}
