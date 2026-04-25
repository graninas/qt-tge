#include "translator.h"

#include <cctype>
#include <cstring>
#include <random>
#include <stdexcept>
#include <string>

namespace tge::formula {


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
  ParseResult result = parse(src);
  if (!result.error.empty())
  {
    throw std::runtime_error(result.error);
  }
  return evaluateAST(result.ast, params);
}


} // namespace tge::formula
