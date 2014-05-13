#pragma once

#include <string>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include "SqlParsing.h"
#include "SqlStructures_Insert.h"

namespace sql_parsing {

template <typename Iterator>
class InsertGrammar : public boost::spirit::qi::grammar<Iterator, sql::insert::Insert(), boost::spirit::ascii::space_type> {
public:
	InsertGrammar() : InsertGrammar::base_type(insert)
	{
		namespace phx = boost::phoenix;

		using boost::spirit::qi::_val;
		using boost::spirit::qi::_1;
		using boost::spirit::qi::char_;
		using boost::spirit::qi::lit;
		using boost::spirit::qi::no_case;
		using boost::spirit::repository::distinct;

		using namespace sql::insert;

		#define KEYWORD(s) no_case[distinct(char_("a-zA-Z_0-9"))[s]]

		insert =
			KEYWORD("INSERT") >> KEYWORD("INTO")
			>> common.column[phx::bind(&Insert::setTable, _val, _1)]
			>> lit("(")
			>> common.column[phx::bind(&Insert::addColumn, _val, _1)] % lit(",")
			>> lit(")")
			>> KEYWORD("VALUES")
			>> lit("(")
			>> common.literal[phx::bind(&Insert::addValue, _val, _1)] % lit(",")
			>> lit(")")
		;
	}

private:
	template<typename T>
	using rule = boost::spirit::qi::rule<Iterator, T, boost::spirit::ascii::space_type>;

	rule<sql::insert::Insert()> insert;

	CommonRules<Iterator> common;

};

sql::insert::Insert parseInsert(const std::string &str);

}
