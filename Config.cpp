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
	if (dbLocal.database.empty()) return false;
	if (dbTemp.database.empty()) return false;

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

	{
		boost::property_tree::ptree db_local = pt.get_child("db_local");
		dbLocal.host = db_local.get<std::string>("host");
		dbLocal.user = db_local.get<std::string>("user");
		dbLocal.password = db_local.get<std::string>("password");
		dbLocal.database = db_local.get<std::string>("database");
	}

	{
		boost::property_tree::ptree db_temp = pt.get_child("db_temp");
		dbTemp.host = db_temp.get<std::string>("host");
		dbTemp.user = db_temp.get<std::string>("user");
		dbTemp.password = db_temp.get<std::string>("password");
		dbTemp.database = db_temp.get<std::string>("database");
	}
}

void Config::save() const {
	boost::property_tree::ptree pt;

	pt.put("db_type", databaseTypes.toString(dbType));

	{
		boost::property_tree::ptree db_local;
		db_local.put("host", dbLocal.host);
		db_local.put("user", dbLocal.user);
		db_local.put("password", dbLocal.password);
		db_local.put("database", dbLocal.database);

		pt.put_child("db_local", db_local);
	}

	{
		boost::property_tree::ptree db_temp;
		db_temp.put("host", dbTemp.host);
		db_temp.put("user", dbTemp.user);
		db_temp.put("password", dbTemp.password);
		db_temp.put("database", dbTemp.database);

		pt.put_child("db_temp", db_temp);
	}

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
