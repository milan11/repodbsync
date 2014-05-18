#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>

#include "../DatabaseTypes.h"
#include "../TextDiff.h"
#include "DatabaseFixture.h"

template <const DatabaseType type, const bool lowerCaseNames = false>
struct Fixture {
	static std::unique_ptr<DatabaseFixture> get() { return std::unique_ptr<DatabaseFixture>(new DatabaseFixture(type, lowerCaseNames)); }
};

typedef boost::mpl::list<Fixture<DatabaseType::POSTGRESQL, false>, Fixture<DatabaseType::POSTGRESQL, true> > Fixtures;

BOOST_AUTO_TEST_CASE_TEMPLATE(open, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(initially_not_versioned, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	BOOST_CHECK_EQUAL(db->get().isVersioned(), false);
	// BOOST_CHECK_THROW(db->get().getVersion(), std::runtime_error); // TODO:
}

BOOST_AUTO_TEST_CASE_TEMPLATE(make_versioned, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->get().makeVersioned();

	BOOST_CHECK_EQUAL(db->get().isVersioned(), true);
	BOOST_CHECK_EQUAL(db->get().getVersion(), 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(make_versioned_set_version, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->get().makeVersioned();
	db->get().setVersion(1);

	BOOST_CHECK_EQUAL(db->get().isVersioned(), true);
	BOOST_CHECK_EQUAL(db->get().getVersion(), 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(make_not_versioned, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->get().makeVersioned();
	db->get().setVersion(1);

	db->get().makeNotVersioned();

	BOOST_CHECK_EQUAL(db->get().isVersioned(), false);
	// BOOST_CHECK_THROW(db->get().getVersion(), std::runtime_error); // TODO:
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_tables_empty, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables;

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_tables, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert(db->changeNameCase("Message"));
	expectedTables.insert(db->changeNameCase("User"));

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_tables_does_not_include_versioninfo, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	db->get().makeVersioned();
	db->get().setVersion(1);

	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert(db->changeNameCase("Message"));
	expectedTables.insert(db->changeNameCase("User"));

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_table_dependencies, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> dependencies = db->get().getTableDependencies(db->changeNameCase("Message"));

	std::set<std::string> expectedDependencies;
	expectedDependencies.insert(db->changeNameCase("User"));

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_table_dependencies_empty, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> dependencies = db->get().getTableDependencies(db->changeNameCase("User"));

	std::set<std::string> expectedDependencies;

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(delete_table, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	db->get().deleteTable(db->changeNameCase("Message"));

	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert(db->changeNameCase("User"));

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_structure, F, Fixtures) {
	Temp temp("temp_test");

	TempFile dump = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		db->get().exportTable(db->changeNameCase("User"), dump.path());
	}

	TempFile dumpAfterImport = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->get().import(dump.path());

		std::set<std::string> tables = db->get().getTables();

		std::set<std::string> expectedTables;
		expectedTables.insert(db->changeNameCase("User"));

		BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());

		db->get().exportTable(db->changeNameCase("User"), dumpAfterImport.path());
	}

	TextDiff diff(dump.path(), dumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_data, F, Fixtures) {
	Temp temp("temp_test");

	TempFile structureDump = temp.createFile();
	TempFile dataDump = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		db->get().exportTable(db->changeNameCase("User"), structureDump.path());
		db->get().exportData(db->changeNameCase("User"), "", dataDump.path());
	}

	TempFile dataDumpAfterImport = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->get().import(structureDump.path());
		db->get().import(dataDump.path());

		db->get().exportData(db->changeNameCase("User"), "", dataDumpAfterImport.path());
	}

	TextDiff diff(dataDump.path(), dataDumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(print_delete_table, F, Fixtures) {
	Temp temp("temp_test");

	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	TempFile deleteTable = temp.createFile();
	db->get().printDeleteTable(db->changeNameCase("Message"), deleteTable.path());

	db->get().import(deleteTable.path());


	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables;
	expectedTables.insert(db->changeNameCase("User"));

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_data_where, F, Fixtures) {
	Temp temp("temp_test");

	TempFile dataDump_expected = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA_filtered();

		db->get().exportData(db->changeNameCase("User"), "", dataDump_expected.path());
	}

	std::set<std::string> thirdUserConditions;

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		thirdUserConditions.insert(db->changeNameCase("Id") + " = 3");
		thirdUserConditions.insert(db->changeNameCase("Name") + " = 'Third User'");
		thirdUserConditions.insert(db->changeNameCase("Id") + " = 3 AND " + db->changeNameCase("Name") + " = 'Third User'");
		thirdUserConditions.insert(db->changeNameCase("Id") + " = 3 OR " + db->changeNameCase("Name") + " = 'Non Existing User'");
		thirdUserConditions.insert(db->changeNameCase("Id") + " = -3 OR " + db->changeNameCase("Name") + " = 'Third User'");
		thirdUserConditions.insert("NOT (" + db->changeNameCase("Id") + " <> 3)");
		thirdUserConditions.insert("NOT (" + db->changeNameCase("Name") + " <> 'Third User')");
		thirdUserConditions.insert("NOT (" + db->changeNameCase("Id") + " <> 3 OR " + db->changeNameCase("Name") + " <> 'Third User')");
	}

	for (const std::string &condition : thirdUserConditions) {
		TempFile dataDump_filteredUsingWhere = temp.createFile();

		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		db->get().exportData(db->changeNameCase("User"), condition, dataDump_filteredUsingWhere.path());

		TextDiff diff(dataDump_expected.path(), dataDump_filteredUsingWhere.path());

		BOOST_REQUIRE_MESSAGE(diff.areEqual(), "Failed with condition: " + condition);
	}
}
