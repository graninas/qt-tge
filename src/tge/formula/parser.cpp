#include "parser.h"

#include <cctype>
#include <cstring>
#include <random>
#include <stdexcept>
#include <string>

namespace tge {
namespace formula {

static std::shared_ptr<ASTNode> makeOp(
    const std::string &op,
    const std::shared_ptr<ASTNode> &left,
    const std::shared_ptr<ASTNode> &right)
{
  auto node = std::make_shared<ASTNode>(ASTNode::Type::Operator, op);
  node->left = left;
  node->right = right;
  return node;
}

class ExpressionParser
{
public:
  explicit ExpressionParser(const std::string &src) : input(src), pos(0) {}

  std::shared_ptr<ASTNode> parse()
  {
    auto ast = parseOrExpr();
    skipSpaces();
    if (!eof())
    {
      throwParseError("Unexpected token");
    }
    return ast;
  }

private:
  const std::string &input;
  std::size_t pos;

  [[noreturn]] void throwParseError(const std::string &msg) const
  {
    throw std::runtime_error("Parse error at position " + std::to_string(pos) + ": " + msg);
  }

  bool eof() const
  {
    return pos >= input.size();
  }

  void skipSpaces()
  {
    while (!eof() && std::isspace(static_cast<unsigned char>(input[pos])))
    {
      ++pos;
    }
  }

  bool matchChar(char ch)
  {
    skipSpaces();
    if (!eof() && input[pos] == ch)
    {
      ++pos;
      return true;
    }
    return false;
  }

  void expectChar(char ch)
  {
    if (!matchChar(ch))
    {
      throwParseError(std::string("Expected '") + ch + "'");
    }
  }

  bool matchText(const char *text)
  {
    skipSpaces();
    const std::size_t n = std::strlen(text);
    if (input.compare(pos, n, text) == 0)
    {
      pos += n;
      return true;
    }
    return false;
  }

  std::shared_ptr<ASTNode> parseOrExpr()
  {
    auto left = parseAndExpr();
    while (matchText("or"))
    {
      left = makeOp("or", left, parseAndExpr());
    }
    return left;
  }

  std::shared_ptr<ASTNode> parseAndExpr()
  {
    auto left = parseComparisonExpr();
    while (matchText("and"))
    {
      left = makeOp("and", left, parseComparisonExpr());
    }
    return left;
  }

  std::shared_ptr<ASTNode> parseComparisonExpr()
  {
    auto left = parseRangeExpr();
    while (true)
    {
      if (matchText(">="))
      {
        left = makeOp(">=", left, parseRangeExpr());
      }
      else if (matchText("<="))
      {
        left = makeOp("<=", left, parseRangeExpr());
      }
      else if (matchText("=="))
      {
        left = makeOp("==", left, parseRangeExpr());
      }
      else if (matchText("!="))
      {
        left = makeOp("!=", left, parseRangeExpr());
      }
      else if (matchText(">"))
      {
        left = makeOp(">", left, parseRangeExpr());
      }
      else if (matchText("<"))
      {
        left = makeOp("<", left, parseRangeExpr());
      }
      else if (matchText("in"))
      {
        left = makeOp("in", left, parseRangeExpr());
      }
      else
      {
        break;
      }
    }
    return left;
  }

  std::shared_ptr<ASTNode> parseRangeExpr()
  {
    auto left = parseAdditiveExpr();
    while (matchText("to"))
    {
      left = makeOp("to", left, parseAdditiveExpr());
    }
    return left;
  }

  std::shared_ptr<ASTNode> parseAdditiveExpr()
  {
    auto left = parseMultiplicativeExpr();
    while (true)
    {
      if (matchChar('+'))
      {
        left = makeOp("+", left, parseMultiplicativeExpr());
      }
      else if (matchChar('-'))
      {
        left = makeOp("-", left, parseMultiplicativeExpr());
      }
      else
      {
        break;
      }
    }
    return left;
  }

  std::shared_ptr<ASTNode> parseMultiplicativeExpr()
  {
    auto left = parsePrimaryExpr();
    while (true)
    {
      if (matchChar('*'))
      {
        left = makeOp("*", left, parsePrimaryExpr());
      }
      else if (matchChar('/'))
      {
        left = makeOp("/", left, parsePrimaryExpr());
      }
      else if (matchText("div"))
      {
        left = makeOp("div", left, parsePrimaryExpr());
      }
      else if (matchText("mod"))
      {
        left = makeOp("mod", left, parsePrimaryExpr());
      }
      else
      {
        break;
      }
    }
    return left;
  }

