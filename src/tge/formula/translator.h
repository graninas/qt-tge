#pragma once
#include <string>
#include <variant>
#include <optional>

#include "parser.h"

namespace tge::formula {

int evaluateAST(const std::shared_ptr<ASTNode> &node, const std::map<std::string, int> &params);

// Parse the expression in 'src' and evaluate it using 'params'.
// Throws std::runtime_error on parse or evaluation errors.
int parseAndEvaluateExpression(const std::string &src, const std::map<std::string, int> &params);

} // namespace tge::formula
