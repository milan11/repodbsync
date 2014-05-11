#pragma once

#include <cstdint>
#include <string>
#include <boost/variant.hpp>

namespace sql {

typedef std::string Column;
typedef boost::variant<int64_t, std::string> Literal;

typedef boost::variant<Literal, Column> Value;

}
