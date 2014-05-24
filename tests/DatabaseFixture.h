#pragma once

#include "../Config_Db.h"
#include "../Database.h"
#include "../DatabaseTypes.h"
#include "../Temp.h"

class DatabaseFixture {
public:
	explicit DatabaseFixture(const DatabaseType type, const bool lowerCaseNames);

	DatabaseType getType() const;
	Database &get();

	std::string changeNameCase(const std::string &orig);

	void fillDataA();
	void fillDataA_filtered();

private:
	void fillDataA_internal(const bool &withoutThirdUser);

	std::string name(const std::string &orig);

	std::string changeNameCase_internal(const std::string &orig);

private:
	const DatabaseType type;
	const bool lowerCaseNames;

	Temp temp;
	Config_Db config;
	std::unique_ptr<Database> database;

};
