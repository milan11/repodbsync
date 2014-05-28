#pragma once

#include "Command.h"
#include "Config_Db.h"
#include "Database.h"
#include "Temp.h"

class Database_PostgreSQL : public Database {
public:
	Database_PostgreSQL(const Config_Db &config, Temp &temp);

	virtual std::set<std::string> getTables() override;
	virtual std::set<std::string> getRoutines() override;
	virtual void exportTable(const std::string &tableName, const boost::filesystem::path &file) override;
	virtual void exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) override;
	virtual void exportRoutine(const std::string &routineName, const boost::filesystem::path &file) override;
	virtual void printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) override;
	virtual void printDeleteRoutine(const std::string &routineName, const boost::filesystem::path &file) override;
	virtual void import(const boost::filesystem::path &file) override;
	virtual void clear() override;
	virtual std::set<std::string> getTableDependencies(const std::string &tableName) override;
	virtual bool isVersioned() override;
	virtual void makeVersioned() override;
	virtual void makeNotVersioned() override;
	virtual uint32_t getVersion() override;
	virtual void setVersion(const uint32_t version) override;

private:
	std::set<std::string> getTables_internal();
	std::set<std::string> getRoutines_internal();
	void import_internal(const boost::filesystem::path &file);
	void deleteTable_internal(const std::string &tableName);
	void deleteRoutine_internal(const std::string &routineName);

	void appendConnectionParamsAndVars_psql(Command &command);
	void appendConnectionParamsAndVars_pgdump(Command &command);
	void setPasswordVariable(Command &command);
	static void appendFormattingParams_psql(Command &command);

	std::string quoteName(const std::string &name);
	std::string quoteRoutineName(const std::string &name);

private:
	const Config_Db &config;
	Temp &temp;

	static const std::string versionTableName;

};
