#include "Config_Db.h"

void Config_Db::read(const boost::property_tree::ptree &pt) {
	host = pt.get<std::string>("host");
	user = pt.get<std::string>("user");
	password = pt.get<std::string>("password");
	database = pt.get<std::string>("database");
	file = pt.get<std::string>("file");
}

boost::property_tree::ptree Config_Db::write() const {
	boost::property_tree::ptree pt;

	pt.put("host", host);
	pt.put("user", user);
	pt.put("password", password);
	pt.put("database", database);
	pt.put("file", file);

	return pt;
}

bool Config_Db::isFilledFor(const DatabaseType dbType) const {
	if (dbType == DatabaseType::SQLITE) {
		return (! file.empty());
	} else {
		return (! host.empty());
	}
}
