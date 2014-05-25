#include "DatabaseFixture.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include "../SafeWriter.h"
#include "../exceptions.h"

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

	if (! config.isFilledFor(type)) {
		THROW(boost::format("Database config not filled in for %1%") % DatabaseTypes().toString(type));
	}

	database = databaseTypes.createDb(type, config, temp);
	database->clear();
}

DatabaseType DatabaseFixture::getType() const {
	return type;
}

bool DatabaseFixture::isLowerCaseNames() const {
	return lowerCaseNames;
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

	w.writeLine("CREATE TABLE " + name("UserRole") + " (");
	w.writeLine(name("Id") + " integer PRIMARY KEY NOT NULL,");
	w.writeLine(name("Name") + " varchar(64) NOT NULL,");
	w.writeLine(name("Description") + " varchar(128) NOT NULL");
	w.writeLine(");");

	w.writeLine("CREATE TABLE " + name("User") + " (");
	w.writeLine(name("Id") + " integer PRIMARY KEY NOT NULL,");
	w.writeLine(name("Name") + " varchar(64) UNIQUE NOT NULL,");
	w.writeLine(name("Role") + " integer NOT NULL,");
	w.writeLine("FOREIGN KEY (" + name("Role") + ") REFERENCES " + name("UserRole") + "(" + name("Id") + ")");
	w.writeLine(");");

	w.writeLine("CREATE TABLE " + name("Message") + " (");
	w.writeLine(name("Text") + " varchar(64) NOT NULL,");
	w.writeLine(name("From") + " integer NOT NULL,");
	w.writeLine(name("To") + " integer NOT NULL,");
	w.writeLine(name("Date") + " date NOT NULL,");
	w.writeLine("FOREIGN KEY (" + name("From") + ") REFERENCES " + name("User") + "(" + name("Id") + "),");
	w.writeLine("FOREIGN KEY (" + name("To") + ") REFERENCES " + name("User") + "(" + name("Id") + ")");
	w.writeLine(");");

	w.writeLine("INSERT INTO " + name("UserRole") + " (" + name("Id") + ", " + name("Name") + ", " + name("Description") + ") VALUES (1, 'Guest', 'Anonymous Users');");

	w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ", " + name("Role") + ") VALUES (1, 'First User', 1);");
	w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ", " + name("Role") + ") VALUES (2, 'Second User', 1);");

	if (! withoutThirdUser) {
		w.writeLine("INSERT INTO " + name("User") + " (" + name("Id") + ", " + name("Name") + ", " + name("Role") + ") VALUES (3, 'Third User', 1);");
	}

	w.writeLine("INSERT INTO " + name("Message") + " (" + name("Text") + ", " + name("From") + ", " + name("To") + ", " + name("Date") + ") VALUES ('hello', 1, 2, '2014-05-18');");

	w.writeLine("CREATE TABLE " + name("CyclicA") + " (");
	w.writeLine(name("Id") + " integer PRIMARY KEY NOT NULL");
	w.writeLine(");");

	w.writeLine("CREATE TABLE " + name("CyclicB") + " (");
	w.writeLine(name("Id") + " integer PRIMARY KEY NOT NULL");
	w.writeLine(");");

	if (type == DatabaseType::MYSQL) {
		// if the foreign key was specified directly in ADD COLUMN, the foreign key constraint has been missing in the CREATE TABLE dump
		w.writeLine("ALTER TABLE " + name("CyclicA") + " ADD COLUMN " + name("refAtoB") + " INTEGER NULL;");
		w.writeLine("ALTER TABLE " + name("CyclicB") + " ADD COLUMN " + name("refBtoA") + " INTEGER NOT NULL;");

		w.writeLine("ALTER TABLE " + name("CyclicA") + " ADD CONSTRAINT " + name("Fk_refAtoB") + " FOREIGN KEY (" + name("refAtoB") + ") REFERENCES " + name("CyclicB") + "(" + name("Id") + ");");
		w.writeLine("ALTER TABLE " + name("CyclicB") + " ADD CONSTRAINT " + name("Fk_refBtoA") + " FOREIGN KEY (" + name("refBtoA") + ") REFERENCES " + name("CyclicA") + "(" + name("Id") + ");");
	} else {
		w.writeLine("ALTER TABLE " + name("CyclicA") + " ADD COLUMN " + name("refAtoB") + " INTEGER NULL REFERENCES " + name("CyclicB") + "(" + name("Id") + ");");
		w.writeLine("ALTER TABLE " + name("CyclicB") + " ADD COLUMN " + name("refBtoA") + " INTEGER NOT NULL DEFAULT 0 REFERENCES " + name("CyclicA") + "(" + name("Id") + ");");
	}

	w.writeLine("INSERT INTO " + name("CyclicA")  + " (" + name("Id") + ") VALUES (2);");
	w.writeLine("INSERT INTO " + name("CyclicB")  + " (" + name("Id") + ", " + name("refBtoA") + ") VALUES (3, 2);");

	w.writeLine("UPDATE " + name("CyclicA")  + "SET " + name("refAtoB") + " = 3;");

	w.close();

	database->import(f.path());
}

std::string DatabaseFixture::name(const std::string &orig) {
	if ((type == DatabaseType::POSTGRESQL) || (type == DatabaseType::SQLITE)) {
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
