#include "SqlParsing_Condition.h"

#include "exceptions.h"

namespace sql_parsing {

sql::condition::Condition parseCondition(const std::string &str) {
	ConditionGrammar<std::string::const_iterator> grammar;

	std::string::const_iterator iter = str.begin();
	std::string::const_iterator end = str.end();
	sql::condition::Condition condition;
	const bool result = phrase_parse(iter, end, grammar, boost::spirit::ascii::space, condition);

	bool ok = (result && (iter == end));

	if (! ok) {
		THROW(boost::format("Unable to parse condition: %1%; at %2%") % str % std::string(iter, end));
	}

	return condition;
}

}
