#include "Database_MySQL.h"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "LinesReader.h"
#include "SafeWriter.h"
#include "exceptions.h"
#include "util.h"

const std::string Database_MySQL::versionTableName = "VersionInfo";
const std::string Database_MySQL::routinePrefix_procedure = "procedure_";
const std::string Database_MySQL::routinePrefix_function = "function_";

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

std::set<std::string> Database_MySQL::getRoutines() {
	return getRoutines_internal();
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

void Database_MySQL::exportRoutine(const std::string &routineName, const boost::filesystem::path &file) {
	TempFile routineDump = temp.createFile();

	Command command("mysql");

	appendConnectionParams(command);
	command.appendArgument("-e");

	std::string label_begin;
	std::string label_end = "character_set_client: ";

	if (routineName.find(routinePrefix_procedure) == 0) {
		command.appendArgument("SHOW CREATE PROCEDURE " + routineName.substr(routinePrefix_procedure.size()) + "\\G;");
		label_begin = "Create Procedure: ";
	}
	else if (routineName.find(routinePrefix_function) == 0) {
		command.appendArgument("SHOW CREATE FUNCTION " + routineName.substr(routinePrefix_function.size()) + "\\G;");
		label_begin = "Create Function: ";
	}
	else {
		THROW(boost::format("Invalid prefix in routine name: %1%") % routineName);
	}

	command.appendRedirectTo(routineDump.path());

	command.execute();

	const std::string delimiter = "//routine//";

	SafeWriter writer(file);
	LinesReader reader(routineDump.path());
	bool inRoutine = false;
	bool found = false;
	while (boost::optional<std::string> line = reader.readLine()) {
		const std::string line_trimmed = boost::algorithm::trim_copy(*line);
		if (! inRoutine) {
			if (line_trimmed.find(label_begin) == 0) {
				inRoutine = true;
				found = true;
				writer.writeLine("DELIMITER " + delimiter + "; ");
				writer.writeLine(line_trimmed.substr(label_begin.size()));
			}
		} else {
			if (line_trimmed.find(label_end) == 0) {
				writer.writeLine(delimiter + "; ");
				writer.writeLine("DELIMITER ;");
				inRoutine = false;
			} else {
				writer.writeLine(line_trimmed);
			}
		}
	}

	if (! found) {
		THROW(boost::format("Routine begin not found in the output: %1%") % routineName);
	}
	if (inRoutine) {
		THROW(boost::format("Routine end not found in the output: %1%") % routineName);
	}

	writer.close();
}

void Database_MySQL::printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) {
	SafeWriter writer(file);
	writer.writeLine("DROP TABLE " + tableName + ";");
	writer.close();
}

void Database_MySQL::printDeleteRoutine(const std::string &routineName, const boost::filesystem::path &file) {
	SafeWriter writer(file);

	if (routineName.find(routinePrefix_procedure) == 0) {
		writer.writeLine("DROP PROCEDURE " + routineName.substr(routinePrefix_procedure.size()) + ";");
	}
	else if (routineName.find(routinePrefix_function) == 0) {
		writer.writeLine("DROP FUNCTION " + routineName.substr(routinePrefix_function.size()) + ";");
	}
	else {
		THROW(boost::format("Invalid prefix in routine name: %1%") % routineName);
	}

	writer.close();
}

void Database_MySQL::import(const boost::filesystem::path &file) {
	import_internal(file);
}

void Database_MySQL::clear() {
	for (const std::string &tableName : getTables_internal()) {
		Command command("mysql");

		appendConnectionParams(command);

		command
			.appendArgument("-e")
			.appendArgument("SET foreign_key_checks = 0; DROP TABLE " + tableName + "; SET foreign_key_checks = 1;")
			.execute();
	}

	for (const std::string &routineName : getRoutines_internal()) {
		deleteRoutine_internal(routineName);
	}
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

std::set<std::string> Database_MySQL::getRoutines_internal() {
	TempFile routines = temp.createFile();

	Command command("mysql");

	appendConnectionParams(command);
	command.appendArgument("-e");

	command.appendArgument("SHOW PROCEDURE STATUS; SHOW FUNCTION STATUS;");

	command
		.appendRedirectTo(routines.path())
		.execute()
	;

	std::set<std::string> result;
	LinesReader reader(routines.path());
	boost::optional<std::string> line;
	while ((line = reader.readLine())) {
		std::vector<std::string> fields;
		boost::split(fields, *line, boost::is_any_of("\t"));

		if (fields.size() >= 3) {
			if (fields[0] == config.database) {
				std::string name;

				if (fields[2] == "PROCEDURE") {
					name += routinePrefix_procedure;
				}
				else if (fields[2] == "FUNCTION") {
					name += routinePrefix_function;
				}
				else {
					continue;
				}

				name += fields[1];

				result.insert(std::move(name));
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
		.appendArgument("DROP TABLE " + tableName + ";")
		.execute();
}

void Database_MySQL::deleteRoutine_internal(const std::string &routineName) {
	Command command("mysql");

	appendConnectionParams(command);
	command.appendArgument("-e");

	if (routineName.find(routinePrefix_procedure) == 0) {
		command.appendArgument("DROP PROCEDURE " + routineName.substr(routinePrefix_procedure.size()) + ";");
	}
	else if (routineName.find(routinePrefix_function) == 0) {
		command.appendArgument("DROP FUNCTION " + routineName.substr(routinePrefix_function.size()) + ";");
	}
	else {
		THROW(boost::format("Invalid prefix in routine name: %1%") % routineName);
	}

	command.execute();
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
