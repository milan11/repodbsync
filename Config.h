#pragma once

#include <boost/filesystem/path.hpp>
#include "Config_Db.h"

class Config {
public:
	explicit Config(boost::filesystem::path file);

	bool fileExists() const;
	bool isFilled() const;

	void load();
	void save() const;

	const Config_Db &getDbLocal() const;
	const Config_Db &getDbTemp() const;

private:
	Config_Db dbLocal;
	Config_Db dbTemp;

	const boost::filesystem::path file;

};
