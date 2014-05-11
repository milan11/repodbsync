#include "SqlPrinting.h"

#include <boost/algorithm/string/replace.hpp>

void printValue(const sql::Value &c, std::ostream &s) {
	switch (c.which()) {
		case 0:
			{
				sql::Literal l = boost::get<sql::Literal>(c);
				switch (l.which()) {
					case 0: s << boost::get<int64_t>(l); break;
					case 1: s << '\'' << boost::replace_all_copy(boost::get<std::string>(l), "'", "''") << '\''; break;
				}
			} break;
		case 1:
			{
				s << '[' << boost::get<sql::Column>(c) << ']';
			} break;
	}
}
