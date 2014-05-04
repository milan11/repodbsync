#include "DatabaseTypes.h"

#include <boost/algorithm/string/join.hpp>
#include "Database_MySQL.h"
#include "Database_PostgreSQL.h"
#include "exceptions.h"

DatabaseTypes::DatabaseTypes() {
	map.insert(MapType::value_type(DatabaseType::MYSQL, "mysql"));
	map.insert(MapType::value_type(DatabaseType::POSTGRESQL, "postgresql"));
}

std::string DatabaseTypes::toString(const DatabaseType type) const {
	MapType::left_const_iterator it = map.left.find(type);
	if (it == map.left.end()) {
		THROW("Unsupported database type (toString)");
	}

	return it->second;
}

DatabaseType DatabaseTypes::fromString(const std::string &str) const {
	MapType::right_const_iterator it = map.right.find(str);
	if (it == map.right.end()) {
		 THROW(boost::format("Unsupported database type: %1%. These are supported: %2%.") % str % boost::algorithm::join(getSupportedTypesStrings(), ", "));
	}

	return it->second;
}

std::unique_ptr<Database> DatabaseTypes::createDb(const DatabaseType type, const Config_Db &config, Temp &temp) const {
	switch (type) {
		case DatabaseType::MYSQL:
			return std::unique_ptr<Database>(new Database_MySQL(config, temp));
		case DatabaseType::POSTGRESQL:
			return std::unique_ptr<Database>(new Database_PostgreSQL(config, temp));
		default:
			THROW("Unsupported database type (createDb)");
	}
}

std::vector<std::string> DatabaseTypes::getSupportedTypesStrings() const {
	std::vector<std::string> result;

	for (MapType::const_iterator it = map.begin(); it != map.end(); ++it) {
		result.push_back(it->right);
	}

	return result;
}
