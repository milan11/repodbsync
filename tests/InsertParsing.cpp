#include <boost/test/unit_test.hpp>

#include "../SqlParsing_Insert.h"
#include "../SqlPrinting_Insert.h"

namespace {

void check(const std::string &toParse, const std::string &requiredResult) {
	sql::insert::Insert insert = sql_parsing::parseInsert(toParse);

	std::ostringstream result;
	::printInsert(insert, result);

	BOOST_CHECK_EQUAL(result.str(), requiredResult);
}

}

BOOST_AUTO_TEST_CASE(insert_into) {
	::check(
		"INSERT INTO table1 (column1, column2, column3) VALUES ('abc', 2, -2)",
		"INSERT INTO table1 (column1, column2, column3) VALUES ('abc', 2, -2)"
	);
}
