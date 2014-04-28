#pragma once

#include <cstdint>
#include <string>
#include <set>
#include "Command.h"
#include "Config_Db.h"
#include "Temp.h"

class DbOperations {
public:
	DbOperations(const Config_Db &config, Temp &temp);

	std::set<std::string> getTables();
	void exportTable(const std::string &tableName, const boost::filesystem::path &file);
	void exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file);
	void printDeleteTable(const std::string &tableName, const boost::filesystem::path &file);
	void import(const boost::filesystem::path &file);
	void deleteTable(const std::string &tableName);
	std::set<std::string> getTableDependencies(const std::string &tableName);
	bool isVersioned();
	void makeVersioned();
	uint32_t getVersion();
	void setVersion(const uint32_t version);

private:
	std::set<std::string> getTables_internal();
	void import_internal(const boost::filesystem::path &file);
	void appendConnectionParams(Command &command);

private:
	const Config_Db &config;
	Temp &temp;

	static const std::string versionTableName;

};
