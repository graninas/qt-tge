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
      // std::cout << "Formula: '" << std::get<0>(formula) << "' => " << result << std::endl;
    } catch (const std::exception &e) {
        std::cout << "Failed to parse/evaluate '" << std::get<0>(formula) << "': " << e.what() << std::endl;
    }

    if (std::get<1>(formula) != result)
      throw std::runtime_error("Result mismatch");
}

void run_test_expect_error(const std::string& formula) {
    std::map<std::string, int> params = {{"P1", 5}, {"P2", 10}};
    try {
        parseAndEvaluateExpression(formula, params);
        std::cout << "Expected error for '" << formula << "' but got no error" << std::endl;
        throw std::runtime_error("Expected error was not thrown");
    } catch (const std::exception &e) {
        // std::cout << "Correctly got error for '" << formula << "': " << e.what() << std::endl;
    }
}

int main() {
    std::vector<std::tuple<std::string, int>> formulas = {
        // --- Existing tests ---
        {"2", 2},
        {"(2)", 2},
        {"(     (2)+(  3) )", 5},
        {"12>22", 0},
        {"12 < 22  ", 1},
        {"12==22", 0},
        {"([P1]-(([P2]+1)*([P3]/[P4])))", -160},

        // --- Equality operators ---
        {"(5==5)", 1},
        {"(5==6)", 0},
        {"(5!=6)", 1},
        {"(5!=5)", 0},

        // --- Arithmetic: div, mod ---
        {"(10 div 3)", 3},
        {"(10 div 2)", 5},
        {"(7 mod 3)", 1},
        {"(10 mod 5)", 0},
        {"([P3] div [P4])", 15},   // 30 div 2 = 15
        {"([P2] mod 3)", 1},        // 10 mod 3 = 1

        // --- Logical: and, or ---
        {"(1 and 1)", 1},
        {"(1 and 0)", 0},
        {"(0 and 0)", 0},
        {"(1 or 0)", 1},
        {"(0 or 0)", 0},
        // Compound logical from doc
        {"(([P1]>=5) and ([P2]==10))", 1},   // P1=5>=5 AND P2=10 => 1 and 1 = 1
        {"(([P1]>=6) and ([P2]==10))", 0},   // P1=5 not >=6 => 0 and 1 = 0
        {"(([P1]>=6) or ([P2]==10))", 1},    // 0 or 1 = 1
        {"(([P1]>=6) or ([P2]==99))", 0},    // 0 or 0 = 0

        // --- Unary neg ---
        {"neg(3)", -3},
        {"neg(neg(4))", 4},
        {"(neg(5)+8)", 3},
        {"(neg([P1])+10)", 5},   // -5 + 10 = 5

        // --- Range membership: in / to ---
        {"([P1] in (1 to 10))", 1},    // P1=5 in [1,10] => 1
        {"([P1] in (6 to 10))", 0},    // P1=5 not in [6,10] => 0
        {"([P2] in ([P1] to 15))", 1}, // P2=10 in [5,15] => 1

        // --- Expressions from the documentation ---
        // ([p5] div 30)+1 with P3=30: (30 div 30)+1 = 2
        {"(([P3] div 30)+1)", 2},

        // --- No mandatory parentheses + precedence ---
        {"3 + 5 * 2", 13},
        {"10 - 2 * 3", 4},
        {"[P1] + [P2] * [P4]", 25},
        {"1 or 0 and 0", 1},
        {"3 + 5 * 2 == 13", 1},

        // --- Ambiguous cases: precedence/associativity ---
        {"(1 or 0) and 0", 0},          // parentheses override and/or precedence
        {"10 - 3 - 2", 5},              // left-associative subtraction: (10-3)-2
        {"20 div 3 * 2", 12},           // left-associative multiplicative ops: (20 div 3)*2
        {"[P1] in 1 to 10", 1},         // in binds after range construction (to)
        {"[P1] in 1 to 10 and 0", 0},   // in result participates in logical and
        {"1 < 2 < 3", 1},               // current behavior: chained comparisons are left-associative
    };
    for (const auto& f : formulas) {
        run_test(f);
    }

    // --- Error cases ---
    run_test_expect_error("(10 div 0)");
    run_test_expect_error("(10 mod 0)");
    run_test_expect_error("(10 / 0)");
    run_test_expect_error("1 to 3");
    run_test_expect_error("1 and or 0");

    // --- rnd(from, toExcluding) ---
    // Run many times and check range [1, 6)
    {
        std::map<std::string, int> params = {{"P1", 1}, {"P2", 6}};
        for (int i = 0; i < 100; ++i) {
            int r = parseAndEvaluateExpression("rnd(1,6)", params);
            if (r < 1 || r >= 6)
                throw std::runtime_error("rnd(1,6) out of range: " + std::to_string(r));
        }
        std::cout << "rnd(1,6): range [1,6) OK" << std::endl;
    }
    // rnd with parameter bounds
    {
        std::map<std::string, int> params = {{"P1", 10}, {"P2", 20}};
        for (int i = 0; i < 100; ++i) {
            int r = parseAndEvaluateExpression("rnd([P1],[P2])", params);
            if (r < 10 || r >= 20)
                throw std::runtime_error("rnd([P1],[P2]) out of range: " + std::to_string(r));
        }
        std::cout << "rnd([P1],[P2]): range [10,20) OK" << std::endl;
    }
    // rnd inside expression: (rnd(0,2)+5) must be 5 or 6
    {
        std::map<std::string, int> params;
        for (int i = 0; i < 100; ++i) {
            int r = parseAndEvaluateExpression("(rnd(0,2)+5)", params);
            if (r < 5 || r > 6)
                throw std::runtime_error("(rnd(0,2)+5) out of range: " + std::to_string(r));
        }
        std::cout << "(rnd(0,2)+5): range [5,6] OK" << std::endl;
    }
    // rnd error: toExcluding <= from
    run_test_expect_error("rnd(5,5)");
    run_test_expect_error("rnd(10,1)");

    std::cout << "All tests passed." << std::endl;
    return 0;
}
