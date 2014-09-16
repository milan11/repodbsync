#pragma once

#include <cstdint>
#include <string>
#include <boost/variant.hpp>

namespace sql {

using Column = std::string;
using Literal = boost::variant<int64_t, std::string>;

using Value = boost::variant<Literal, Column>;

}
