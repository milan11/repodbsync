#pragma once

#include "Command.h"
#include "Config_Db.h"
#include "Database.h"
#include "Temp.h"

class Database_SQLite : public Database {
public:
	Database_SQLite(const Config_Db &config, Temp &temp);

	virtual std::set<std::string> getTables() override;
	virtual void exportTable(const std::string &tableName, const boost::filesystem::path &file) override;
	virtual void exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) override;
	virtual void printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) override;
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
	void import_internal(const boost::filesystem::path &file);
	void deleteTable_internal(const std::string &tableName);

	std::vector<std::string> getColumns(const std::string &tableName);

private:
	const Config_Db &config;
	Temp &temp;

	static const std::string versionTableName;

};
