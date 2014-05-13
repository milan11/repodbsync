#include "SqlParsing_Insert.h"

#include "exceptions.h"

namespace sql_parsing {

sql::insert::Insert parseInsert(const std::string &str) {
	InsertGrammar<std::string::const_iterator> grammar;

	std::string::const_iterator iter = str.begin();
	std::string::const_iterator end = str.end();
	sql::insert::Insert insert;
	const bool result = phrase_parse(iter, end, grammar, boost::spirit::ascii::space, insert);

	bool ok = (result && (iter == end));

	if (! ok) {
		THROW(boost::format("Unable to parse insert: %1%; at %2%") % str % std::string(iter, end));
	}

	return insert;
}

}
