#pragma once

#include <boost/variant/static_visitor.hpp>
#include "SqlStructures_Condition.h"
#include "SqlStructures_Insert.h"

class ConditionEvaluator : public boost::static_visitor<> {
public:
	ConditionEvaluator(const sql::insert::Insert &insert);

public:
	bool getResult() const;

public:
	void operator()(const sql::condition::Or &c);
	void operator()(const sql::condition::And &c);
	void operator()(const sql::condition::Not &c);
	void operator()(const sql::condition::Comparison &c);

private:
	const sql::Literal &getLiteral(const sql::Value &c);

private:
	const sql::insert::Insert &insert;
	bool result;

};

bool isInsertAndMatchesWhere(const std::string &potentialInsert, const sql::condition::Condition &condition);
