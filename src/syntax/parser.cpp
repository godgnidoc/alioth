#include "alioth/syntax/parser.h"

#include "alioth/lex/scanner.h"
#include "alioth/syntax/syntax.h"

namespace alioth::syntax {

AST Parser::Parse(SyntaxCRef syntax, SourceRef source) {
  Parser context{};

  context.root_ = std::make_shared<ast::Root>();
  context.root_->syntax = syntax;
  context.root_->source = source;

  return context._Parse();
}

AST Parser::_Parse() {
  auto syntax = root_->syntax;
  auto scanner_ = lex::Scanner::Create(syntax->GetLex(), root_->source);

  stack_.push_back(0);

  while (true) {
    auto const input = root_->MakeTerm(scanner_->NextToken());
    if (syntax->IsIgnored(input->id)) continue;

    auto* state = &syntax->GetState(stack_.back());

    if (!seens_.empty() &&
        std::dynamic_pointer_cast<ast::Term>(seens_.back())) {
      std::vector<std::string> seens;
      for (auto seen : seens_) seens.push_back(seen->GetName());
      logger_->trace("shift: {}; ({} . {})", seens_.back()->GetName(),
                     fmt::join(seens, " "), input->GetName());
    }

    /**
     * 尝试归约
     */
    auto reduce = state->reduce.find(input->id);
    while (reduce != state->reduce.end()) {
      /**
       * 找到归约使用的产生式
       */
      auto formula_id = reduce->second;
      auto formula = syntax->GetReduce(formula_id);

      /**
       * 生成一个非终结符节点
       */
      auto ntrm = std::make_shared<ast::Ntrm>();
      ntrm->parent = root_;
      ntrm->formula = formula_id;
      ntrm->id = formula.ntrm;
      ntrm->sentence.insert(ntrm->sentence.begin(),
                            seens_.begin() + seens_.size() - formula.length,
                            seens_.end());
      for (auto symbol : ntrm->sentence) symbol->parent = ntrm;

      /**
       * 收集属性
       */
      std::vector<std::string> sentence;
      auto ait = formula.attrs.begin();
      auto uit = formula.unfolds.begin();
      for (auto i = 0UL; i < ntrm->sentence.size(); ++i) {
        auto offset = static_cast<int>(i);
        auto symbol = ntrm->sentence.at(offset);

        /**
         * 收集展开符号的属性
         */
        auto unfold = (uit != formula.unfolds.end() && *uit == offset);
        if (unfold) {
          ++uit;
          sentence.push_back("..." + symbol->GetName());

          auto n = std::dynamic_pointer_cast<ast::Ntrm>(symbol);
          for (auto const& [name, attrs] : n->attributes) {
            auto& attributes = ntrm->attributes[name];
            attributes.insert(attributes.end(), attrs.begin(), attrs.end());
          }

          continue;
        }

        /**
         * 检查符号是否被收集为属性
         */
        if (ait == formula.attrs.end() || ait->first != offset) {
          if (!unfold) sentence.push_back(symbol->GetName());
          continue;
        }

        /**
         * 将符号收集为属性
         */
        auto name = syntax->GetAttrName(ait++->second);
        ntrm->attributes[name].push_back(symbol);

        sentence.push_back(symbol->GetName() + "@" + name);
      }

      /**
       * 依据产生式长度，从栈中弹出相应数量的状态和符号
       */
      stack_.resize(stack_.size() - formula.length);
      state = &syntax->GetState(stack_.back());
      seens_.resize(seens_.size() - formula.length);

      /**
       * 将归约结果压入栈中
       * 并执行状态转移
       */
      seens_.push_back(ntrm);
      stack_.push_back(state->shift.at(ntrm->id));
      state = &syntax->GetState(stack_.back());

      {
        std::vector<std::string> seens;
        for (auto seen : seens_) seens.push_back(seen->GetName());
        logger_->trace("reduce: {} -> {}; ({} . {})", ntrm->GetName(),
                       fmt::join(sentence, " "), fmt::join(seens, " "),
                       input->GetName());
      }

      /**
       * 重置归约条件
       */
      reduce = state->reduce.find(input->id);
    }

    /**
     * 尝试状态转移
     */
    auto shift = state->shift.find(input->id);
    if (shift != state->shift.end()) {
      if (input->id == 0) break;

      stack_.push_back(shift->second);
      seens_.push_back(input);
      continue;
    }

    /**
     * 处理错误
     */
    logger_->error("{}: syntax error: unexpected token {} {}",
                   input->GetLocation(), input->GetName(), input->GetText());

    std::set<std::string> expected;
    for (auto const& [id, _] : state->shift) {
      expected.insert(syntax->GetSymbolName(id));
    }
    for (auto const& [id, _] : state->reduce) {
      expected.insert(syntax->GetSymbolName(id));
    }

    logger_->info("expected: {}", fmt::join(expected, " | "));

    // TODO: 错误处理
    throw std::runtime_error("syntax error");
  }

  /**
   * 检查并接受解析结果
   */
  if (seens_.size() != 1) {
    throw std::runtime_error("syntax error(seens_.size() != 1)");
  }

  auto accept = std::dynamic_pointer_cast<ast::Ntrm>(seens_.front());
  if (accept->id != syntax->GetAcceptSymbol()) {
    throw std::runtime_error(
        "syntax error(accept->id != syntax->GetAcceptSymbol())");
  }

  /**
   * 接受解析结果
   */
  root_->id = accept->id;
  root_->formula = accept->formula;
  root_->sentence = accept->sentence;
  root_->attributes = accept->attributes;
  for (auto const& symbol : root_->sentence) symbol->parent = root_;

  return root_;
}

}  // namespace alioth::syntax