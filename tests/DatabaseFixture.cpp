#include "DatabaseFixture.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include "../DatabaseUtils.h"
#include "../SafeWriter.h"

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

void DatabaseFixture::fillDataA() {
	TempFile f = temp.createFile();

	SafeWriter w(f.path());

	w.writeLine("CREATE TABLE \"User\" (");
	w.writeLine("id integer NOT NULL,");
	w.writeLine("name text NOT NULL");
	w.writeLine(");");

	w.writeLine("ALTER TABLE ONLY \"User\"");
	w.writeLine("ADD CONSTRAINT id PRIMARY KEY (id);");

	w.writeLine("ALTER TABLE ONLY \"User\"");
	w.writeLine("ADD CONSTRAINT name UNIQUE (name);");

	w.writeLine("CREATE TABLE \"Message\" (");
	w.writeLine("text text NOT NULL,");
	w.writeLine("\"from\" integer NOT NULL,");
	w.writeLine("\"to\" integer NOT NULL,");
	w.writeLine("date date NOT NULL");
	w.writeLine(");");

	w.writeLine("CREATE INDEX fki_from ON \"Message\" USING btree (\"from\");");
	w.writeLine("CREATE INDEX fki_to ON \"Message\" USING btree (\"to\");");

	w.writeLine("ALTER TABLE ONLY \"Message\"");
	w.writeLine("ADD CONSTRAINT \"from\" FOREIGN KEY (\"from\") REFERENCES \"User\"(id);");

	w.writeLine("ALTER TABLE ONLY \"Message\"");
	w.writeLine("ADD CONSTRAINT \"to\" FOREIGN KEY (\"to\") REFERENCES \"User\"(id);");

	w.writeLine("INSERT INTO \"User\" (id, name) VALUES (1, 'First User');");
	w.writeLine("INSERT INTO \"User\" (id, name) VALUES (2, 'Second User');");

	w.writeLine("INSERT INTO \"Message\" (text, \"from\", \"to\", date) VALUES ('hello', 1, 2, '2014-05-18');");

	w.close();

	database->import(f.path());
}
