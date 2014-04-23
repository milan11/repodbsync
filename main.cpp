#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <set>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include "Config.h"
#include "DbOperations.h"
#include "IgnoredData.h"
#include "LinesReader.h"
#include "Outs.h"
#include "Question.h"
#include "SafeWriter.h"
#include "Scripts.h"
#include "ScriptProperties.h"
#include "Temp.h"
#include "TextDiff.h"
#include "exceptions.h"
#include "util.h"

std::vector<std::string> sortByDependencies(const std::set<std::string> &tables, DbOperations &db) {
	std::map<std::string, std::set<std::string> > tablesWithDependencies;
	for (const std::string &table : tables) {
		tablesWithDependencies[table] = db.getTableDependencies(table);
	}

	std::set<std::string> added;
	std::vector<std::string> result;

	while (added.size() < tables.size()) {
		bool someAdded = false;
		for (const std::string &table : tables) {
			if (added.find(table) != added.end()) {
				continue;
			}

			const std::set<std::string> &dependencies = tablesWithDependencies[table];
			bool allDependenciesAlreadyAdded = std::all_of(dependencies.begin(), dependencies.end(), [&added](const std::string &dependency) -> bool { return added.find(dependency) != added.end(); });

			if (allDependenciesAlreadyAdded) {
				added.insert(table);
				result.push_back(table);
				someAdded = true;
			}
		}

		bool forceAddingOne = (! someAdded);
		if (forceAddingOne) {
			for (const std::string &table : tables) {
				if (added.find(table) != added.end()) {
					continue;
				}

				added.insert(table);
				result.push_back(table);
				break;
			}
		}
	}

	return result;
}

IgnoredData parseIgnoredDataLine(const std::string &line) {
	std::string tableName;
	std::string where;
	std::string::size_type delimPos = line.find_first_of(" \t");
	if (delimPos == std::string::npos) {
		tableName = line;
	} else {
		tableName = line.substr(0, delimPos);
		where = boost::algorithm::trim_copy(line.substr(delimPos + 1));
	}

	return IgnoredData(std::move(tableName), std::move(where));
}

boost::optional<std::string> getIgnoredDataWhere(const std::string &tableName, const std::vector<IgnoredData> &ignoredDataList) {
	for (const IgnoredData &ignoredData : ignoredDataList) {
		if (ignoredData.getTableName() == tableName) {
			const bool ignoreAll = ignoredData.getWhere().empty();
			if (ignoreAll) {
				return boost::none;
			}
		}
	}

	std::ostringstream where;
	bool first = true;
	for (const IgnoredData &ignoredData : ignoredDataList) {
		if (ignoredData.getTableName() == tableName) {
			if (first) {
				first = false;
			} else {
				where << " OR ";
			}
			where << ignoredData.getWhere();
		}
	}

	return where.str();
}

