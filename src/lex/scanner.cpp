#include "alioth/lex/scanner.h"

#include "alioth/lex/lex.h"

namespace alioth::lex {

Scanner::Scanner(LexCRef lex, SourceRef source)
    : context_(0),
      line_(1UL),
      column_(1UL),
      offset_(0UL),
      source_(source),
      lex_(lex) {}

ScannerRef Scanner::Create(LexCRef lex, SourceRef source) {
  return std::shared_ptr<Scanner>(new Scanner(lex, source));
}

std::shared_ptr<Scanner> Scanner::Clone() const {
  auto clone = std::shared_ptr<Scanner>(new Scanner(lex_, source_));
  clone->context_ = context_;
  clone->line_ = line_;
  clone->column_ = column_;
  clone->offset_ = offset_;
  return clone;
}

Token Scanner::NextToken() {
  Token token{
      .id = Token::kEOF,
      .start_line = line_,
      .start_column = column_,
      .end_line = line_,
      .end_column = column_,
      .offset = offset_,
      .length = 0,
      .source = source_,
      .lex = lex_,
  };

  if (offset_ >= source_->content.size()) return token;

  auto state = lex_->GetState(lex_->GetState(0)->transitions.at(context_));
  while (source_->content[offset_] != '\0') {
    auto const ch = static_cast<unsigned char>(source_->content[offset_]);
    auto tr = state->transitions.find(ch);
    if (tr == state->transitions.end()) break;

    state = lex_->GetState(tr->second);
    token.length++;
    token.end_line = line_;
    token.end_column = column_;

    offset_++;
    column_++;
    if (ch == '\n') {
      line_++;
      column_ = 1UL;
    }
  }

  if (state->accepts > 0) {
    token.id = state->accepts;
  }

  return token;
}

void Scanner::SetContext(int context) { context_ = context; }
void Scanner::SetContext(std::string const& context) {
  context_ = lex_->GetContextId(context);
}
}  // namespace alioth::lex