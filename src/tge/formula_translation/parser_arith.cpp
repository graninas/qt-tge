#include "parser_arith.h"
#include "translator.h"
#include <cctype>
#include <sstream>
#include <memory>
#include <stdexcept>

namespace tge::formula_translation {

using tge::formula_translation::ASTNode;
using tge::formula_translation::ASTLiteral;
using tge::formula_translation::ASTBinaryOp;
using tge::formula_translation::ASTCompareOp;
using tge::formula_translation::ASTLogicalOp;
using tge::formula_translation::ASTError;

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
        // Comparison operators
        case '=': current_ = {TokenType::Eq}; ++pos_; return;
        case '<':
            if (pos_ + 1 < input_.size() && input_[pos_ + 1] == '>') {
                current_ = {TokenType::Neq}; pos_ += 2; return;
            } else if (pos_ + 1 < input_.size() && input_[pos_ + 1] == '=') {
                current_ = {TokenType::Le}; pos_ += 2; return;
            } else {
                current_ = {TokenType::Lt}; ++pos_; return;
            }
        case '>':
            if (pos_ + 1 < input_.size() && input_[pos_ + 1] == '=') {
                current_ = {TokenType::Ge}; pos_ += 2; return;
            } else {
                current_ = {TokenType::Gt}; ++pos_; return;
            }
        // Logical operators (and, or) and function names
        if (std::isalpha(c)) {
            size_t start = pos_;
            while (pos_ < input_.size() && std::isalpha(input_[pos_])) ++pos_;
            std::string word = input_.substr(start, pos_ - start);
            if (word == "and") { current_ = {TokenType::And}; return; }
            if (word == "or") { current_ = {TokenType::Or}; return; }
            // For function names like neg, do not advance pos_ or set Invalid here; let parse_factor handle
            pos_ = start;
            current_ = {TokenType::Invalid};
            return;
        }
        default: current_ = {TokenType::Invalid}; ++pos_; break;
    }
}

// Forward declarations
std::unique_ptr<tge::formula_translation::ASTNode> parse_logical(Tokenizer& tz);
std::unique_ptr<tge::formula_translation::ASTNode> parse_comparison(Tokenizer& tz);
std::unique_ptr<tge::formula_translation::ASTNode> parse_term(Tokenizer& tz);
std::unique_ptr<tge::formula_translation::ASTNode> parse_arith(Tokenizer& tz);

// expr = logical
std::unique_ptr<tge::formula_translation::ASTNode> parse_expr(Tokenizer& tz) {
    auto node = parse_logical(tz);
    if (tz.current().type != Tokenizer::TokenType::End) {
        auto err = std::make_unique<tge::formula_translation::ASTNode>();
        err->node = tge::formula_translation::ASTError{{"Unexpected input after end of expression"}};
        return err;
    }
    return node;
}

