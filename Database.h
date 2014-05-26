#pragma once

#include <cstdint>
#include <string>
#include <set>
#include <boost/filesystem/path.hpp>

class Database {
public:
	virtual std::set<std::string> getTables() = 0;
	virtual std::set<std::string> getRoutines() = 0;
	virtual void exportTable(const std::string &tableName, const boost::filesystem::path &file) = 0;
	virtual void exportData(const std::string &tableName, const std::string &ignoreWhere, const boost::filesystem::path &file) = 0;
	virtual void exportRoutine(const std::string &routineName, const boost::filesystem::path &file) = 0;
	virtual void printDeleteTable(const std::string &tableName, const boost::filesystem::path &file) = 0;
	virtual void printDeleteRoutine(const std::string &routineName, const boost::filesystem::path &file) = 0;
	virtual void import(const boost::filesystem::path &file) = 0;
	virtual void clear() = 0;
	virtual std::set<std::string> getTableDependencies(const std::string &tableName) = 0;
	virtual bool isVersioned() = 0;
	virtual void makeVersioned() = 0;
	virtual void makeNotVersioned() = 0;
	virtual uint32_t getVersion() = 0;
	virtual void setVersion(const uint32_t version) = 0;

};
