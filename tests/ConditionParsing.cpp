#define BOOST_TEST_MODULE ConditionParsing
#include <boost/test/unit_test.hpp>

#include "../SqlParsing_Condition.h"
#include "../SqlPrinting_Condition.h"

BOOST_AUTO_TEST_CASE(and_or_priority)
{
	std::string str = "x = 1 or y = 2 and z = 3";
	sql::condition::Condition condition = sql_parsing::parseCondition(str);

	std::ostringstream result;
	boost::apply_visitor(SqlPrinter_Condition(result), condition);

	BOOST_CHECK_EQUAL(result.str(), "([x]=1 OR ([y]=2 AND [z]=3))");
}

