#include "Config.h"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

Config::Config(boost::filesystem::path file, const DatabaseTypes &databaseTypes)
:
	file(std::move(file)),
	databaseTypes(databaseTypes),
	dbType(DatabaseType::MYSQL)
{
}

bool Config::fileExists() const {
	return boost::filesystem::exists(file);
}

bool Config::isFilled() const {
	if (! dbLocal.isFilledFor(dbType)) return false;
	if (! dbTemp.isFilledFor(dbType)) return false;

	return true;
}

void Config::load() {
	boost::property_tree::ptree pt;
	boost::property_tree::read_info(file.string(), pt);

	std::string typeStr;
	try {
		typeStr = pt.get<std::string>("db_type");
	} catch (const boost::property_tree::ptree_bad_path &) {
		typeStr = "mysql";
	}

	dbType = databaseTypes.fromString(typeStr);
	dbLocal.read(pt.get_child("db_local"));
	dbTemp.read(pt.get_child("db_temp"));
}

void Config::save() const {
	boost::property_tree::ptree pt;

	pt.put("db_type", databaseTypes.toString(dbType));

	pt.put_child("db_local", dbLocal.write());
	pt.put_child("db_temp", dbTemp.write());

	boost::property_tree::write_info(file.string(), pt);
}

DatabaseType Config::getDbType() const {
	return dbType;
}

const Config_Db &Config::getDbLocal() const {
	return dbLocal;
}

const Config_Db &Config::getDbTemp() const {
	return dbTemp;
}
