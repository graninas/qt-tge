// formula_translation.cpp
#include "formula_translation.h"

namespace tge::formula_translation {

std::variant<EvaluationModel, std::string> parse_formula(const std::string& formula) {
    // Dummy implementation: just store the string
    if (formula.empty()) {
        return std::string{"Formula is empty"};
    }
    EvaluationModel model;
    model.original = formula;
    return model;
}

std::variant<Value, std::string> eval_formula(const EvaluationModel& model, const Context& /*ctx*/) {
    // Dummy implementation: always return int 42
    Value v;
    v.type = Value::Type::Int;
    v.intValue = 42;
    return v;
}

} // namespace tge::formula_translation
