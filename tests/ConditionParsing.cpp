#define BOOST_TEST_MODULE ConditionParsing
#include <boost/test/unit_test.hpp>

#include "../SqlParsing_Condition.h"
#include "../SqlPrinting_Condition.h"

namespace {

void check(const std::string &toParse, const std::string &requiredResult) {
	sql::condition::Condition condition = sql_parsing::parseCondition(toParse);

	std::ostringstream result;
	boost::apply_visitor(SqlPrinter_Condition(result), condition);

	BOOST_CHECK_EQUAL(result.str(), requiredResult);
}

}

BOOST_AUTO_TEST_CASE(and_or_priority) {
	::check(
		"x = 1 OR y = 2 AND z = 3",
		"([x]=1) OR (([y]=2) AND ([z]=3))"
	);
}

BOOST_AUTO_TEST_CASE(column_number_string) {
	::check(
		"x = a OR x = 2 OR x = -2 OR x = 'abc'",
		"([x]=[a]) OR ([x]=2) OR ([x]=-2) OR ([x]='abc')"
	);
}

BOOST_AUTO_TEST_CASE(quoted_string) {
	::check(
		"x = 'abc''def'",
		"[x]='abc''def'"
	);
}
