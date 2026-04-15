// formula_translation.cpp
#include "formula_translation.h"
#include "translator.h"

namespace tge::formula_translation {

std::variant<EvaluationModel, std::string> parse_formula(const std::string& formula) {
    return parse_formula_impl(formula);
}

std::variant<Value, std::string> eval_formula(const EvaluationModel& model, const Context& ctx) {
    return eval_formula_impl(model, ctx);
}

} // namespace tge::formula_translation
