#pragma once

#include <ostream>
#include <boost/variant/static_visitor.hpp>
#include "SqlStructures_Condition.h"

class SqlPrinter_Condition : public boost::static_visitor<> {
public:
	SqlPrinter_Condition(std::ostream &s);

public:
	void operator()(const sql::condition::Or &c) const;
	void operator()(const sql::condition::And &c) const;
	void operator()(const sql::condition::Not &c) const;
	void operator()(const sql::condition::Comparison &c) const;

private:
	void printList(const sql::condition::List &c, const std::string &separator) const;

private:
	std::ostream &s;

};
