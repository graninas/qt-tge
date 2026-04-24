// test_formula_translation.cpp
#include "../src/tge/formula/parser.h"
#include <iostream>
#include <vector>
#include <string>

using namespace tge::formula;

void run_test(const std::string& formula) {
    try {

      std::map<std::string, int> params = {
          {"P1", 5},
          {"P2", 10},
          {"P3", 30},
          {"P4", 2}};
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
        "((2)+(3))",
        "12>22",
        "12<22",
        "12==22",
        "([P1]-(([P2]+1)*([P3]/[P4])))"
    };
    for (const auto& f : formulas) {
        run_test(f);
    }
    return 0;
}
