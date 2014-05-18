#include "DatabaseFixture.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include "../DatabaseUtils.h"

DatabaseFixture::DatabaseFixture(const DatabaseType &type)
	:
	  temp("temp")
{
	DatabaseTypes databaseTypes;
	boost::property_tree::ptree pt;
	boost::property_tree::read_info("testdbconfig_" + databaseTypes.toString(type), pt);
	config.read(pt);
	database = databaseTypes.createDb(type, config, temp);

	{
		DatabaseUtils u(*database);
		u.clear();
	}
}

Database &DatabaseFixture::get() {
	return *database.get();
}
