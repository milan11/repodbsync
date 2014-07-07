#include "DataFilter.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include "SqlParsing_Insert.h"
#include "SqlPrinting.h"
#include "exceptions.h"

ConditionEvaluator::ConditionEvaluator(const sql::insert::Insert &insert)
:
	insert(insert),
	result(false)
{
}

bool ConditionEvaluator::getResult() const {
	return result;
}

void ConditionEvaluator::operator()(const sql::condition::Or &c) {
	result = false;

	for (std::vector<sql::condition::Condition>::const_iterator it = c.conditions.begin(); it != c.conditions.end(); ++it) {
		ConditionEvaluator v(insert);
		boost::apply_visitor(v, *it);

		if (v.getResult()) {
			result = true;
			break;
		}
	}
}

void ConditionEvaluator::operator()(const sql::condition::And &c) {
	result = true;

	for (std::vector<sql::condition::Condition>::const_iterator it = c.conditions.begin(); it != c.conditions.end(); ++it) {
		ConditionEvaluator v(insert);
		boost::apply_visitor(v, *it);

		if (! v.getResult()) {
			result = false;
			break;
		}
	}
}

void ConditionEvaluator::operator()(const sql::condition::Not &c) {
	ConditionEvaluator v(insert);
	boost::apply_visitor(v, c.condition);

	result = ! v.getResult();
}

namespace {
	template<typename T>
	bool applyOperator(const T &a, const T &b, const sql::condition::Operator &op) {
		switch (op) {
			case sql::condition::Operator::EQUAL: return a == b;
			case sql::condition::Operator::NOT_EQUAL: return a != b;
			case sql::condition::Operator::LESS: return a < b;
			case sql::condition::Operator::GREATER: return a > b;
			case sql::condition::Operator::LESS_OR_EQUAL: return a <= b;
			case sql::condition::Operator::GREATER_OR_EQUAL: return a >= b;
			default: THROW("Unsupported operator");
		}
	}
}

void ConditionEvaluator::operator()(const sql::condition::Comparison &c) {
	const sql::Literal &value1 = getLiteral(c.value1);
	const sql::Literal &value2 = getLiteral(c.value2);

	if (value1.which() != value2.which()) {
		std::ostringstream value1str;
		::printLiteral(value1, value1str);
		std::ostringstream value2str;
		::printLiteral(value2, value2str);
		THROW(boost::format("Data filter comparison - incompatible types: %1%, %2%") % value1str.str() % value2str.str());
	}

	switch (value1.which()) {
		case 0: result = ::applyOperator(boost::get<int64_t>(value1), boost::get<int64_t>(value2), c.op); break;
		case 1: result = ::applyOperator(boost::get<std::string>(value1), boost::get<std::string>(value2), c.op); break;
		default: THROW(boost::format("Unsupported type index in literal: %1%") % value1.which());
	}
}

const sql::Literal &ConditionEvaluator::getLiteral(const sql::Value &c) {
	switch (c.which()) {
		case 0:
			return boost::get<sql::Literal>(c);
		case 1:
			{
				const sql::Column &column = boost::get<sql::Column>(c);
				for (const sql::insert::Item &item : insert.items) {
					if (item.column == column) {
						return item.value;
					}
				}
				THROW(boost::format("Unable to find column in insert while evaluating data filter condition: %1%") % column);
			}
		default:
			THROW(boost::format("Invalid type."));
	}
}

namespace {
	bool isInsert(const std::string &potentialInsert) {
		std::string toCompare = potentialInsert;
		std::transform(toCompare.begin(), toCompare.end(), toCompare.begin(), ::tolower);

		toCompare.erase(remove_if(toCompare.begin(), toCompare.end(), ::isspace), toCompare.end());

		return (toCompare.find("insertinto") == 0);
	}
}

bool isInsertAndMatchesWhere(const std::string &potentialInsert, const sql::condition::Condition &condition) {
	if (! isInsert(potentialInsert)) {
		return false;
	}

	std::string trimmedInsert = boost::trim_copy(potentialInsert);

	if ((! trimmedInsert.empty()) && (trimmedInsert[trimmedInsert.size() - 1] == ';')) {
		trimmedInsert.pop_back();
	}

	sql::insert::Insert insert = sql_parsing::parseInsert(trimmedInsert);

	ConditionEvaluator v(insert);
	boost::apply_visitor(v, condition);
	return v.getResult();
}
