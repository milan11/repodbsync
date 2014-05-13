#include "SqlPrinting.h"

#include <boost/algorithm/string/replace.hpp>

void printValue(const sql::Value &c, std::ostream &s) {
	switch (c.which()) {
		case 0: ::printLiteral(boost::get<sql::Literal>(c), s); break;
		case 1: s << '[' << boost::get<sql::Column>(c) << ']'; break;
	}
}

void printLiteral(const sql::Literal &l, std::ostream &s) {
	switch (l.which()) {
		case 0: s << boost::get<int64_t>(l); break;
		case 1: s << '\'' << boost::replace_all_copy(boost::get<std::string>(l), "'", "''") << '\''; break;
	}
}
