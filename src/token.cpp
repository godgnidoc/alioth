#include "alioth/token.h"

#include "alioth/lex/lex.h"

namespace alioth {

std::string Token::GetText() const {
  return source->content.substr(offset, length);
}

std::string Token::GetName() const { return lex->GetTokenName(id); }

std::string Token::GetLocation() const {
  return source->path + ":" + std::to_string(start_line) + ":" +
         std::to_string(start_column);
}
}  // namespace alioth