// test_formula_translation.cpp
#include "../src/tge/formula/parser.h"
#include <iostream>
#include <vector>
#include <string>

using namespace tge::formula;

void run_test(const std::string& formula) {
    try {
        std::map<std::string,int> params; // empty for numeric-only samples
        int result = parseAndEvaluateExpression(formula, params);
        std::cout << "Formula: '" << formula << "' => " << result << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Failed to parse/evaluate '" << formula << "': " << e.what() << std::endl;
    }
}

int main() {
    std::vector<std::string> formulas = {
        "2",
        "(2)",
        "((2)+(3))"
    };
    for (const auto& f : formulas) {
        run_test(f);
    }
    return 0;
}
