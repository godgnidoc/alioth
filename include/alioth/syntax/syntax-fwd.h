#ifndef __ALIOTH_SYNTAX_FWD_H__
#define __ALIOTH_SYNTAX_FWD_H__

#include <utility>
#include <memory>

namespace alioth {
class Syntax;

using SyntaxRef = std::shared_ptr<Syntax>;
using SyntaxCRef = std::shared_ptr<Syntax const>;
}  // namespace alioth

#endif