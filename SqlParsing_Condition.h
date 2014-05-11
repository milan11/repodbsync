#pragma once

#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include "SqlParsing.h"
#include "SqlStructures_Condition.h"

namespace sql_parsing {

template <typename Iterator>
class ConditionGrammar : public boost::spirit::qi::grammar<Iterator, sql::condition::Condition(), boost::spirit::ascii::space_type> {
public:
	ConditionGrammar() : ConditionGrammar::base_type(condition)
	{
		namespace phx = boost::phoenix;

		using boost::spirit::qi::_val;
		using boost::spirit::qi::_1;
		using boost::spirit::qi::char_;
		using boost::spirit::qi::lit;
		using boost::spirit::qi::no_case;
		using boost::spirit::repository::distinct;

		using namespace sql::condition;

		#define KEYWORD(s) no_case[distinct(char_("a-zA-Z_0-9"))[s]]

		condition = parentheses[_val = _1] | condition_or[_val = _1] | condition_and[_val = _1] | condition_not[_val = _1] | condition_comparison[_val = _1];
		parentheses = lit("(") >> condition[_val = _1] >> lit(")");
		condition_inOr = parentheses[_val = _1] | condition_and[_val = _1] | condition_not[_val = _1] | condition_comparison[_val = _1];
		condition_inAnd = parentheses[_val = _1] | condition_not[_val = _1] | condition_comparison[_val = _1];
		condition_inNot = parentheses[_val = _1] | condition_comparison[_val = _1];

		condition_or =
			condition_inOr[phx::bind(&Or::add, _val, _1)]
			>> +(KEYWORD("OR") >> condition_inOr[phx::bind(&Or::add, _val, _1)])
		;

		condition_and =
			condition_inAnd[phx::bind(&And::add, _val, _1)]
			>> +(KEYWORD("AND") >> condition_inAnd[phx::bind(&And::add, _val, _1)])
		;

		condition_not =
			KEYWORD("NOT")
			>> condition_inNot[phx::bind(&Not::set, _val, _1)]
		;

		condition_comparison =
			common.value[phx::bind(&Comparison::setValue1, _val, _1)]
			>> op[phx::bind(&Comparison::setOperator, _val, _1)]
			>> common.value[phx::bind(&Comparison::setValue2, _val, _1)]
		;

		op =
			lit("=")[_val = Operator::EQUAL]
			| lit("<>")[_val = Operator::NOT_EQUAL]
			| lit("<")[_val = Operator::LESS]
			| lit(">")[_val = Operator::GREATER]
			| lit("<=")[_val = Operator::LESS_OR_EQUAL]
			| lit(">=")[_val = Operator::GREATER_OR_EQUAL]
		;
	}

private:
	template<typename T>
	using rule = boost::spirit::qi::rule<Iterator, T, boost::spirit::ascii::space_type>;

	rule<sql::condition::Condition()> condition, parentheses, condition_inOr, condition_inAnd, condition_inNot;

	rule<sql::condition::Or()> condition_or;
	rule<sql::condition::And()> condition_and;
	rule<sql::condition::Not()> condition_not;
	rule<sql::condition::Comparison()> condition_comparison;

	rule<sql::condition::Operator()> op;

	CommonRules<Iterator> common;

};

sql::condition::Condition parseCondition(const std::string &str);

}
