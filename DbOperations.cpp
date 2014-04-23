#include "DbOperations.h"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "LinesReader.h"
#include "SafeWriter.h"
#include "exceptions.h"
#include "util.h"

const std::string DbOperations::versionTableName = "VersionInfo";

DbOperations::DbOperations(const Config_Db &config, Temp &temp)
	:
	  config(config),
	  temp(temp)
{
}

std::set<std::string> DbOperations::getTables() {
	std::set<std::string> tables = getTables_internal();
	tables.erase(versionTableName);
	return tables;
}

void DbOperations::exportTable(const std::string &tableName, const boost::filesystem::path &file) {
	std::ostringstream command;

	command << "mysqldump";
	appendConnectionParams(command);
	command << ' ' << tableName;
	command
		<< " --compact"
		<< " --no-data"
	;

	command << " > " << file.string();

	executeCommand(command.str());
}

void DbOperations::exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) {
	std::ostringstream command;

	command << "mysqldump";
	appendConnectionParams(command);
	command << ' ' << tableName;
	command
		<< " --compact"
		<< " --no-create-info"
		<< " --skip-extended-insert"
		<< " --order-by-primary"
	;

	if (! ignoreWhere.empty()) {
		std::string ignoreWhereReplaced = ignoreWhere;
		boost::algorithm::replace_all(ignoreWhereReplaced, "'", "\\'");
		//boost::algorithm::replace_all(ignoreWhereReplaced, "\\", "\\\\");
		command << " --where='NOT(" << ignoreWhereReplaced << ")'";
	}

	command << " > " << file.string();

	executeCommand(command.str());

	// mysqldump -h... -u... -p... --no-data --no-create-info --routines > MySQLStoredProcedures.sql &
	// --skip-extended-insert
	/*
	std::ostringstream command;

	Temp tempDir = temp.createDir();
	TempFile schemaFile = tempDir.createFile(tableName + ".sql");
	TempFile dataFile = tempDir.createFile(tableName + ".txt");

	command << "mysqldump";
	appendConnectionParams(command);
	command << ' ' << tableName;
	command
		<< " --fields-terminated-by=','"
		<< " --tab=" << tempDir.path().string()
	;

	::getchar();
	executeCommand(command.str());

	boost::filesystem::copy(dataFile.path(), file);
	*/
}

void DbOperations::printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) {
	SafeWriter writer(file);
	writer.writeLine("DROP TABLE " + tableName + ";");
}

void DbOperations::import(const boost::filesystem::path &file) {
	import_internal(file);
}

void DbOperations::deleteTable(const std::string &tableName) {
	std::ostringstream command;

	command << "mysql";
	appendConnectionParams(command);
	command
		<< " -e 'SET foreign_key_checks = 0; DROP TABLE " << tableName << "; SET foreign_key_checks = 1;'";
	;

	executeCommand(command.str());
}

std::set<std::string> DbOperations::getTableDependencies(const std::string &tableName) {
	TempFile tableDump = temp.createFile();

	std::ostringstream command;

	command << "mysqldump";
	appendConnectionParams(command);
	command << ' ' << tableName;
	command
		<< " --skip-comments"
		<< " --no-data"
		<< " > " << tableDump.path().string();
	;

	executeCommand(command.str());

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

bool DbOperations::isVersioned() {
	std::set<std::string> tables = getTables_internal();
	return tables.find(versionTableName) != tables.end();
}

void DbOperations::makeVersioned() {
	TempFile versionTableCreation = temp.createFile();

	SafeWriter writer(versionTableCreation.path());
	writer.writeLine("CREATE TABLE `" + versionTableName + "` (");
	writer.writeLine("`value` int(11) NOT NULL");
	writer.writeLine(") ENGINE=InnoDB DEFAULT CHARSET=utf8;");
	writer.writeLine("INSERT INTO VersionInfo(value) VALUES(0);");

	import_internal(versionTableCreation.path());
}

uint32_t DbOperations::getVersion() {
	TempFile version = temp.createFile();

	std::ostringstream command;
	command << "mysqldump";
	appendConnectionParams(command);
	command << ' ' << versionTableName;
	command
		<< " > " << version.path().string();
	;

	executeCommand(command.str());

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

void DbOperations::setVersion(const uint32_t version) {
	std::ostringstream command;

	command << "mysql";
	appendConnectionParams(command);
	command
		<< " -e 'UPDATE " << versionTableName << " SET value=" << version << "';";
	;

	executeCommand(command.str());
}

std::set<std::string> DbOperations::getTables_internal() {
	TempFile tables = temp.createFile();

	std::ostringstream command;
	command << "mysqldump";
	appendConnectionParams(command);
	command
		<< " --compact"
		<< " --no-data"
		<< " > " << tables.path().string();
	;

	executeCommand(command.str());

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

void DbOperations::import_internal(const boost::filesystem::path &file) {
	std::ostringstream command;

	command << "mysql";
	appendConnectionParams(command);
	command
		<< " < " << file.string();
	;

	executeCommand(command.str());
}

void DbOperations::appendConnectionParams(std::ostream &os) {
	os
		<< " --host " << config.host
		<< " -u " << config.user
		<< " -p" << config.password
		<< " " << config.database
	;
}
