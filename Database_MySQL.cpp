#include "Database_MySQL.h"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "LinesReader.h"
#include "SafeWriter.h"
#include "exceptions.h"
#include "util.h"

const std::string Database_MySQL::versionTableName = "VersionInfo";

// mysqldump --host localhost -uroot -p.. --compact  --skip-extended-insert --order-by-primary --no-create-info --where "te=..." m_local abc
// mysqldump -h... -u... -p... --no-data --no-create-info --routines > MySQLStoredProcedures.sql &
// --skip-extended-insert

Database_MySQL::Database_MySQL(const Config_Db &config, Temp &temp)
	:
	  config(config),
	  temp(temp)
{
}

std::set<std::string> Database_MySQL::getTables() {
	std::set<std::string> tables = getTables_internal();
	tables.erase(versionTableName);
	return tables;
}

void Database_MySQL::exportTable(const std::string &tableName, const boost::filesystem::path &file) {
	Command command("mysqldump");

	appendConnectionParams(command);

	command
		.appendArgument(tableName)
		.appendArgument("--compact")
		.appendArgument("--no-data")
		.appendRedirectTo(file)
		.execute()
	;
}

void Database_MySQL::exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) {
	Command command("mysqldump");

	appendConnectionParams(command);

	command
		.appendArgument(tableName)
		.appendArgument("--compact")
		.appendArgument("--no-create-info")
		.appendArgument("--skip-extended-insert")
		.appendArgument("--order-by-primary")
	;

	if (! ignoreWhere.empty()) {
		const std::string whereCondition = "NOT(" + ignoreWhere + ")";

		command
			.appendArgument("--where")
			.appendArgument(whereCondition)
		;
	}

	command
		.appendRedirectTo(file)
		.execute()
	;
}

void Database_MySQL::printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) {
	SafeWriter writer(file);
	writer.writeLine("DROP TABLE " + tableName + ";");
	writer.close();
}

void Database_MySQL::import(const boost::filesystem::path &file) {
	import_internal(file);
}

void Database_MySQL::deleteTable(const std::string &tableName) {
	deleteTable_internal(tableName);
}

std::set<std::string> Database_MySQL::getTableDependencies(const std::string &tableName) {
	TempFile tableDump = temp.createFile();

	Command command("mysqldump");

	appendConnectionParams(command);

	command
		.appendArgument(tableName)
		.appendArgument("--skip-comments")
		.appendArgument("--no-data")
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

bool Database_MySQL::isVersioned() {
	std::set<std::string> tables = getTables_internal();
	return tables.find(versionTableName) != tables.end();
}

void Database_MySQL::makeVersioned() {
	TempFile versionTableCreation = temp.createFile();

	SafeWriter writer(versionTableCreation.path());
	writer.writeLine("CREATE TABLE `" + versionTableName + "` (");
	writer.writeLine("`value` int(11) NOT NULL");
	writer.writeLine(") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
	writer.writeLine("INSERT INTO " + versionTableName + "(value) VALUES(0);");
	writer.close();

	import_internal(versionTableCreation.path());
}

void Database_MySQL::makeNotVersioned() {
	deleteTable_internal(versionTableName);
}

uint32_t Database_MySQL::getVersion() {
	TempFile version = temp.createFile();

	Command command("mysqldump");

	appendConnectionParams(command);

	command
		.appendArgument(versionTableName)
		.appendRedirectTo(version.path())
		.execute()
	;

	LinesReader reader(version.path());
	while (boost::optional<std::string> line = reader.readLine()) {
		if (line->find("INSERT INTO `" + versionTableName + "` VALUES (") == 0) {
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

void Database_MySQL::setVersion(const uint32_t version) {
	Command command("mysql");

	appendConnectionParams(command);

	command
		.appendArgument("-e")
		.appendArgument("UPDATE " + versionTableName + " SET value=" + std::to_string(version))
		.execute()
	;
}

std::set<std::string> Database_MySQL::getTables_internal() {
	TempFile tables = temp.createFile();

	Command command("mysqldump");

	appendConnectionParams(command);

	command
		.appendArgument("--compact")
		.appendArgument("--no-data")
		.appendRedirectTo(tables.path())
		.execute()
	;

	std::set<std::string> result;
	LinesReader reader(tables.path());
	boost::optional<std::string> line;
	while ((line = reader.readLine())) {
		if (line->find("CREATE TABLE `") == 0) {
			std::string::size_type firstApos = (line->find('`'));
			std::string::size_type lastApos = (line->rfind('`'));
			if ((firstApos != std::string::npos) && (lastApos != std::string::npos) && (firstApos < lastApos)) {
				result.insert(line->substr(firstApos + 1, lastApos - firstApos - 1));
			}
		}
	}

	return result;
}

void Database_MySQL::import_internal(const boost::filesystem::path &file) {
	Command command("mysql");

	appendConnectionParams(command);

	command
		.appendRedirectFrom(file)
		.execute()
	;
}

void Database_MySQL::deleteTable_internal(const std::string &tableName) {
	Command command("mysql");

	appendConnectionParams(command);

	command
		.appendArgument("-e")
		.appendArgument("SET foreign_key_checks = 0; DROP TABLE " + tableName + "; SET foreign_key_checks = 1;")
		.execute();
}

void Database_MySQL::appendConnectionParams(Command &command) {
	command
		.appendArgument("--host")
		.appendArgument(config.host)

		.appendArgument("-u")
		.appendArgument(config.user)

		.appendArgument("-p" + config.password)

		.appendArgument(config.database)
	;
}