  std::shared_ptr<ASTNode> parsePrimaryExpr()
  {
    if (matchChar('('))
    {
      auto expr = parseOrExpr();
      expectChar(')');
      return expr;
    }

    if (matchText("neg("))
    {
      auto operand = parseOrExpr();
      expectChar(')');
      return makeOp("neg", operand, nullptr);
    }

    if (matchText("rnd("))
    {
      auto from = parseOrExpr();
      expectChar(',');
      auto toExcluding = parseOrExpr();
      expectChar(')');
      return makeOp("rnd", from, toExcluding);
    }

    if (matchChar('['))
    {
      skipSpaces();
      if (eof() || !std::isalpha(static_cast<unsigned char>(input[pos])))
      {
        throwParseError("Expected parameter identifier");
      }

      std::string ident;
      ident.push_back(input[pos++]);
      while (!eof() && std::isalnum(static_cast<unsigned char>(input[pos])))
      {
        ident.push_back(input[pos++]);
      }

      expectChar(']');
      return std::make_shared<ASTNode>(ASTNode::Type::Parameter, ident);
    }

    skipSpaces();
    if (!eof() && std::isdigit(static_cast<unsigned char>(input[pos])))
    {
      std::size_t start = pos;
      while (!eof() && std::isdigit(static_cast<unsigned char>(input[pos])))
      {
        ++pos;
      }
      int value = std::stoi(input.substr(start, pos - start));
      return std::make_shared<ASTNode>(ASTNode::Type::Number, value);
    }

    throwParseError("Expected primary expression");
  }
};

// Evaluate the AST using the parameter map
int evaluateAST(const std::shared_ptr<ASTNode> &node, const std::map<std::string, int> &params)
{
  if (!node)
  {
    throw std::runtime_error("Invalid AST node");
  }

  switch (node->type)
  {
  case ASTNode::Type::Parameter:
  {
    auto it = params.find(node->value);
    if (it == params.end())
    {
      throw std::runtime_error("Undefined parameter: " + node->value);
    }
    return it->second;
  }
  case ASTNode::Type::Number:
    return node->number;
  case ASTNode::Type::Operator:
  {
    // Handle unary operators first (no right child)
    if (node->value == "neg")
    {
      int operand = evaluateAST(node->left, params);
      return -operand;
    }

    // rnd(from, toExcluding) — random integer in [from, toExcluding)
    if (node->value == "rnd")
    {
      int from = evaluateAST(node->left, params);
      int toExcluding = evaluateAST(node->right, params);
      if (toExcluding <= from)
      {
        throw std::runtime_error("rnd: toExcluding must be greater than from");
      }
      static std::mt19937 rng(std::random_device{}());
      std::uniform_int_distribution<int> dist(from, toExcluding - 1);
      return dist(rng);
    }

    // For 'in', evaluate left normally but right is a range node - handle specially
    if (node->value == "in")
    {
      if (!node->right || node->right->value != "to")
      {
        throw std::runtime_error("'in' operator requires a 'to' range on the right");
      }
      int val = evaluateAST(node->left, params);
      int rangeStart = evaluateAST(node->right->left, params);
      int rangeEnd = evaluateAST(node->right->right, params);
      return (val >= rangeStart && val <= rangeEnd) ? 1 : 0;
    }

    // Binary operators: evaluate both sides
    int leftValue = evaluateAST(node->left, params);
    int rightValue = evaluateAST(node->right, params);

    if (node->value == ">=")
    {
      return (leftValue >= rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == "<=")
    {
      return (leftValue <= rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == ">")
    {
      return (leftValue > rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == "<")
    {
      return (leftValue < rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == "==")
    {
      return (leftValue == rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == "!=")
    {
      return (leftValue != rightValue) ? 1 : 0; // Return 1 for true, 0 for false
    }
    else if (node->value == "+")
    {
      return leftValue + rightValue;
    }
    else if (node->value == "-")
    {
      return leftValue - rightValue;
    }
    else if (node->value == "*")
    {
      return leftValue * rightValue;
    }
    else if (node->value == "/")
    {
      if (rightValue == 0)
      {
        throw std::runtime_error("Division by zero");
      }
      return leftValue / rightValue;
    }
    else if (node->value == "div")
    {
      if (rightValue == 0)
      {
        throw std::runtime_error("Division by zero");
      }
      return leftValue / rightValue;
    }
    else if (node->value == "mod")
    {
      if (rightValue == 0)
      {
        throw std::runtime_error("Division by zero");
      }
      return leftValue % rightValue;
    }
    else if (node->value == "and")
    {
      return (leftValue != 0 && rightValue != 0) ? 1 : 0;
    }
    else if (node->value == "or")
    {
      return (leftValue != 0 || rightValue != 0) ? 1 : 0;
    }
    else if (node->value == "<>")
    {
      return (leftValue != rightValue) ? 1 : 0;
    }
    else if (node->value == "=")
    {
      return (leftValue == rightValue) ? 1 : 0;
    }
    else if (node->value == "to")
    {
      // 'to' is only valid as the right child of 'in'; standalone is an error
      throw std::runtime_error("'to' operator used outside of 'in' context");
    }
    else
    {
      throw std::runtime_error("Unknown operator: " + node->value);
    }
  }
  default:
    throw std::runtime_error("Unknown AST node type");
  }
}

int parseAndEvaluateExpression(const std::string &src, const std::map<std::string, int> &params)
{
  ExpressionParser parser(src);
  std::shared_ptr<ASTNode> ast = parser.parse();
  return evaluateAST(ast, params);
}

ParseResult parse(const std::string &src)
{
  try
  {
    ExpressionParser parser(src);
    std::shared_ptr<ASTNode> ast = parser.parse();
    return {ast, ""};
  }
  catch (const std::exception &e)
  {
    return {nullptr, e.what()};
  }
}

} // namespace formula
} // namespace tge