// logical = comparison { (and|or) comparison }
std::unique_ptr<tge::formula_translation::ASTNode> parse_logical(Tokenizer& tz) {
    auto left = parse_comparison(tz);
    while (tz.current().type == Tokenizer::TokenType::And || tz.current().type == Tokenizer::TokenType::Or) {
        tge::formula_translation::ASTLogicalOp::Op op = (tz.current().type == Tokenizer::TokenType::And) ? tge::formula_translation::ASTLogicalOp::Op::And : tge::formula_translation::ASTLogicalOp::Op::Or;
        tz.next();
        auto right = parse_comparison(tz);
        auto node = std::make_unique<tge::formula_translation::ASTNode>();
        node->node = tge::formula_translation::ASTLogicalOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

// comparison = arith [comp_op arith]
std::unique_ptr<tge::formula_translation::ASTNode> parse_comparison(Tokenizer& tz) {
    auto left = parse_arith(tz);
    using TT = Tokenizer::TokenType;
    if (tz.current().type == TT::Eq || tz.current().type == TT::Neq || tz.current().type == TT::Lt || tz.current().type == TT::Gt || tz.current().type == TT::Le || tz.current().type == TT::Ge) {
        tge::formula_translation::ASTCompareOp::Op op;
        switch (tz.current().type) {
            case TT::Eq: op = tge::formula_translation::ASTCompareOp::Op::Eq; break;
            case TT::Neq: op = tge::formula_translation::ASTCompareOp::Op::Neq; break;
            case TT::Lt: op = tge::formula_translation::ASTCompareOp::Op::Lt; break;
            case TT::Gt: op = tge::formula_translation::ASTCompareOp::Op::Gt; break;
            case TT::Le: op = tge::formula_translation::ASTCompareOp::Op::Le; break;
            case TT::Ge: op = tge::formula_translation::ASTCompareOp::Op::Ge; break;
            default: op = tge::formula_translation::ASTCompareOp::Op::Eq; break;
        }
        tz.next();
        auto right = parse_arith(tz);
        auto node = std::make_unique<tge::formula_translation::ASTNode>();
        node->node = tge::formula_translation::ASTCompareOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

// arith = term { (+|-) term }
std::unique_ptr<tge::formula_translation::ASTNode> parse_arith(Tokenizer& tz) {
    auto left = parse_term(tz);
    while (tz.current().type == Tokenizer::TokenType::Plus || tz.current().type == Tokenizer::TokenType::Minus) {
        tge::formula_translation::ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Plus) ? tge::formula_translation::ASTBinaryOp::Op::Add : tge::formula_translation::ASTBinaryOp::Op::Sub;
        tz.next();
        auto right = parse_term(tz);
        auto node = std::make_unique<tge::formula_translation::ASTNode>();
        node->node = tge::formula_translation::ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

// Recursive descent parser for arithmetic expressions
// expr = term { (+|-) term }
// term = factor { (*|/) factor }
// factor = number | '(' expr ')'
std::unique_ptr<tge::formula_translation::ASTNode> parse_term(Tokenizer& tz);
std::unique_ptr<tge::formula_translation::ASTNode> parse_factor(Tokenizer& tz);

std::unique_ptr<tge::formula_translation::ASTNode> parse_term(Tokenizer& tz) {
    auto left = parse_factor(tz);
    while (tz.current().type == Tokenizer::TokenType::Mul || tz.current().type == Tokenizer::TokenType::Div) {
        tge::formula_translation::ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Mul) ? tge::formula_translation::ASTBinaryOp::Op::Mul : tge::formula_translation::ASTBinaryOp::Op::Div;
        tz.next();
        auto right = parse_factor(tz);
        auto node = std::make_unique<tge::formula_translation::ASTNode>();
        node->node = tge::formula_translation::ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<tge::formula_translation::ASTNode> parse_factor(Tokenizer& tz) {
    if (tz.current().type == Tokenizer::TokenType::Number) {
        double val = tz.current().value;
        tz.next();
        auto node = std::make_unique<tge::formula_translation::ASTNode>();
        if (std::abs(val - static_cast<int>(val)) < 0.0001) {
            node->node = tge::formula_translation::ASTLiteral{tge::formula_translation::ASTLiteral::Type::Int, static_cast<int>(val), val};
        } else {
            node->node = tge::formula_translation::ASTLiteral{tge::formula_translation::ASTLiteral::Type::Float, 0, val};
        }
        return node;
    } else if (tz.current().type == Tokenizer::TokenType::LParen) {
        tz.next();
        auto node = parse_expr(tz);
        if (tz.current().type == Tokenizer::TokenType::RParen) {
            tz.next();
        } else {
            auto err = std::make_unique<tge::formula_translation::ASTNode>();
            err->node = tge::formula_translation::ASTError{{"Expected ')'"}};
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
                    auto minusOne = std::make_unique<tge::formula_translation::ASTNode>();
                    minusOne->node = tge::formula_translation::ASTLiteral{tge::formula_translation::ASTLiteral::Type::Int, -1, -1.0};
                    auto node = std::make_unique<tge::formula_translation::ASTNode>();
                    node->node = tge::formula_translation::ASTBinaryOp{tge::formula_translation::ASTBinaryOp::Op::Mul, std::move(minusOne), std::move(arg)};
                    return node;
                } else {
                    auto err = std::make_unique<tge::formula_translation::ASTNode>();
                    err->node = tge::formula_translation::ASTError{{"Expected ')' after function argument"}};
                    return err;
                }
            }
        }
        auto err = std::make_unique<tge::formula_translation::ASTNode>();
        err->node = tge::formula_translation::ASTError{{"Unexpected token"}};
        return err;
    } else {
        auto err = std::make_unique<tge::formula_translation::ASTNode>();
        err->node = tge::formula_translation::ASTError{{"Unexpected token"}};
        return err;
    }
}

} // namespace tge::formula_translation
