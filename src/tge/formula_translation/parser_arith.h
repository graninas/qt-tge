#pragma once
#include "translator.h"
#include <string>
#include <memory>

namespace tge::formula_translation {

// Tokenizer for arithmetic expressions
class Tokenizer {
public:
    enum class TokenType {
        Number, Plus, Minus, Mul, Div,
        LParen, RParen, End, Invalid,
        Eq, Neq, Lt, Gt, Le, Ge, // Comparison
        And, Or // Logical
    };
    struct Token {
        TokenType type;
        double value = 0.0;
    };
    Tokenizer(const std::string& input);
    const Token& current() const;
    void next();
    // Accessors for parser
    const std::string& input() const { return input_; }
    size_t pos() const { return pos_; }
    void set_pos(size_t p) { pos_ = p; }
private:
    std::string input_;
    size_t pos_;
    Token current_;
};

// Parse an expression, returns unique_ptr<ASTNode>
std::unique_ptr<struct ASTNode> parse_expr(Tokenizer& tz);

} // namespace tge::formula_translation
