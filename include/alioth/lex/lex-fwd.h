#ifndef __ALIOTH_LEX_FWD_H__
#define __ALIOTH_LEX_FWD_H__

#include <memory>

namespace alioth {

class Lex;
using LexRef = std::shared_ptr<Lex>;
using LexCRef = std::shared_ptr<Lex const>;

}  // namespace alioth

#endif