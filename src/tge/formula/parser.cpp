#include "parser.h"

#include <stdexcept>
#include <cctype>
#include <string>
#include <sstream>
#include <variant>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <tuple>
#include <functional>
#include <algorithm>
#include <random>

#include "ps/free_parsec.h"

namespace tge {
namespace formula {

// Define the Abstract Syntax Tree (AST) for the custom language
struct ASTNode
{
  enum class Type
  {
    Parameter,
    Operator,
    Number
  };

  Type type;
  std::string value; // For parameters and operators
  int number;        // For numeric values

  std::shared_ptr<ASTNode> left;  // Left child (for binary operators)
  std::shared_ptr<ASTNode> right; // Right child (for binary operators)

  ASTNode(Type t, const std::string &val) : type(t), value(val), number(0) {}
  ASTNode(Type t, int num) : type(t), number(num) {}
};

ps::Parser<std::string> parameterIdentifier()
{
  using namespace ps;
  Parser<char> firstChar = alt(upper, lower);
  Parser<std::list<char>> restChars = many(alphanum);
  auto seqp = sequence(firstChar, restChars);
  return merge(seqp);
}

// Parser for the custom language

ps::Parser<std::shared_ptr<ASTNode>> expressionParser();

ps::Parser<std::shared_ptr<ASTNode>> parensParser()
{
  using namespace ps;

  Parser<std::shared_ptr<ASTNode>> parser =
      between(
          parseChar('('),
          expressionParser(),
          parseChar(')')) +
      "parens";

  return parser;
}

ps::Parser<std::shared_ptr<ASTNode>> parameterParser()
{
  using namespace ps;
  return fmap<std::string, std::shared_ptr<ASTNode>>(
      [](const std::string &param)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Parameter, param);
      },
      between(parseChar('['), parameterIdentifier(), parseChar(']')))
      + "param";
}

ps::Parser<std::shared_ptr<ASTNode>> numberParser()
{
  using namespace ps;
  return fmap<int, std::shared_ptr<ASTNode>>(
      [](int num)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Number, num);
      },
      mergeTo<int>(many1(digit)))
      + "number";
}

ps::Parser<std::shared_ptr<ASTNode>> negParser()
{
  using namespace ps;
  // Parses neg(expr) and returns a unary negation node
  return fmap<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>(
      [](const std::shared_ptr<ASTNode> &operand)
      {
        auto node = std::make_shared<ASTNode>(ASTNode::Type::Operator, "neg");
        node->left = operand;
        return node;
      },
      between(parseLit("neg("), expressionParser(), parseChar(')')))
      + "neg";
}

ps::Parser<std::shared_ptr<ASTNode>> rndParser()
{
  using namespace ps;
  // Parses rnd(from, toExcluding)
  auto seqp = sequence(
      skip(parseLit("rnd(")),
      expressionParser(),
      skip(parseChar(',')),
      expressionParser(),
      skip(parseChar(')')));

  return fmap<
      std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>,
      std::shared_ptr<ASTNode>>(
      [](const std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>> &t)
      {
        auto node = std::make_shared<ASTNode>(ASTNode::Type::Operator, "rnd");
        node->left = std::get<0>(t);
        node->right = std::get<1>(t);
        return node;
      },
      seqp) + "rnd";
}

ps::Parser<std::shared_ptr<ASTNode>> termParser()
{
  using namespace ps;
  return alt(try_(negParser()),
             alt(try_(rndParser()),
                 alt(try_(numberParser()),
                     alt(try_(parameterParser()), parensParser()))));
}

ps::Parser<std::shared_ptr<ASTNode>> operatorParser()
{
  using namespace ps;
  return fmap<std::string, std::shared_ptr<ASTNode>>(
      [](const std::string &op)
      {
        return std::make_shared<ASTNode>(ASTNode::Type::Operator, op);
      },
      // longer operators first to avoid prefix conflicts
      choice(parseLit(">=") + "ge",
             parseLit("<=") + "le",
             parseLit("==") + "eq",
             parseLit("!=") + "ne",
            //  parseLit("<>") + "ne2",
             parseLit(">") + "gt",
             parseLit("<") + "lt",
            //  parseLit("=") + "eq2",
             parseLit("+") + "add",
             parseLit("-") + "sub",
             parseLit("*") + "mul",
             parseLit("/") + "divop",
             parseLit("div") + "div",
             parseLit("mod") + "mod",
             parseLit("and") + "and",
             parseLit("or") + "or",
             parseLit("to") + "to",
             parseLit("in") + "in"))
      + "op";
}

ps::Parser<std::shared_ptr<ASTNode>> operatorParensExprParser()
{
  using namespace ps;

  auto seqp = sequence(
      skip(parseChar('(')),
      expressionParser(),
      operatorParser(),
      expressionParser(),
      skip(parseChar(')')))
      + "operator expr";

  Parser<std::shared_ptr<ASTNode>> mappedSeqp = fmap<
      std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>,
      std::shared_ptr<ASTNode>>(
      [](const std::tuple<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>> &t)
      {
        auto left = std::get<0>(t);
        auto op = std::get<1>(t);
        auto right = std::get<2>(t);

        // Set the left and right children of the operator node
        op->left = left;
        op->right = right;

        return op;
      },
      seqp);
  return mappedSeqp;
}



ps::Parser<std::shared_ptr<ASTNode>> expressionParser()
{
  using namespace ps;

  // Define the parser lazily to handle recursion
  return lazy<std::shared_ptr<ASTNode>>(
      []()
      {
        return alt(
            try_(operatorParensExprParser()),
            termParser());
      });
}

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
  using namespace ps;

  std::string noSpaces;
  for (char c : src) {
    if (!std::isspace(static_cast<unsigned char>(c))) {
      noSpaces += c;
    }
  }
  const std::string input = "(" + noSpaces + ")"; // Wrap the input in parentheses to ensure it matches the operator expression format


  ParserRuntime runtime(input, State{});
  Parser<std::shared_ptr<ASTNode>> parser = expressionParser();

  ParserResult<std::shared_ptr<ASTNode>> result = parseWithRuntime(runtime, parser);

  if (!isRight(result)) {
    // Get error info if available
    throw std::runtime_error("Parse error");
  }

  std::shared_ptr<ASTNode> ast = getParseSucceeded(result).parsed;
  return evaluateAST(ast, params);
}

} // namespace formula
} // namespace tge
