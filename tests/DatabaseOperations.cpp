#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/mpl/list.hpp>
#include "../DatabaseTypes.h"
#include "../TextDiff.h"
#include "../exceptions.h"
#include "DatabaseFixture.h"

template <const DatabaseType type, const bool lowerCaseNames = false>
struct Fixture {
	static std::unique_ptr<DatabaseFixture> get() { return std::unique_ptr<DatabaseFixture>(new DatabaseFixture(type, lowerCaseNames)); }
};

typedef boost::mpl::list<Fixture<DatabaseType::MYSQL>, Fixture<DatabaseType::POSTGRESQL, false>, Fixture<DatabaseType::POSTGRESQL, true>, Fixture<DatabaseType::SQLITE> > Fixtures;

BOOST_AUTO_TEST_CASE_TEMPLATE(open, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(fill_data, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(clear_database, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	db->get().clear();

	{
		std::set<std::string> tables = db->get().getTables();

		std::set<std::string> expectedTables;

		BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
	}

	{
		std::set<std::string> routines = db->get().getRoutines();

		std::set<std::string> expectedRoutines;

		BOOST_CHECK_EQUAL_COLLECTIONS(routines.begin(), routines.end(), expectedRoutines.begin(), expectedRoutines.end());
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(clear_database_is_not_versioned, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->get().makeVersioned();
	db->get().setVersion(1);

	db->get().clear();

	BOOST_CHECK_EQUAL(db->get().isVersioned(), false);
	BOOST_CHECK_THROW(db->get().getVersion(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(output_for_review, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	const std::string databaseTypeName = DatabaseTypes().toString(db->getType());

	boost::filesystem::path reviewDirectory = "review_" + databaseTypeName + (db->isLowerCaseNames() ? "_lc" : "");
	if (boost::filesystem::exists(reviewDirectory)) {
		if ((! boost::filesystem::is_directory(reviewDirectory))) {
			THROW("Review file exists but is not a directory.");
		}

		for (boost::filesystem::directory_iterator it_end, it(reviewDirectory); it != it_end; ++it) {
			if (it->path().extension() == ".sql") {
				boost::filesystem::remove(*it);
			} else {
				THROW(boost::format("Unknown file in review directory: %1%") % it->path().string());
			}
		}
	} else {
		try {
			boost::filesystem::create_directory(reviewDirectory);
		} HANDLE_RETHROW(boost::format("Unable to create directory: %1%") % reviewDirectory.string());
	}

	db->fillDataA();

	std::set<std::string> tables = db->get().getTables();

	for (const std::string &table : tables) {
		db->get().exportTable(table, reviewDirectory / (table + "_table.sql"));
		db->get().exportData(table, "", reviewDirectory / (table + "_data.sql"));
	}

	std::set<std::string> routines = db->get().getRoutines();
	for (const std::string &routine : routines) {
		db->get().exportRoutine(routine, reviewDirectory / (routine + "_routine.sql"));
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(initially_not_versioned, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	BOOST_CHECK_EQUAL(db->get().isVersioned(), false);
	BOOST_CHECK_THROW(db->get().getVersion(), std::runtime_error);
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
	BOOST_CHECK_THROW(db->get().getVersion(), std::runtime_error);
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

	std::set<std::string> expectedTables {
		db->changeNameCase("Message"),
		db->changeNameCase("User"),
		db->changeNameCase("UserRole"),
		db->changeNameCase("CyclicA"),
		db->changeNameCase("CyclicB")
	};

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_routines_empty, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	std::set<std::string> routines = db->get().getRoutines();

	std::set<std::string> expectedRoutines;

	BOOST_CHECK_EQUAL_COLLECTIONS(routines.begin(), routines.end(), expectedRoutines.begin(), expectedRoutines.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_routines, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	if (db->getType() == DatabaseType::SQLITE) {
		return;
	}

	db->fillDataA();

	std::set<std::string> routines = db->get().getRoutines();

	std::set<std::string> expectedRoutines;

	if (db->getType() == DatabaseType::MYSQL) {
		expectedRoutines = {
			db->changeNameCase("procedure_" + db->changeNameCase("UserCount")),
			db->changeNameCase("function_" + db->changeNameCase("UserCount"))
		};
	}
	else if (db->getType() == DatabaseType::POSTGRESQL) {
		expectedRoutines = {
			db->changeNameCase(db->changeNameCase("UserCount_p") + "(integer)"),
			db->changeNameCase(db->changeNameCase("UserCount_pout") + "(integer)"),
			db->changeNameCase(db->changeNameCase("UserCount_f") + "()"),
			db->changeNameCase(db->changeNameCase("TriggerFunction") + "()")
		};
	}
	else {
		THROW("Unsupported database in this test case");
	}

	BOOST_CHECK_EQUAL_COLLECTIONS(routines.begin(), routines.end(), expectedRoutines.begin(), expectedRoutines.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_tables_does_not_include_versioninfo, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	db->get().makeVersioned();
	db->get().setVersion(1);

	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables {
		db->changeNameCase("Message"),
		db->changeNameCase("User"),
		db->changeNameCase("UserRole"),
		db->changeNameCase("CyclicA"),
		db->changeNameCase("CyclicB")
	};

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_table_dependencies_user, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> dependencies = db->get().getTableDependencies(db->changeNameCase("User"));

	std::set<std::string> expectedDependencies {
		db->changeNameCase("UserRole")
	};

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_table_dependencies_message, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> dependencies = db->get().getTableDependencies(db->changeNameCase("Message"));

	std::set<std::string> expectedDependencies {
		db->changeNameCase("User")
	};

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(get_table_dependencies_empty, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	std::set<std::string> dependencies = db->get().getTableDependencies(db->changeNameCase("UserRole"));

	std::set<std::string> expectedDependencies;

	BOOST_CHECK_EQUAL_COLLECTIONS(dependencies.begin(), dependencies.end(), expectedDependencies.begin(), expectedDependencies.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_structure, F, Fixtures) {
	Temp temp("temp_test");

	TempFile dump = temp.createFile();
	TempFile triggerFunction = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("TriggerFunction") + "()", triggerFunction.path());
		}
		db->get().exportTable(db->changeNameCase("UserRole"), dump.path());
	}

	TempFile dumpAfterImport = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().import(triggerFunction.path());
		}
		db->get().import(dump.path());

		std::set<std::string> tables = db->get().getTables();

		std::set<std::string> expectedTables {
			db->changeNameCase("UserRole")
		};

		BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());

		db->get().exportTable(db->changeNameCase("UserRole"), dumpAfterImport.path());
	}

	TextDiff diff(dump.path(), dumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_data, F, Fixtures) {
	Temp temp("temp_test");

	TempFile structureDump = temp.createFile();
	TempFile dataDump = temp.createFile();
	TempFile triggerFunction = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("TriggerFunction") + "()", triggerFunction.path());
		}
		db->get().exportTable(db->changeNameCase("UserRole"), structureDump.path());
		db->get().exportData(db->changeNameCase("UserRole"), "", dataDump.path());
	}

	TempFile dataDumpAfterImport = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().import(triggerFunction.path());
		}
		db->get().import(structureDump.path());
		db->get().import(dataDump.path());

		db->get().exportData(db->changeNameCase("UserRole"), "", dataDumpAfterImport.path());
	}

	TextDiff diff(dataDump.path(), dataDumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_routine, F, Fixtures) {
	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		if (db->getType() == DatabaseType::SQLITE) {
			return;
		}
	}

	Temp temp("temp_test");

	TempFile routineDump = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		if (db->getType() == DatabaseType::MYSQL) {
			db->get().exportRoutine("procedure_" + db->changeNameCase("UserCount") , routineDump.path());
		}
		else if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("UserCount_p") + "(integer)", routineDump.path());
		}
		else {
			THROW("Unsupported database in this test case");
		}
	}

	TempFile routineDumpAfterImport = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->get().import(routineDump.path());

		std::set<std::string> routines = db->get().getRoutines();

		std::set<std::string> expectedRoutines;

		if (db->getType() == DatabaseType::MYSQL) {
			expectedRoutines = {
				db->changeNameCase("procedure_" + db->changeNameCase("UserCount"))
			};
		}
		else if (db->getType() == DatabaseType::POSTGRESQL) {
			expectedRoutines = {
				db->changeNameCase(db->changeNameCase("UserCount_p") + "(integer)")
			};
		}
		else {
			THROW("Unsupported database in this test case");
		}

		BOOST_CHECK_EQUAL_COLLECTIONS(routines.begin(), routines.end(), expectedRoutines.begin(), expectedRoutines.end());

		if (db->getType() == DatabaseType::MYSQL) {
			db->get().exportRoutine("procedure_" + db->changeNameCase("UserCount"), routineDumpAfterImport.path());
		}
		else if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("UserCount_p") + "(integer)", routineDumpAfterImport.path());
		}
		else {
			THROW("Unsupported database in this test case");
		}
	}

	TextDiff diff(routineDump.path(), routineDumpAfterImport.path());
	BOOST_CHECK_EQUAL(diff.areEqual(), true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(export_import_all, F, Fixtures) {
	Temp temp("temp_test");

	TempFile structureDump = temp.createFile();
	TempFile dataDump = temp.createFile();
	TempFile routineDump = temp.createFile();
	TempFile triggerFunction = temp.createFile();

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		db->fillDataA();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("TriggerFunction") + "()", triggerFunction.path());
		}
		db->get().exportTable(db->changeNameCase("UserRole"), structureDump.path());
		db->get().exportData(db->changeNameCase("UserRole"), "", dataDump.path());
		if (db->getType() == DatabaseType::MYSQL) {
			db->get().exportRoutine("procedure_" + db->changeNameCase("UserCount"), routineDump.path());
		}
		else if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().exportRoutine(db->changeNameCase("UserCount_p") + "(integer)", routineDump.path());
		}
		else if (db->getType() == DatabaseType::SQLITE) {
			// nothing
		} else {
			THROW("Unsupported database in this test case");
		}
	}

	{
		std::unique_ptr<DatabaseFixture> db = F::get();

		if (db->getType() == DatabaseType::POSTGRESQL) {
			db->get().import(triggerFunction.path());
		}
		db->get().import(structureDump.path());
		db->get().import(dataDump.path());
		if (db->getType() != DatabaseType::SQLITE) {
			db->get().import(routineDump.path());
		}
	}
}

