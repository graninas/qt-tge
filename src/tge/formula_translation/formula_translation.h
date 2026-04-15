// formula_translation.h
#pragma once
#include <string>
#include <variant>
#include "translator.h"

namespace tge::formula_translation {

// API for parsing and evaluating formulas
std::variant<tge::formula_translation::EvaluationModel, std::string> parse_formula(const std::string &formula);
std::variant<tge::formula_translation::Value, std::string> eval_formula(const tge::formula_translation::EvaluationModel &model, const tge::formula_translation::Context &ctx);

} // namespace tge::formula_translation
