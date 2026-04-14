// test_formula_translation.cpp
#include "formula_translation.h"
#include <iostream>
#include <vector>
#include <string>

using namespace tge::formula_translation;

void run_test(const std::string& formula) {
    std::cout << "Testing formula: " << formula << std::endl;
    auto parse_result = parse_formula(formula);
    if (std::holds_alternative<std::string>(parse_result)) {
        std::cout << "  Parse error: " << std::get<std::string>(parse_result) << std::endl;
        return;
    }
    const EvaluationModel& model = std::get<EvaluationModel>(parse_result);
    Context ctx;
    auto eval_result = eval_formula(model, ctx);
    if (std::holds_alternative<std::string>(eval_result)) {
        std::cout << "  Eval error: " << std::get<std::string>(eval_result) << std::endl;
        return;
    }
    const Value& value = std::get<Value>(eval_result);
    switch (value.type) {
        case Value::Type::Int:
            std::cout << "  Result: int " << value.intValue << std::endl;
            break;
        case Value::Type::Float:
            std::cout << "  Result: float " << value.floatValue << std::endl;
            break;
        case Value::Type::Bool:
            std::cout << "  Result: bool " << (value.boolValue ? "true" : "false") << std::endl;
            break;
        default:
            std::cout << "  Result: unknown type" << std::endl;
    }
}

int main() {
    std::vector<std::string> formulas = {
        "[p1] >= ([p2]+1) * [p15]/[p7]",
        "([p1]>=30) and ([p2]=1)",
        "([p3]>80)",
        "([p1] in (0 to [p4]) and ([p2]=1))",
        "(([-2*3]) and (1<2))",
        "{([p5] div 30)+1}",
        "[1..64]",
        "[p2] to [p8]",
        "[1..49;51..98;100]",
        ""
    };
    for (const auto& f : formulas) {
        run_test(f);
    }
    return 0;
}
