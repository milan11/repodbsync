#pragma once

#include <vector>
#include "SqlStructures.h"

namespace sql {
namespace condition {

struct Or;
struct And;
struct Not;
struct Comparison;

enum class Operator {
	EQUAL,
	NOT_EQUAL,
	LESS,
	GREATER,
	LESS_OR_EQUAL,
	GREATER_OR_EQUAL,
};

struct Comparison {
	Value value1;
	Operator op;
	Value value2;

	void setValue1(Value value);
	void setOperator(Operator op);
	void setValue2(Value value);
};

typedef boost::variant<
	boost::recursive_wrapper<Or>,
	boost::recursive_wrapper<And>,
	boost::recursive_wrapper<Not>,
	Comparison
> Condition;

struct List {
	std::vector<Condition> conditions;

	void add(Condition condition);
};

struct Or : public List {};
struct And : public List {};

struct Not {
	Condition condition;

	void set(Condition condition);
};

}
}
