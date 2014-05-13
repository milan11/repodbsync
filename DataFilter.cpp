#include "DataFilter.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include "SqlParsing_Condition.h"
#include "SqlPrinting_Condition.h"
#include "SqlParsing_Insert.h"
#include "SqlPrinting_Insert.h"

namespace {
	bool isInsert(const std::string &potentialInsert) {
		std::string toCompare = potentialInsert;
		std::transform(toCompare.begin(), toCompare.end(), toCompare.begin(), ::tolower);

		toCompare.erase(remove_if(toCompare.begin(), toCompare.end(), ::isspace), toCompare.end());

		return (toCompare.find("insertinto") == 0);
	}
}

bool isInsertAndMatchesWhere(const std::string &potentialInsert, const std::string &where) {
	if (! isInsert(potentialInsert)) {
		return false;
	}

	std::string trimmedInsert = boost::trim_copy(potentialInsert);

	if ((! trimmedInsert.empty()) && (trimmedInsert[trimmedInsert.size() - 1] == ';')) {
		trimmedInsert.pop_back();
	}

	sql::insert::Insert insert = sql_parsing::parseInsert(trimmedInsert);
	sql::condition::Condition condition = sql_parsing::parseCondition(where);

	return false;
}
