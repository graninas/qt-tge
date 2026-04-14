// formula_translation.h
#pragma once
#include <string>
#include <variant>
#include <optional>

namespace tge::formula_translation {

// Forward declaration
struct EvaluationModel;
struct Context;
struct Value;

// Dummy/initial parse method: returns either EvaluationModel or error string
std::variant<EvaluationModel, std::string> parse_formula(const std::string& formula);

// Dummy/initial eval method: returns either Value or error string
std::variant<Value, std::string> eval_formula(const EvaluationModel& model, const Context& ctx);

// Context structure (empty for now)
struct Context {
    // In the future: parameters, variables, etc.
};

// Value structure (can be extended later)
struct Value {
    enum class Type { Int, Float, Bool, Error };
    Type type;
    int intValue = 0;
    double floatValue = 0.0;
    bool boolValue = false;
    // For now, only one value is valid depending on type
};

// EvaluationModel structure (dummy for now)
struct EvaluationModel {
    std::string original;
};

} // namespace tge::formula_translation
