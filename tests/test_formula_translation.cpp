// test_formula_translation.cpp
#include "formula_translation.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <variant>

using tge::formula_translation::parse_formula;
using tge::formula_translation::eval_formula;
using tge::formula_translation::EvaluationModel;
using tge::formula_translation::Context;
using tge::formula_translation::Value;

int main() {
    struct TestCase {
        std::string formula;
        double expected;
    };
    std::vector<TestCase> tests = {
        {"1+2", 3.0},
        {"2*3+4", 10.0},
        {"2+3*4", 14.0},
        {"(2+3)*4", 20.0},
        {"10-2*3", 4.0},
        {"10/(2+3)", 2.0},
        {"neg(5)+2", -3.0},
        {"2*neg(3)", -6.0},
        {"2.5+1.5", 4.0},
        {"6/4", 1.5},
        {"(1+2)*(3+4)", 21.0},
    };
    int passed = 0;

    for (const auto& test : tests) {
        auto parseResult = parse_formula(test.formula);
        if (parseResult.index() == 1) {
            std::cout << "Parse error for '" << test.formula << "': " << std::get<1>(parseResult) << std::endl;
            continue;
        }
        const EvaluationModel& model = std::get<0>(parseResult);
        Context ctx;
        auto evalResult = eval_formula(model, ctx);
        if (evalResult.index() == 1) {
            std::cout << "Eval error for '" << test.formula << "': " << std::get<1>(evalResult) << std::endl;
            continue;
        }
        const Value& v = std::get<0>(evalResult);
        double actual = (v.type == Value::Type::Int) ? v.intValue : v.floatValue;
        if (std::abs(actual - test.expected) < 0.001) {
            ++passed;
        } else {
            std::cout << "Test failed for '" << test.formula << "': expected " << test.expected << ", got " << actual << std::endl;
        }
    }
    std::cout << passed << "/" << tests.size() << " tests passed." << std::endl;
    return (passed == tests.size()) ? 0 : 1;
}