void main_inner() {

	boost::filesystem::path rootDir = boost::filesystem::current_path();

	const std::string configFileName = "dbup_config_local.txt";
	const std::string scriptsDirName = "scripts";

	const std::string ignoredTablesFileName = "ignore_tables.txt";
	const std::string ignoredTablesLocalFileName = "ignore_tables_local.txt";
	const std::string ignoredDataFileName = "ignore_data.txt";
	const std::string ignoredDataLocalFileName = "ignore_data_local.txt";
	const std::string gitignoreFileName = ".gitignore";

	Config c(rootDir / configFileName);

	if (! c.fileExists()) {
		if (! boost::filesystem::is_empty(rootDir)) {
			THROW(boost::format("The current directory %1% is not a repository database directory. If you want to initialize a new repository database directory, create an empty directory (e.g. in your git repository working copy) and run this again in that new empty directory.") % rootDir.string());
		}

		c.save();
		THROW(boost::format("Config not found. Fill the configuration in the file %1%") % configFileName);
	}

	c.load();
	if (! c.isFilled()) {
		THROW(boost::format("Config not filled. Fill the configuration in the file %1%") % configFileName);
	}

	bool ignoreFilesCreated = boost::filesystem::exists(rootDir / scriptsDirName);
	if (! ignoreFilesCreated) {
		if (! boost::filesystem::exists(rootDir / ignoredTablesFileName)) {
			SafeWriter w(rootDir / ignoredTablesFileName);
		}
		if (! boost::filesystem::exists(rootDir / ignoredTablesLocalFileName)) {
			SafeWriter w(rootDir / ignoredTablesLocalFileName);
		}
		if (! boost::filesystem::exists(rootDir / ignoredDataFileName)) {
			SafeWriter w(rootDir / ignoredDataFileName);
		}
		if (! boost::filesystem::exists(rootDir / ignoredDataLocalFileName)) {
			SafeWriter w(rootDir / ignoredDataLocalFileName);
		}
		if (! boost::filesystem::exists(rootDir / gitignoreFileName)) {
			SafeWriter w(rootDir / gitignoreFileName);
			w.writeLine(configFileName);
			w.writeLine(ignoredTablesLocalFileName);
			w.writeLine(ignoredDataLocalFileName);
		}

	}

	Temp temp(rootDir / "temp");
	Outs outs(rootDir / "outs");
	Scripts scripts(rootDir / scriptsDirName);
	const uint32_t repositoryVersion = scripts.getVersion();

	std::set<std::string> ignoredTables;
	if (boost::filesystem::exists(rootDir / ignoredTablesFileName)) {
		LinesReader reader(rootDir / ignoredTablesFileName);
		while (boost::optional<std::string> line = reader.readLine()) {
			ignoredTables.insert(std::move(*line));
		}
	}
	if (boost::filesystem::exists(rootDir / ignoredTablesLocalFileName)) {
		LinesReader reader(rootDir / ignoredTablesLocalFileName);
		while (boost::optional<std::string> line = reader.readLine()) {
			ignoredTables.insert(std::move(*line));
		}
	}

	std::vector<IgnoredData> ignoredData;
	if (boost::filesystem::exists(rootDir / ignoredDataFileName)) {
		LinesReader reader(rootDir / ignoredDataFileName);
		while (boost::optional<std::string> line = reader.readLine()) {
			ignoredData.push_back(parseIgnoredDataLine(*line));
		}
	}
	if (boost::filesystem::exists(rootDir / ignoredDataLocalFileName)) {
		LinesReader reader(rootDir / ignoredDataLocalFileName);
		while (boost::optional<std::string> line = reader.readLine()) {
			ignoredData.push_back(parseIgnoredDataLine(*line));
		}
	}


	DbOperations dbLocal(c.getDbLocal(), temp);
	DbOperations dbTemp(c.getDbTemp(), temp);

	try {
		for (const std::string &tableName : dbTemp.getTables()) {
			dbTemp.deleteTable(tableName);
		}
	} HANDLE_RETHROW("Unable to clear the temporary database.");

	try {
		for (const boost::filesystem::path &script : scripts.getFiles()) {
			dbTemp.import(script);
		}
	} HANDLE_RETHROW("Unable to import data to the temporary database using scripts in the repository.");

	if (! dbLocal.isVersioned()) {
		Question question;
		question.setText("The local database is not versioned. Add version information?");
		const Question::OptionIndex option_yes = question.addOption("y", "yes");
		(void)question.addOption("n", "no");
		if (question.show() == option_yes) {
			dbLocal.makeVersioned();
		} else {
			THROW("Aborted - not creating version information.");
		}
	}

	if (dbLocal.getVersion() > repositoryVersion) {
		Question question;
		question.setText("The local database has higher version than the repository. Repair this by setting database version to the repository version? Current database version: " + std::to_string(dbLocal.getVersion()) + ", Current repository version: " + std::to_string(repositoryVersion));
		const Question::OptionIndex option_yes = question.addOption("y", "yes");
		(void)question.addOption("n", "no");
		if (question.show() == option_yes) {
			dbLocal.setVersion(repositoryVersion);
		} else {
			THROW("Aborted - the local VALUEdatabase has higher version than the repository.");
		}
	}

	{
		Question question;
		const Question::OptionIndex option_automatic = question.addOption("y", "Apply script automatically.");
		(void)question.addOption("n", "Increment the version only - if you have changed your local database manually to reflect the changes in this script or if you already had your database in the target state (e.g. when you just created this new script to reflect your local database).");
		const Question::OptionIndex option_automatic_all = question.addOption("yall", "Apply all scripts automatically (maybe dangerous).");
		const Question::OptionIndex option_manual_all = question.addOption("nall", "Increment the version to the target version of the last script - if you know that you have applied all changes from the scripts in your database.");
		const Question::OptionIndex option_abort = question.addOption("a", "Abort.");

		Question::OptionIndex selectedOption = option_abort;

		const uint32_t localDbVersion = dbLocal.getVersion();
		for (const boost::filesystem::path &script : scripts.getFiles()) {
			ScriptProperties scriptProperties(script);

			const uint32_t scriptTargetVersion = scriptProperties.getTargetDbVersion();
			if (localDbVersion < scriptTargetVersion) {
				if ((selectedOption != option_automatic_all) && (selectedOption != option_manual_all)) {
					question.setText("The script " + scriptProperties.getName() + " will upgrade your local database to version " + std::to_string(scriptProperties.getTargetDbVersion()) + ". Apply?");

					selectedOption = question.show();
				}

				if (selectedOption == option_abort) {
					THROW("Aborted.");
				}

				if ((selectedOption == option_automatic) || (selectedOption == option_automatic_all)) {
					dbLocal.import(script);
				}

				dbLocal.setVersion(scriptTargetVersion);
			}
		}
	}

	std::set<std::string> tablesInLocal = dbLocal.getTables();
	std::set<std::string> tablesInRepository = dbTemp.getTables();

	std::set<std::string> repositoryOnly;
	std::set<std::string> localOnly;
	std::set<std::string> differentTable;
	std::set<std::string> differentData;
	for (const std::string &table : tablesInRepository) {
		if (ignoredTables.find(table) != ignoredTables.end()) {
			continue;
		}

		if (tablesInLocal.find(table) == tablesInLocal.end()) {
			repositoryOnly.insert(table);
		}
	}

	for (const std::string &table : tablesInLocal) {
		if (ignoredTables.find(table) != ignoredTables.end()) {
			continue;
		}

		if (tablesInRepository.find(table) != tablesInRepository.end()) {
			{
				TempFile localDump = temp.createFile();
				dbLocal.exportTable(table, localDump.path());
				TempFile repositoryDump = temp.createFile();
				dbTemp.exportTable(table, repositoryDump.path());

				TextDiff diff(localDump.path(), repositoryDump.path());
				if (! diff.areEqual()) {
					localDump.moveTo(outs.createFile(table + "_local.sql"));
					repositoryDump.moveTo(outs.createFile(table + "_repository.sql"));
					differentTable.insert(table);
				}
			}

			boost::optional<std::string> ignoreDataWhere = getIgnoredDataWhere(table, ignoredData);
			if (ignoreDataWhere) {
				TempFile localDump = temp.createFile();
				dbLocal.exportData(table, *ignoreDataWhere, localDump.path());
				TempFile repositoryDump = temp.createFile();
				dbTemp.exportData(table, *ignoreDataWhere, repositoryDump.path());

				TextDiff diff(localDump.path(), repositoryDump.path());
				if (! diff.areEqual()) {
					localDump.moveTo(outs.createFile(table + "_local_data.sql"));
					repositoryDump.moveTo(outs.createFile(table + "_repository_data.sql"));
					differentData.insert(table);
				}
			}

		} else {
			localOnly.insert(table);
		}
	}

	uint32_t nextScriptTargetVersion = repositoryVersion + 1;
	for (const std::string &table : boost::adaptors::reverse(sortByDependencies(repositoryOnly, dbTemp))) {
		const std::string scriptFileName = ScriptProperties(nextScriptTargetVersion, "delete_" + table).toFileName();
		dbLocal.printDeleteTable(table, outs.createFile(scriptFileName));
		++nextScriptTargetVersion;
	}

	for (const std::string &table : sortByDependencies(localOnly, dbLocal)) {
		const std::string scriptFileName = ScriptProperties(nextScriptTargetVersion, "create_" + table).toFileName();
		dbLocal.exportTable(table, outs.createFile(scriptFileName));
		++nextScriptTargetVersion;

		boost::optional<std::string> ignoreDataWhere = getIgnoredDataWhere(table, ignoredData);
		if (ignoreDataWhere) {
			TempFile data = temp.createFile();
			dbLocal.exportData(table, *ignoreDataWhere, data.path());

			if (! boost::filesystem::is_empty(data.path())) {
				const std::string scriptFileName = ScriptProperties(nextScriptTargetVersion, "fill_" + table).toFileName();
				data.moveTo(outs.createFile(scriptFileName));
				++nextScriptTargetVersion;
			}
		}
	}

	const bool conflicts = (! repositoryOnly.empty()) || (! localOnly.empty()) || (! differentTable.empty() || (! differentData.empty()));

	if (conflicts) {
		std::cout << "There are some differences between the repository and your local database. There are 4 ways to resolve a difference:" << std::endl;
		std::cout << "  1. Add scripts to the repository (scripts directory) which will reflect the state of your local database." << std::endl;
		std::cout << "  2. Make changes in your local database to reflect the state of the repository database. Maybe this is a rare case, because you should already have all relevant changes from the repository applied to your local database because you have the same version of the database." << std::endl;
		std::cout << "  3. Ignore the table. Add its name on a separate line to the file ignore_tables.txt (shared in the repository) or ignore_tables_local.txt (for you only)." << std::endl;
		std::cout << "  4. Ignore the table data. Add its name on a separate line to the file ignore_data.txt (shared in the repository) or ignore_data_local.txt (for you only)." << std::endl;
	}

	if (! repositoryOnly.empty()) {
		std::cout << "The following tables are in the repository, but are not in the local database." << std::endl;
		std::cout << "  HINT: If you want to create new scripts in the repository deleting these tables, the files named named <num>_delete_<table>.sql in the outs directory can help you." << std::endl;
		for (const std::string &table : repositoryOnly) {
			std::cout << "  " << table << std::endl;
		}
	}

	if (! localOnly.empty()) {
		std::cout << "The following tables are in the local database, but are missing in the repository." << std::endl;
		std::cout << "  HINT: If you want to create new scripts in the repository creating these tables, the files named <num>_create_<table>.sql in the outs directory can help you." << std::endl;
		for (const std::string &table : localOnly) {
			std::cout << "  " << table << std::endl;
		}
	}

	if (! differentTable.empty()) {
		std::cout << "The following tables are different." << std::endl;
		std::cout << "  HINT: The dumps of the two states of each table are in outs, named <table>_local.sql and <table>_repository.sql." << std::endl;
		for (const std::string &table : differentTable) {
			std::cout << "  " << table << std::endl;
		}
	}

	if (! differentData.empty()) {
		std::cout << "The following tables have different data:" << std::endl;
		std::cout << "  HINT: The dumps of the two states of each table are in outs, named <table>_local.csv and <table>_repository.csv." << std::endl;
		for (const std::string &table : differentData) {
			std::cout << "  " << table << std::endl;
		}
	}

	if (conflicts) {
		std::cout << "Some more hints:" << std::endl;
		std::cout << "  HINT: A script file has not to contain a database script regarding one table only. You can even add one single script creating the whole database (this can be the case for the first script)." << std::endl;
		std::cout << "  HINT: The script names have to begin with a six digit number followed with an underscore. The numbers have to begin with 1 and no number can be missing." << std::endl;
		std::cout << "  HINT: Remember that once committed, the scripts should not be changed. You can only add a following script reverting the change in the database (e.g. DROP TABLE reverting CREATE TABLE)." << std::endl;
	}
}

int main() {
	try {
		main_inner();
	} HANDLE_PRINT;

	return 0;
}
