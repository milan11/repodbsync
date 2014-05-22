#pragma once

#include <string>
#include <boost/bimap.hpp>
#include "Database.h"
#include "Temp.h"

enum class DatabaseType {
	MYSQL,
	POSTGRESQL,
	SQLITE,
};

class Config_Db;

class DatabaseTypes {
public:
	DatabaseTypes();

	std::string toString(const DatabaseType type) const;
	DatabaseType fromString(const std::string &str) const;
	std::unique_ptr<Database> createDb(const DatabaseType type, const Config_Db &config, Temp &temp) const;

private:
	std::vector<std::string> getSupportedTypesStrings() const;

private:
	typedef boost::bimap<DatabaseType, std::string> MapType;
	MapType map;

};
