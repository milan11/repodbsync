#pragma once

#include "Command.h"
#include "Config_Db.h"
#include "Database.h"
#include "Temp.h"

class Database_PostgreSQL : public Database {
public:
	Database_PostgreSQL(const Config_Db &config, Temp &temp);

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

	void appendConnectionParamsAndVars_psql(Command &command);
	void appendConnectionParamsAndVars_pgdump(Command &command);
	void setPasswordVariable(Command &command);
	static void appendFormattingParams_psql(Command &command);

	std::string quoteName(const std::string &name);

private:
	const Config_Db &config;
	Temp &temp;

	static const std::string versionTableName;

};
