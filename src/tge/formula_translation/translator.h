#pragma once
#include <string>
#include <variant>
#include <memory>
#include <vector>

namespace tge::formula_translation {

// Value structure
struct Value {
    enum class Type { Int, Float, Bool, Error };
    Type type;
    int intValue = 0;
    double floatValue = 0.0;
    bool boolValue = false;
};

// AST for arithmetic formulas
struct ASTLiteral {
    enum class Type { Int, Float };
    Type type;
    int intValue = 0;
    double floatValue = 0.0;
};

struct ASTError {
    std::vector<std::string> messages;
};

struct ASTBinaryOp {
    enum class Op { Add, Sub, Mul, Div };
    Op op;
    std::unique_ptr<struct ASTNode> left;
    std::unique_ptr<struct ASTNode> right;
};

struct ASTNode {
    std::variant<ASTLiteral, ASTBinaryOp, ASTError> node;
};

struct EvaluationModel {
    std::string original;
    std::unique_ptr<ASTNode> ast;
    EvaluationModel();
    EvaluationModel(const EvaluationModel &other);
    EvaluationModel &operator=(const EvaluationModel &other);
    ~EvaluationModel();
};

struct Context {
    // In the future: parameters, variables, etc.
};

using EvalResult = std::variant<Value, std::vector<std::string>>;

// Implementation API (not exposed in formula_translation.h)
std::variant<EvaluationModel, std::string> parse_formula_impl(const std::string &formula);
std::variant<Value, std::string> eval_formula_impl(const EvaluationModel &model, const Context &ctx);

} // namespace tge::formula_translation
