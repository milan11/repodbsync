#include <boost/test/unit_test.hpp>

#include "../DatabaseTypes.h"
#include "DatabaseFixture.h"

BOOST_AUTO_TEST_CASE(open) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);
}

BOOST_AUTO_TEST_CASE(not_versioned) {
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

	BOOST_CHECK_EQUAL(db.get().getVersion(), 1);
}
