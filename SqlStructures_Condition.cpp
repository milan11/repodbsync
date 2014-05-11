#include "SqlStructures_Condition.h"

namespace sql {
namespace condition {

void Comparison::setValue1(Value value) {
	value1 = value;
}

void Comparison::setOperator(Operator op) {
	this->op = op;
}

void Comparison::setValue2(Value value) {
	value2 = value;
}

void List::add(Condition condition) {
	conditions.push_back(std::move(condition));
}

void Not::set(Condition condition) {
	this->condition = std::move(condition);
}

}
}
