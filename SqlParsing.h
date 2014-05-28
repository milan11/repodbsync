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
		using boost::spirit::qi::char_;
		using boost::spirit::qi::alpha;
		using boost::spirit::qi::digit;
		using boost::spirit::qi::graph;
		using boost::spirit::lexeme;
		using boost::spirit::as_string;

		value = literal[_val = _1] | column[_val = _1];

		literal = int_[_val = _1] | quotedString[_val = _1];

		auto columnName = (alpha | '_' | '@' | '#') >> *(alpha | digit | '_' | '@' | '#' | '$');
		column = lexeme[as_string[columnName][_val = _1] | ('"' >> as_string[columnName][_val = _1] >> '"')];

		char quote = '\'';
		char escapeChar = '\\';

		auto escapedChar = (escapeChar >> char_[_val += _1]);

		auto partInQuotes = *(escapedChar | (~char_(quote))[_val += _1]);
		quotedString = lexeme[
			(quote >> as_string[partInQuotes] >> quote)
			>> *(char_(quote)[_val += quote] >> as_string[partInQuotes] >> quote)
		];
	}

	template<typename T>
	using rule = boost::spirit::qi::rule<Iterator, T, boost::spirit::ascii::space_type>;

	rule<sql::Value()> value;
	rule<sql::Literal()> literal;
	rule<sql::Column()> column;

	rule<std::string()> quotedString;
};

}
