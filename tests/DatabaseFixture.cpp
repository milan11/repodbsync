#include "DatabaseFixture.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include "../DatabaseUtils.h"
#include "../SafeWriter.h"

DatabaseFixture::DatabaseFixture(const DatabaseType type, const bool lowerCaseNames)
	:
	  type(type),
	  lowerCaseNames(lowerCaseNames),
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

std::string DatabaseFixture::changeNameCase(const std::string &orig) {
	return changeNameCase_internal(orig);
}

void DatabaseFixture::fillDataA() {
	fillDataA_internal(false);
}

void DatabaseFixture::fillDataA_filtered() {
	fillDataA_internal(true);
}

void DatabaseFixture::fillDataA_internal(const bool &withoutThirdUser) {
	TempFile f = temp.createFile();

	SafeWriter w(f.path());

	w.writeLine("CREATE TABLE " + name("User") + " (");
	w.writeLine(name("Id") + " integer NOT NULL,");
	w.writeLine(name("Name") + " varchar(64) NOT NULL");
	w.writeLine(");");

	w.writeLine("ALTER TABLE " + name("User"));
	w.writeLine("ADD CONSTRAINT " + name("Id") + " PRIMARY KEY (" + name("Id") + ");");

	w.writeLine("ALTER TABLE " + name("User"));
	w.writeLine("ADD CONSTRAINT " + name("Name") + " UNIQUE (" + name("Name") + ");");

	w.writeLine("CREATE TABLE " + name("Message") + " (");
	w.writeLine(name("Text") + " varchar(64) NOT NULL,");
	w.writeLine(name("From") + " integer NOT NULL,");
	w.writeLine(name("To") + " integer NOT NULL,");
	w.writeLine(name("Date") + " date NOT NULL");
	w.writeLine(");");

	w.writeLine("CREATE INDEX " + name("Fki_from") + " ON " + name("Message") + " (" + name("From") + ");");
	w.writeLine("CREATE INDEX " + name("Fki_to") + " ON " + name("Message") + " (" + name("To") + ");");

	w.writeLine("ALTER TABLE " + name("Message"));
	w.writeLine("ADD CONSTRAINT " + name("From") + " FOREIGN KEY (" + name("From") + ") REFERENCES " + name("User") + "(" + name("Id") + ");");

	w.writeLine("ALTER TABLE " + name("Message"));
	w.writeLine("ADD CONSTRAINT " + name("To") + " FOREIGN KEY (" + name("To") + ") REFERENCES " + name("User") + "(" + name("Id") + ");");

	w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ") VALUES (1, 'First User');");
	w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ") VALUES (2, 'Second User');");

	if (! withoutThirdUser) {
		w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ") VALUES (3, 'Third User');");
	}

	w.writeLine("INSERT INTO " + name("Message") + " (" + name("Text") + ", " + name("From") + ", " + name("To") + ", " + name("Date") + ") VALUES ('hello', 1, 2, '2014-05-18');");

	w.close();

	database->import(f.path());
}

std::string DatabaseFixture::name(const std::string &orig) {
	if (type == DatabaseType::POSTGRESQL) {
		return '"' + changeNameCase_internal(orig) + '"';
	}

	if (type == DatabaseType::MYSQL) {
		return '`' + changeNameCase_internal(orig) + '`';
	}

	return changeNameCase_internal(orig);
}

std::string DatabaseFixture::changeNameCase_internal(const std::string &orig) {
	if (lowerCaseNames) {
		return boost::algorithm::to_lower_copy(orig);
	} else {
		return orig;
	}
}
