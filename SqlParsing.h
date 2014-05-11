#pragma once

#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "SqlStructures.h"

namespace sql_parsing {

template <typename Iterator>
struct CommonRules {
	CommonRules() {
		using boost::spirit::qi::_val;
		using boost::spirit::qi::_1;
		using boost::spirit::qi::int_;
		using boost::spirit::qi::alpha;
		using boost::spirit::qi::digit;
		using boost::spirit::qi::graph;
		using boost::spirit::lexeme;
		using boost::spirit::as_string;

		value = literal[_val = _1] | column[_val = _1];

		literal = int_[_val = _1] | quoted_string[_val = _1];
		column = lexeme[as_string[(alpha | '_' | '@' | '#') >> *(alpha | digit | '_' | '@' | '#' | '$')]][_val = _1];

		quoted_string = lexeme[
			('\'' >> as_string[*(graph - '\'')][_val += _1] >> '\'')
			>> *('\'' >> as_string[*(graph - '\'')][_val += ("\'" + _1)] >> '\'')
		];
	}

	template<typename T>
	using rule = boost::spirit::qi::rule<Iterator, T, boost::spirit::ascii::space_type>;

	rule<sql::Value()> value;
	rule<sql::Literal()> literal;
	rule<sql::Column()> column;

	rule<std::string()> quoted_string;
};

}