BOOST_AUTO_TEST_CASE_TEMPLATE(print_delete_table, F, Fixtures) {
	Temp temp("temp_test");

	std::unique_ptr<DatabaseFixture> db = F::get();

	db->fillDataA();

	TempFile deleteTable = temp.createFile();
	db->get().printDeleteTable(db->changeNameCase("Message"), deleteTable.path());

	db->get().import(deleteTable.path());


	std::set<std::string> tables = db->get().getTables();

	std::set<std::string> expectedTables {
		db->changeNameCase("User"),
		db->changeNameCase("UserRole"),
		db->changeNameCase("CyclicA"),
		db->changeNameCase("CyclicB")
	};

	BOOST_CHECK_EQUAL_COLLECTIONS(tables.begin(), tables.end(), expectedTables.begin(), expectedTables.end());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(print_delete_routine, F, Fixtures) {
	std::unique_ptr<DatabaseFixture> db = F::get();

	if (db->getType() == DatabaseType::SQLITE) {
		return;
	}

	Temp temp("temp_test");

	db->fillDataA();

	TempFile deleteRoutine = temp.createFile();
	if (db->getType() == DatabaseType::MYSQL) {
		db->get().printDeleteRoutine("procedure_" + db->changeNameCase("UserCount"), deleteRoutine.path());
	}
	else if (db->getType() == DatabaseType::POSTGRESQL) {
		db->get().printDeleteRoutine(db->changeNameCase("UserCount_p") + "(integer)", deleteRoutine.path());
	}
	else {
		THROW("Unsupported database in this test case");
	}

	db->get().import(deleteRoutine.path());


	std::set<std::string> routines = db->get().getRoutines();

	std::set<std::string> expectedRoutines;

	if (db->getType() == DatabaseType::MYSQL) {
		expectedRoutines = {
			db->changeNameCase("function_" + db->changeNameCase("UserCount"))
		};
	}
	else if (db->getType() == DatabaseType::POSTGRESQL) {
		expectedRoutines = {
			db->changeNameCase(db->changeNameCase("UserCount_pout") + "(integer)"),
			db->changeNameCase(db->changeNameCase("UserCount_f") + "()"),
			db->changeNameCase(db->changeNameCase("TriggerFunction") + "()")
		};
	}
	else {
		THROW("Unsupported database in this test case");
	}

	BOOST_CHECK_EQUAL_COLLECTIONS(routines.begin(), routines.end(), expectedRoutines.begin(), expectedRoutines.end());
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

		const std::string thirdUserNameSqlString = "'Third User \" '' \\\\'";
		const std::string thirdUserNameSqlString_escape = "'Third User \\\" \\' \\\\'";

		thirdUserConditions = std::set<std::string> {
			db->changeNameCase("Id") + " = 3",
			db->changeNameCase("Name") + " = " + thirdUserNameSqlString,
			db->changeNameCase("Name") + " = " + thirdUserNameSqlString_escape,
			db->changeNameCase("Id") + " = 3 AND " + db->changeNameCase("Name") + " = " + thirdUserNameSqlString,
			db->changeNameCase("Id") + " = 3 OR " + db->changeNameCase("Name") + " = 'Non Existing User'",
			db->changeNameCase("Id") + " = -3 OR " + db->changeNameCase("Name") + " = " + thirdUserNameSqlString,
			"NOT (" + db->changeNameCase("Id") + " <> 3)",
			"NOT (" + db->changeNameCase("Name") + " <> " + thirdUserNameSqlString + ")",
			"NOT (" + db->changeNameCase("Id") + " <> 3 OR " + db->changeNameCase("Name") + " <> " + thirdUserNameSqlString + ")"
		};
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
