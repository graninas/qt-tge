// test_formula_translation.cpp
#include "../src/tge/formula/parser.h"
#include <iostream>
#include <vector>
#include <string>

using namespace tge::formula;

void run_test(const std::tuple<const std::string, int>& formula) {
    int result = 0;
    try {
      std::map<std::string, int> params = {
          {"P1", 5},
          {"P2", 10},
          {"P3", 30},
          {"P4", 2}};
      result = parseAndEvaluateExpression(std::get<0>(formula), params);
      std::cout << "Formula: '" << std::get<0>(formula) << "' => " << result << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Failed to parse/evaluate '" << std::get<0>(formula) << "': " << e.what() << std::endl;
    }

    if (std::get<1>(formula) != result)
      throw std::runtime_error("Result mismatch");
}

int main() {
    std::vector<std::tuple<std::string, int>> formulas = {
        {"2", 2},
        {"(2)", 2},
        {"(     (2)+(  3) )", 5},
        {"12>22", 0},
        {"12 < 22  ", 1},
        {"12==22", 0},
        {"([P1]-(([P2]+1)*([P3]/[P4])))", -160}
    };
    for (const auto& f : formulas) {
        run_test(f);
    }
    return 0;
}
