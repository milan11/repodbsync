#pragma once

#include "Command.h"
#include "Config_Db.h"
#include "Database.h"
#include "Temp.h"

class Database_MySQL : public Database {
public:
	Database_MySQL(const Config_Db &config, Temp &temp);

	virtual std::set<std::string> getTables();
	virtual void exportTable(const std::string &tableName, const boost::filesystem::path &file);
	virtual void exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file);
	virtual void printDeleteTable(const std::string &tableName, const boost::filesystem::path &file);
	virtual void import(const boost::filesystem::path &file);
	virtual void deleteTable(const std::string &tableName);
	virtual std::set<std::string> getTableDependencies(const std::string &tableName);
	virtual bool isVersioned();
	virtual void makeVersioned();
	virtual uint32_t getVersion();
	virtual void setVersion(const uint32_t version);

private:
	std::set<std::string> getTables_internal();
	void import_internal(const boost::filesystem::path &file);
	void appendConnectionParams(Command &command);

private:
	const Config_Db &config;
	Temp &temp;

	static const std::string versionTableName;

};
