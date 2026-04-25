#ifndef TGE_FORMULA_PARSER_H
#define TGE_FORMULA_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace tge {
namespace formula {

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

struct ParseResult
{
  std::shared_ptr<ASTNode> ast;
  std::string error; // empty if no error
};

// Parse the expression in 'src' and evaluate it using 'params'.
// Throws std::runtime_error on parse or evaluation errors.
int parseAndEvaluateExpression(const std::string &src, const std::map<std::string, int> &params);
ParseResult parse(const std::string &src);

} // namespace formula
} // namespace tge

#endif // TGE_FORMULA_PARSER_H
