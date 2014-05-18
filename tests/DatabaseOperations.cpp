#include <boost/test/unit_test.hpp>

#include "../DatabaseTypes.h"
#include "DatabaseFixture.h"

BOOST_AUTO_TEST_CASE(open) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);
}

BOOST_AUTO_TEST_CASE(not_versioned) {
	DatabaseFixture db(DatabaseType::POSTGRESQL);

	BOOST_CHECK_EQUAL(db.get().isVersioned(), false);
}
