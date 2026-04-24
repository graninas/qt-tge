#ifndef TGE_FORMULA_PARSER_H
#define TGE_FORMULA_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace tge {
namespace formula {

// Parse the expression in 'src' and evaluate it using 'params'.
// Throws std::runtime_error on parse or evaluation errors.
int parseAndEvaluateExpression(const std::string &src, const std::map<std::string, int> &params);


} // namespace formula
} // namespace tge

#endif // TGE_FORMULA_PARSER_H
