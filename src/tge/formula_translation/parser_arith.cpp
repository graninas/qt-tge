#include "parser_arith.h"
#include "translator.h"
#include <cctype>
#include <sstream>
#include <memory>
#include <stdexcept>

namespace tge::formula_translation {

Tokenizer::Tokenizer(const std::string& input) : input_(input), pos_(0) { next(); }
const Tokenizer::Token& Tokenizer::current() const { return current_; }
void Tokenizer::next() {
    while (pos_ < input_.size() && std::isspace(input_[pos_])) ++pos_;
    if (pos_ >= input_.size()) { current_ = {TokenType::End}; return; }
    char c = input_[pos_];
    if (std::isdigit(c) || c == '.') {
        size_t start = pos_;
        while (pos_ < input_.size() && (std::isdigit(input_[pos_]) || input_[pos_] == '.')) ++pos_;
        current_ = {TokenType::Number, std::stod(input_.substr(start, pos_ - start))};
        return;
    }
    switch (c) {
        case '+': current_ = {TokenType::Plus}; ++pos_; break;
        case '-': current_ = {TokenType::Minus}; ++pos_; break;
        case '*': current_ = {TokenType::Mul}; ++pos_; break;
        case '/': current_ = {TokenType::Div}; ++pos_; break;
        case '(': current_ = {TokenType::LParen}; ++pos_; break;
        case ')': current_ = {TokenType::RParen}; ++pos_; break;
        default: current_ = {TokenType::Invalid}; ++pos_; break;
    }
}

// Recursive descent parser for arithmetic expressions
// expr = term { (+|-) term }
// term = factor { (*|/) factor }
// factor = number | '(' expr ')'
std::unique_ptr<ASTNode> parse_expr(Tokenizer& tz);
std::unique_ptr<ASTNode> parse_term(Tokenizer& tz);
std::unique_ptr<ASTNode> parse_factor(Tokenizer& tz);

std::unique_ptr<ASTNode> parse_expr(Tokenizer& tz) {
    auto left = parse_term(tz);
    while (tz.current().type == Tokenizer::TokenType::Plus || tz.current().type == Tokenizer::TokenType::Minus) {
        ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Plus) ? ASTBinaryOp::Op::Add : ASTBinaryOp::Op::Sub;
        tz.next();
        auto right = parse_term(tz);
        auto node = std::make_unique<ASTNode>();
        node->node = ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> parse_term(Tokenizer& tz) {
    auto left = parse_factor(tz);
    while (tz.current().type == Tokenizer::TokenType::Mul || tz.current().type == Tokenizer::TokenType::Div) {
        ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Mul) ? ASTBinaryOp::Op::Mul : ASTBinaryOp::Op::Div;
        tz.next();
        auto right = parse_factor(tz);
        auto node = std::make_unique<ASTNode>();
        node->node = ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNode> parse_factor(Tokenizer& tz) {
    if (tz.current().type == Tokenizer::TokenType::Number) {
        double val = tz.current().value;
        tz.next();
        auto node = std::make_unique<ASTNode>();
        if (std::abs(val - static_cast<int>(val)) < 0.0001) {
            node->node = ASTLiteral{ASTLiteral::Type::Int, static_cast<int>(val), val};
        } else {
            node->node = ASTLiteral{ASTLiteral::Type::Float, 0, val};
        }
        return node;
    } else if (tz.current().type == Tokenizer::TokenType::LParen) {
        tz.next();
        auto node = parse_expr(tz);
        if (tz.current().type == Tokenizer::TokenType::RParen) {
            tz.next();
        } else {
            auto err = std::make_unique<ASTNode>();
            err->node = ASTError{{"Expected ')'"}};
            return err;
        }
        return node;
    } else if (tz.current().type == Tokenizer::TokenType::Invalid) {
        // Try to parse function call: neg(expr)
        size_t start = tz.input().find_first_not_of(" ", tz.pos() - 1);
        if (start != std::string::npos) {
            std::string func;
            size_t i = start;
            while (i < tz.input().size() && std::isalpha(tz.input()[i])) ++i;
            func = tz.input().substr(start, i - start);
            if (func == "neg" && tz.input()[i] == '(') {
                tz.set_pos(i + 1);
                tz.next();
                auto arg = parse_expr(tz);
                if (tz.current().type == Tokenizer::TokenType::RParen) {
                    tz.next();
                    // Represent neg(x) as Mul(-1, x)
                    auto minusOne = std::make_unique<ASTNode>();
                    minusOne->node = ASTLiteral{ASTLiteral::Type::Int, -1, -1.0};
                    auto node = std::make_unique<ASTNode>();
                    node->node = ASTBinaryOp{ASTBinaryOp::Op::Mul, std::move(minusOne), std::move(arg)};
                    return node;
                } else {
                    auto err = std::make_unique<ASTNode>();
                    err->node = ASTError{{"Expected ')' after function argument"}};
                    return err;
                }
            }
        }
        auto err = std::make_unique<ASTNode>();
        err->node = ASTError{{"Unexpected token"}};
        return err;
    } else {
        auto err = std::make_unique<ASTNode>();
        err->node = ASTError{{"Unexpected token"}};
        return err;
    }
}

} // namespace tge::formula_translation
