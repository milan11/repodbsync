#pragma once

#include "Database.h"

class DatabaseUtils {
public:
	DatabaseUtils(Database &database);

public:
	void clear();

private:
	Database &database;

};
