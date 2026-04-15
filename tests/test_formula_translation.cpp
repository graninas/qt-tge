// test_formula_translation.cpp
#include "formula_translation.h"
#include <iostream>
#include <vector>
#include <string>

using namespace tge::formula::translation;

void run_test(const std::string& formula, std::optional<Value> expected = std::nullopt, bool expect_error = false) {
    auto parsed = parse_formula(formula);
    if (std::holds_alternative<std::string>(parsed)) {
        std::cout << "[FAIL] Parse error for: '" << formula << "'\n  Error: " << std::get<std::string>(parsed) << std::endl;
        if (!expect_error) std::cout << "  (Unexpected parse error)\n";
        return;
    }
    auto model = std::get<EvaluationModel>(parsed);
    Context ctx; // empty for now
    auto result = eval_formula(model, ctx);
    if (std::holds_alternative<std::string>(result)) {
        std::cout << "[FAIL] Eval error for: '" << formula << "'\n  Error: " << std::get<std::string>(result) << std::endl;
        if (!expect_error) std::cout << "  (Unexpected eval error)\n";
        return;
    }
    Value v = std::get<Value>(result);
    if (expect_error) {
        std::cout << "[FAIL] Expected error but got value for: '" << formula << "'\n";
        return;
    }
    std::cout << "[PASS] '" << formula << "' => ";
    switch (v.type) {
        case Value::Type::Int: std::cout << v.intValue << " (int)"; break;
        case Value::Type::Float: std::cout << v.floatValue << " (float)"; break;
        case Value::Type::Bool: std::cout << (v.boolValue ? "true" : "false") << " (bool)"; break;
        case Value::Type::Error: std::cout << "<error>"; break;
    }
    if (expected) {
        // Only check int/float/bool for now
        bool ok = false;
        if (v.type == expected->type) {
            switch (v.type) {
                case Value::Type::Int: ok = (v.intValue == expected->intValue); break;
                case Value::Type::Float: ok = (std::abs(v.floatValue - expected->floatValue) < 1e-3); break;
                case Value::Type::Bool: ok = (v.boolValue == expected->boolValue); break;
                default: break;
            }
        }
        std::cout << (ok ? " [OK]" : " [WRONG]");
    }
    std::cout << std::endl;
}

int main() {
    std::vector<std::tuple<std::string, std::optional<Value>, bool>> tests = {
        // Arithmetic
        {"1+2", Value{Value::Type::Int, 3}, false},
        {"5-3", Value{Value::Type::Int, 2}, false},
        {"4*7", Value{Value::Type::Int, 28}, false},
        {"8/2", Value{Value::Type::Int, 4}, false},
        {"7 div 2", Value{Value::Type::Int, 3}, false},
        {"7 mod 4", Value{Value::Type::Int, 3}, false},
        {"1.5+2.5", Value{Value::Type::Float, 0, 4.0}, false},
        {"5/2", Value{Value::Type::Int, 2}, false}, // integer division
        {"5/2.0", Value{Value::Type::Float, 0, 2.5}, false},
        {"2.5*2", Value{Value::Type::Float, 0, 5.0}, false},
        // Division by zero
        {"5/0", std::nullopt, true},
        {"5 div 0", std::nullopt, true},
        {"5 mod 0", std::nullopt, true},
        // Comparison
        {"2=2", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"2<>3", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"2<3", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"3>2", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"2<=2", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"3>=2", Value{Value::Type::Bool, 0, 0.0, true}, false},
        // Logical
        {"1 and 0", Value{Value::Type::Bool, 0, 0.0, false}, false},
        {"1 or 0", Value{Value::Type::Bool, 0, 0.0, true}, false},
        {"(1<2) and (2<3)", Value{Value::Type::Bool, 0, 0.0, true}, false},
        // Parentheses and precedence
        {"2+3*4", Value{Value::Type::Int, 14}, false},
        {"(2+3)*4", Value{Value::Type::Int, 20}, false},
        {"2+(3*4)", Value{Value::Type::Int, 14}, false},
        // Ambiguous precedence (should error)
        {"2+3*", std::nullopt, true},
        {"2+", std::nullopt, true},
        {"(2+3", std::nullopt, true},
        {"2+3)", std::nullopt, true},
        // Unary minus not supported
        {"-2+3", std::nullopt, true},
        // Whitespace ignored
        {" 2 + 3 ", Value{Value::Type::Int, 5}, false},
    };
    for (const auto& [f, expected, expect_error] : tests) {
        run_test(f, expected, expect_error);
    }
    return 0;
}
