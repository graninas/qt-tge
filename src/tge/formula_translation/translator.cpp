#include "translator.h"
#include "parser_arith.h"
#include <cmath>
#include <memory>
#include <iostream> // Include iostream for debug output

namespace tge::formula_translation {

// Helper for rounding floats to 3 decimals
static double round3(double x) { return std::round(x * 1000.0) / 1000.0; }

std::variant<EvaluationModel, std::string> parse_formula_impl(const std::string &formula)
{
    std::cout << "[DEBUG] Input formula: '" << formula << "'" << std::endl;
    Tokenizer tz(formula);
    std::cout << "[DEBUG] Initial token type: " << static_cast<int>(tz.current().type) << ", value: " << tz.current().value << std::endl;
    auto ast = parse_expr(tz);
    std::cout << "[DEBUG] After parse_expr, token type: " << static_cast<int>(tz.current().type) << ", pos: " << tz.pos() << std::endl;
    if (tz.current().type != Tokenizer::TokenType::End)
    {
        std::cout << "[DEBUG] Not at end: token type: " << static_cast<int>(tz.current().type) << ", pos: " << tz.pos() << std::endl;
        return std::string{"Unexpected input after end of expression"};
    }
    if (std::holds_alternative<ASTError>(ast->node))
    {
        std::cout << "[DEBUG] ASTError encountered" << std::endl;
        return std::get<ASTError>(ast->node).messages.front();
    }
    EvaluationModel model;
    model.original = formula;
    model.ast = std::move(ast);
    return model;
}

EvalResult eval_ast(const ASTNode &node)
{
    if (std::holds_alternative<ASTLiteral>(node.node))
    {
        const auto &lit = std::get<ASTLiteral>(node.node);
        Value v;
        if (lit.type == ASTLiteral::Type::Int)
        {
            v.type = Value::Type::Int;
            v.intValue = lit.intValue;
        }
        else
        {
            v.type = Value::Type::Float;
            v.floatValue = round3(lit.floatValue);
        }
        return v;
    }
    else if (std::holds_alternative<ASTBinaryOp>(node.node))
    {
        const auto &bin = std::get<ASTBinaryOp>(node.node);
        auto lres = eval_ast(*bin.left);
        auto rres = eval_ast(*bin.right);
        if (std::holds_alternative<std::vector<std::string>>(lres))
            return lres;
        if (std::holds_alternative<std::vector<std::string>>(rres))
            return rres;
        const Value &lv = std::get<Value>(lres);
        const Value &rv = std::get<Value>(rres);
        bool isFloat = lv.type == Value::Type::Float || rv.type == Value::Type::Float;
        double lval = (lv.type == Value::Type::Int) ? lv.intValue : lv.floatValue;
        double rval = (rv.type == Value::Type::Int) ? rv.intValue : rv.floatValue;
        Value result;
        switch (bin.op)
        {
        case ASTBinaryOp::Op::Add:
            if (isFloat)
            {
                result.type = Value::Type::Float;
                result.floatValue = round3(lval + rval);
            }
            else
            {
                result.type = Value::Type::Int;
                result.intValue = static_cast<int>(lval + rval);
            }
            break;
        case ASTBinaryOp::Op::Sub:
            if (isFloat)
            {
                result.type = Value::Type::Float;
                result.floatValue = round3(lval - rval);
            }
            else
            {
                result.type = Value::Type::Int;
                result.intValue = static_cast<int>(lval - rval);
            }
            break;
        case ASTBinaryOp::Op::Mul:
            if (isFloat)
            {
                result.type = Value::Type::Float;
                result.floatValue = round3(lval * rval);
            }
            else
            {
                result.type = Value::Type::Int;
                result.intValue = static_cast<int>(lval * rval);
            }
            break;
        case ASTBinaryOp::Op::Div:
            if (std::abs(rval) < 0.0001)
                return std::vector<std::string>{"Division by zero"};
            result.type = Value::Type::Float;
            result.floatValue = round3(lval / rval);
            break;
        }
        return result;
    }
    else if (std::holds_alternative<ASTCompareOp>(node.node))
    {
        const auto &cmp = std::get<ASTCompareOp>(node.node);
        auto lres = eval_ast(*cmp.left);
        auto rres = eval_ast(*cmp.right);
        if (std::holds_alternative<std::vector<std::string>>(lres))
            return lres;
        if (std::holds_alternative<std::vector<std::string>>(rres))
            return rres;
        const Value &lv = std::get<Value>(lres);
        const Value &rv = std::get<Value>(rres);
        double lval = (lv.type == Value::Type::Int) ? lv.intValue : lv.floatValue;
        double rval = (rv.type == Value::Type::Int) ? rv.intValue : rv.floatValue;
        bool result = false;
        switch (cmp.op) {
            case ASTCompareOp::Op::Eq: result = (lval == rval); break;
            case ASTCompareOp::Op::Neq: result = (lval != rval); break;
            case ASTCompareOp::Op::Lt: result = (lval < rval); break;
            case ASTCompareOp::Op::Gt: result = (lval > rval); break;
            case ASTCompareOp::Op::Le: result = (lval <= rval); break;
            case ASTCompareOp::Op::Ge: result = (lval >= rval); break;
        }
        Value v;
        v.type = Value::Type::Int;
        v.intValue = result ? 1 : 0;
        return v;
    }
    else if (std::holds_alternative<ASTLogicalOp>(node.node))
    {
        const auto &log = std::get<ASTLogicalOp>(node.node);
        auto lres = eval_ast(*log.left);
        auto rres = eval_ast(*log.right);
        if (std::holds_alternative<std::vector<std::string>>(lres))
            return lres;
        if (std::holds_alternative<std::vector<std::string>>(rres))
            return rres;
        const Value &lv = std::get<Value>(lres);
        const Value &rv = std::get<Value>(rres);
        int lval = (lv.type == Value::Type::Int) ? lv.intValue : (lv.type == Value::Type::Float ? (lv.floatValue != 0.0) : 0);
        int rval = (rv.type == Value::Type::Int) ? rv.intValue : (rv.type == Value::Type::Float ? (rv.floatValue != 0.0) : 0);
        Value v;
        v.type = Value::Type::Int;
        switch (log.op) {
            case ASTLogicalOp::Op::And: v.intValue = (lval && rval) ? 1 : 0; break;
            case ASTLogicalOp::Op::Or: v.intValue = (lval || rval) ? 1 : 0; break;
        }
        return v;
    }
    else if (std::holds_alternative<ASTError>(node.node))
    {
        return std::get<ASTError>(node.node).messages;
    }
    return std::vector<std::string>{"Unknown AST node"};
}

std::variant<Value, std::string> eval_formula_impl(const EvaluationModel &model, const Context &)
{
    if (!model.ast)
        return std::string{"No AST"};
    EvalResult res = eval_ast(*model.ast);
    if (std::holds_alternative<Value>(res))
        return std::get<Value>(res);
    return std::get<std::vector<std::string>>(res).front();
}

// Helper for deep copying ASTNode
std::unique_ptr<ASTNode> clone_ast(const std::unique_ptr<ASTNode>& node) {
    if (!node) return nullptr;
    auto newNode = std::make_unique<ASTNode>();
    if (std::holds_alternative<ASTLiteral>(node->node)) {
        newNode->node = std::get<ASTLiteral>(node->node);
    } else if (std::holds_alternative<ASTBinaryOp>(node->node)) {
        const auto& bin = std::get<ASTBinaryOp>(node->node);
        ASTBinaryOp newBin{bin.op, clone_ast(bin.left), clone_ast(bin.right)};
        newNode->node = std::move(newBin);
    } else if (std::holds_alternative<ASTError>(node->node)) {
        newNode->node = std::get<ASTError>(node->node);
    }
    return newNode;
}

// EvaluationModel methods
EvaluationModel::EvaluationModel() = default;
EvaluationModel::~EvaluationModel() = default;
EvaluationModel::EvaluationModel(const EvaluationModel &other) : original(other.original) {
    ast = clone_ast(other.ast);
}
EvaluationModel &EvaluationModel::operator=(const EvaluationModel &other) {
    if (this != &other) {
        original = other.original;
        ast = clone_ast(other.ast);
    }
    return *this;
}

} // namespace tge::formula_translation
