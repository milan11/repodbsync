#pragma once

#include <boost/filesystem/path.hpp>
#include "Config_Db.h"
#include "DatabaseTypes.h"

class Config {
public:
	explicit Config(boost::filesystem::path file, const DatabaseTypes &databaseTypes);

	bool fileExists() const;
	bool isFilled() const;

	void load();
	void save() const;

	DatabaseType getDbType() const;
	const Config_Db &getDbLocal() const;
	const Config_Db &getDbTemp() const;

private:
	const boost::filesystem::path file;
	const DatabaseTypes &databaseTypes;

	DatabaseType dbType;
	Config_Db dbLocal;
	Config_Db dbTemp;

};
