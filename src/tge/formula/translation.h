// translation.h
#pragma once
#include <string>
#include <variant>
#include <optional>
#include "translation_types.h"

namespace tge::formula::translation {


// Context structure (empty for now)
struct Context {
    // In the future: parameters, variables, etc.
};

std::variant<EvaluationModel, std::string> parse_formula(const std::string &formula);

std::variant<Value, std::string> eval_formula(const EvaluationModel &model, const Context &ctx);

} // namespace tge::formula::translation
