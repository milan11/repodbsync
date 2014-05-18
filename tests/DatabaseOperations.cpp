#include <boost/test/unit_test.hpp>

#include "../DatabaseTypes.h"
#include "../TextDiff.h"
#include "DatabaseFixture.h"

BOOST_AUTO_TEST_CASE(open) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);
}

BOOST_AUTO_TEST_CASE(initially_not_versioned) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	BOOST_CHECK_EQUAL(db.get().isVersioned(), false);
	// BOOST_CHECK_THROW(db.get().getVersion(), std::runtime_error); // TODO:
}

BOOST_AUTO_TEST_CASE(make_versioned) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.get().makeVersioned();

	BOOST_CHECK_EQUAL(db.get().isVersioned(), true);
	BOOST_CHECK_EQUAL(db.get().getVersion(), 0);
}

BOOST_AUTO_TEST_CASE(make_versioned_set_version) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.get().makeVersioned();
	db.get().setVersion(1);

	BOOST_CHECK_EQUAL(db.get().isVersioned(), true);
	BOOST_CHECK_EQUAL(db.get().getVersion(), 1);
}

BOOST_AUTO_TEST_CASE(make_not_versioned) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.get().makeVersioned();
	db.get().setVersion(1);

	db.get().makeNotVersioned();

	BOOST_CHECK_EQUAL(db.get().isVersioned(), false);
	// BOOST_CHECK_THROW(db.get().getVersion(), std::runtime_error); // TODO:
}

BOOST_AUTO_TEST_CASE(get_tables_empty) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	std::set<std::string> tables = db.get().getTables();

	std::set<std::string> expectedTables;

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE(get_tables) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.fillDataA();

	std::set<std::string> tables = db.get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert("Message");
	expectedTables.insert("User");

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE(get_tables_does_not_include_versioninfo) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.fillDataA();

	db.get().makeVersioned();
	db.get().setVersion(1);

	std::set<std::string> tables = db.get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert("Message");
	expectedTables.insert("User");

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE(get_table_dependencies) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.fillDataA();

	std::set<std::string> dependencies = db.get().getTableDependencies("Message");

	std::set<std::string> expectedDependencies;
	expectedDependencies.insert("User");

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE(get_table_dependencies_empty) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.fillDataA();

	std::set<std::string> dependencies = db.get().getTableDependencies("User");

	std::set<std::string> expectedDependencies;

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE(delete_table) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	db.fillDataA();

	db.get().deleteTable("Message");

	std::set<std::string> tables = db.get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert("User");

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE(export_import_structure) {
	Temp temp("temp_test");

	TempFile dump = temp.createFile();

	{
		DatabaseFixture db(DatabaseType::POSTGRESQL);

		db.fillDataA();

		db.get().exportTable("User", dump.path());
	}

	TempFile dumpAfterImport = temp.createFile();

	{
		DatabaseFixture db(DatabaseType::POSTGRESQL);

		db.get().import(dump.path());

		std::set<std::string> tables = db.get().getTables();

		std::set<std::string> expectedTables;
		expectedTables.insert("User");

		BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());

		db.get().exportTable("User", dumpAfterImport.path());
	}

	TextDiff diff(dump.path(), dumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE(export_import_data) {
	Temp temp("temp_test");

	TempFile structureDump = temp.createFile();
	TempFile dataDump = temp.createFile();

	{
		DatabaseFixture db(DatabaseType::POSTGRESQL);

		db.fillDataA();

		db.get().exportTable("User", structureDump.path());
		db.get().exportData("User", "", dataDump.path());
	}

	TempFile dataDumpAfterImport = temp.createFile();

	{
		DatabaseFixture db(DatabaseType::POSTGRESQL);

		db.get().import(structureDump.path());
		db.get().import(dataDump.path());

		db.get().exportData("User", "", dataDumpAfterImport.path());
	}

	TextDiff diff(dataDump.path(), dataDumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}
