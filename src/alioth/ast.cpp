#include "alioth/ast.h"

namespace alioth {

ASTTerm ASTRootNode::Term(SymbolID id, size_t offset, size_t length) {
  auto term = std::make_shared<ASTTermNode>();
  term->root = weak_from_this();
  term->id = id;
  term->offset = offset;
  term->length = length;
  return term;
}

ASTNtrm ASTRootNode::Ntrm(FormulaID formula,
                          std::vector<AST>::const_iterator begin,
                          std::vector<AST>::const_iterator end) {
  auto ntrm = std::make_shared<ASTNtrmNode>();
  auto f = syntax->formulas.at(formula);
  ntrm->root = weak_from_this();
  ntrm->id = f.head;
  ntrm->formula = formula;
  ntrm->sentence.insert(ntrm->sentence.end(), begin, end);

  for (auto i = 0UL; i < ntrm->sentence.size(); ++i) {
    /**
     * 收集展开符号的属性
     */
    if (f.body.at(i).Unfolded()) {
      auto n = std::dynamic_pointer_cast<ASTNtrmNode>(ntrm->sentence.at(i));
      for (auto const& [name, attrs] : n->attributes) {
        auto& attributes = ntrm->attributes[name];
        attributes.insert(attributes.end(), attrs.begin(), attrs.end());
      }

      continue;
    }

    /**
     * 将符号收集为属性
     */
    if (f.body.at(i).attr) {
      ntrm->attributes[*f.body.at(i).attr].push_back(ntrm->sentence.at(i));
      continue;
    }
  }

  return ntrm;
}

}  // namespace alioth