#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>

class Config_Db {

public:
	void read(const boost::property_tree::ptree &pt);
	boost::property_tree::ptree write() const;

public:
	std::string host;
	std::string user;
	std::string password;
	std::string database;

};
