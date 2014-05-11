#include "SqlPrinting_Condition.h"

#include "SqlPrinting.h"
#include "exceptions.h"

SqlPrinter_Condition::SqlPrinter_Condition(std::ostream &s)
	:
	  s(s)
{
}

void SqlPrinter_Condition::operator()(const sql::condition::Or &c) const {
	printList(c, " OR ");
}

void SqlPrinter_Condition::operator()(const sql::condition::And &c) const {
	printList(c, " AND ");
}

void SqlPrinter_Condition::operator()(const sql::condition::Not &c) const {
	s << " NOT ";
	s << "(";
	boost::apply_visitor(*this, c.condition);
	s << ")";
}

void SqlPrinter_Condition::operator()(const sql::condition::Comparison &c) const {
	::printValue(c.value1, s);
	switch (c.op) {
		case sql::condition::Operator::EQUAL: s << "="; break;
		case sql::condition::Operator::NOT_EQUAL: s << "<>"; break;
		case sql::condition::Operator::LESS: s << "<"; break;
		case sql::condition::Operator::GREATER: s << ">"; break;
		case sql::condition::Operator::LESS_OR_EQUAL: s << "<="; break;
		case sql::condition::Operator::GREATER_OR_EQUAL: s << ">="; break;
		default: THROW("Unsupported operator");
	}

	::printValue(c.value2, s);
}

void SqlPrinter_Condition::printList(const sql::condition::List &c, const std::string &separator) const
{
	for (std::vector<sql::condition::Condition>::const_iterator it = c.conditions.begin(); it != c.conditions.end(); ++it) {
		if (it != c.conditions.begin()) {
			s << separator;
		}
		s << "(";
		boost::apply_visitor(*this, *it);
		s << ")";
	}
}
