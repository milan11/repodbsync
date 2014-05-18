#include "DatabaseUtils.h"

DatabaseUtils::DatabaseUtils(Database &database)
	:
	  database(database)
{
}

void DatabaseUtils::clear() {
	if (database.isVersioned()) {
		database.makeNotVersioned();
	}
	for (const std::string &tableName : database.getTables()) {
		database.deleteTable(tableName);
	}
}
