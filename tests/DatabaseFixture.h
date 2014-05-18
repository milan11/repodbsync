#pragma once

#include "../Config_Db.h"
#include "../Database.h"
#include "../DatabaseTypes.h"
#include "../Temp.h"

class DatabaseFixture {
public:
	DatabaseFixture(const DatabaseType &type);

	Database &get();

	void fillDataA();

private:
	Temp temp;
	Config_Db config;
	std::unique_ptr<Database> database;

};
