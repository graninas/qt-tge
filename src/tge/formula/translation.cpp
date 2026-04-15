// translation.cpp
#include "translation.h"

namespace tge::formula::translation {

std::variant<EvaluationModel, std::string> parse_formula(const std::string& formula) {
    // Dummy implementation: always return an empty model
    return EvaluationModel{};
}

std::variant<Value, std::string> eval_formula(const EvaluationModel& model, const Context& /*ctx*/) {
    // Dummy implementation: always return int 42
    Value v;
    v.type = Value::Type::Int;
    v.intValue = 42;
    return v;
}

} // namespace tge::formula::translation
