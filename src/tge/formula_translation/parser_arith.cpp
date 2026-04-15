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

// Remove any forward declaration of struct ASTNode here
// Use fully qualified names for all AST types
#define ASTNODE tge::formula_translation::ASTNode
#define ASTLITERAL tge::formula_translation::ASTLiteral
#define ASTBINARYOP tge::formula_translation::ASTBinaryOp
#define ASTCOMPAREOP tge::formula_translation::ASTCompareOp
#define ASTLOGICALOP tge::formula_translation::ASTLogicalOp
#define ASTERROR tge::formula_translation::ASTError

// Forward declarations
std::unique_ptr<ASTNODE> parse_logical(Tokenizer& tz);
std::unique_ptr<ASTNODE> parse_comparison(Tokenizer& tz);
std::unique_ptr<ASTNODE> parse_term(Tokenizer& tz);
std::unique_ptr<ASTNODE> parse_arith(Tokenizer& tz);

// expr = arith
std::unique_ptr<ASTNODE> parse_expr(Tokenizer& tz) {
    return parse_arith(tz);
}

// arith = term { (+|-) term }
std::unique_ptr<ASTNODE> parse_arith(Tokenizer& tz) {
    auto left = parse_term(tz);
    while (tz.current().type == Tokenizer::TokenType::Plus || tz.current().type == Tokenizer::TokenType::Minus) {
        ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Plus) ? ASTBinaryOp::Op::Add : ASTBinaryOp::Op::Sub;
        tz.next();
        auto right = parse_term(tz);
        auto node = std::make_unique<ASTNODE>();
        node->node = ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

// logical = comparison { (and|or) comparison }
std::unique_ptr<ASTNODE> parse_logical(Tokenizer& tz) {
    auto left = parse_comparison(tz);
    while (tz.current().type == Tokenizer::TokenType::And || tz.current().type == Tokenizer::TokenType::Or) {
        ASTLogicalOp::Op op = (tz.current().type == Tokenizer::TokenType::And) ? ASTLogicalOp::Op::And : ASTLogicalOp::Op::Or;
        tz.next();
        auto right = parse_comparison(tz);
        auto node = std::make_unique<ASTNODE>();
        node->node = ASTLogicalOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

// comparison = arith [comp_op arith]
std::unique_ptr<ASTNODE> parse_comparison(Tokenizer& tz) {
    auto left = parse_term(tz);
    using TT = Tokenizer::TokenType;
    if (tz.current().type == TT::Eq || tz.current().type == TT::Neq || tz.current().type == TT::Lt || tz.current().type == TT::Gt || tz.current().type == TT::Le || tz.current().type == TT::Ge) {
        ASTCompareOp::Op op;
        switch (tz.current().type) {
            case TT::Eq: op = ASTCompareOp::Op::Eq; break;
            case TT::Neq: op = ASTCompareOp::Op::Neq; break;
            case TT::Lt: op = ASTCompareOp::Op::Lt; break;
            case TT::Gt: op = ASTCompareOp::Op::Gt; break;
            case TT::Le: op = ASTCompareOp::Op::Le; break;
            case TT::Ge: op = ASTCompareOp::Op::Ge; break;
            default: op = ASTCompareOp::Op::Eq; break;
        }
        tz.next();
        auto right = parse_term(tz);
        auto node = std::make_unique<ASTNODE>();
        node->node = ASTCompareOp{op, std::move(left), std::move(right)};
        return node;
    }
    return left;
}

// Recursive descent parser for arithmetic expressions
// expr = term { (+|-) term }
// term = factor { (*|/) factor }
// factor = number | '(' expr ')'
std::unique_ptr<ASTNODE> parse_term(Tokenizer& tz);
std::unique_ptr<ASTNODE> parse_factor(Tokenizer& tz);

std::unique_ptr<ASTNODE> parse_term(Tokenizer& tz) {
    auto left = parse_factor(tz);
    while (tz.current().type == Tokenizer::TokenType::Mul || tz.current().type == Tokenizer::TokenType::Div) {
        ASTBinaryOp::Op op = (tz.current().type == Tokenizer::TokenType::Mul) ? ASTBinaryOp::Op::Mul : ASTBinaryOp::Op::Div;
        tz.next();
        auto right = parse_factor(tz);
        auto node = std::make_unique<ASTNODE>();
        node->node = ASTBinaryOp{op, std::move(left), std::move(right)};
        left = std::move(node);
    }
    return left;
}

std::unique_ptr<ASTNODE> parse_factor(Tokenizer& tz) {
    if (tz.current().type == Tokenizer::TokenType::Number) {
        double val = tz.current().value;
        tz.next();
        auto node = std::make_unique<ASTNODE>();
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
            auto err = std::make_unique<ASTNODE>();
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
                    auto minusOne = std::make_unique<ASTNODE>();
                    minusOne->node = ASTLiteral{ASTLiteral::Type::Int, -1, -1.0};
                    auto node = std::make_unique<ASTNODE>();
                    node->node = ASTBinaryOp{ASTBinaryOp::Op::Mul, std::move(minusOne), std::move(arg)};
                    return node;
                } else {
                    auto err = std::make_unique<ASTNODE>();
                    err->node = ASTError{{"Expected ')' after function argument"}};
                    return err;
                }
            }
        }
        auto err = std::make_unique<ASTNODE>();
        err->node = ASTError{{"Unexpected token"}};
        return err;
    } else {
        auto err = std::make_unique<ASTNODE>();
        err->node = ASTError{{"Unexpected token"}};
        return err;
    }
}

} // namespace tge::formula_translation
