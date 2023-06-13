#include "alioth/ast.h"

#include "aliox/skeleton.h"

namespace alioth {

ASTNtrm ASTNode::AsNtrm() {
  return std::dynamic_pointer_cast<ASTNtrmNode>(shared_from_this());
}

ASTTerm ASTNode::AsTerm() {
  return std::dynamic_pointer_cast<ASTTermNode>(shared_from_this());
}

ASTTerm ASTNode::First() {
  if (auto term = AsTerm()) return term;

  auto ntrm = AsNtrm();
  for (auto symbol : ntrm->sentence) {
    auto term = symbol->First();
    if (term) return term;
  }
  return nullptr;
}

ASTTerm ASTNode::Last() {
  if (auto term = AsTerm()) return term;

  auto ntrm = AsNtrm();
  for (auto it = ntrm->sentence.rbegin(); it != ntrm->sentence.rend(); ++it) {
    auto symbol = *it;
    auto term = symbol->Last();
    if (term) return term;
  }
  return nullptr;
}

FormulaID ASTNode::OriginFormula() {
  auto ntrm = AsNtrm();
  if (!ntrm) return -1UL;

  auto root = ntrm->root.lock();
  auto syntax = root->syntax;
  while (syntax->formulas.at(ntrm->formula).Unfolded()) {
    if (ntrm->sentence.size() != 1) {
      /**
       * 被忽略的单词在归约时会被填回句子
       * 按理说不会影响完全展开产生式
       * 但是为了保险起见还是检查一下
       */
      throw std::runtime_error(
          fmt::format("Unexpected sentence size {} in unfolded formula {}",
                      ntrm->sentence.size(), ntrm->formula));
    }
    ntrm = ntrm->sentence.front()->AsNtrm();
  }

  return ntrm->formula;
}

AST ASTNode::Attr(std::string const& key) {
  auto ntrm = AsNtrm();
  if (!ntrm) return nullptr;

  auto it = ntrm->attributes.find(key);
  if (it == ntrm->attributes.end()) return nullptr;
  if (it->second.empty()) return nullptr;

  return it->second.front();
}

std::vector<AST> ASTNode::Attrs(std::string const& key) {
  auto ntrm = AsNtrm();
  if (!ntrm) return {};

  auto it = ntrm->attributes.find(key);
  if (it == ntrm->attributes.end()) return {};

  return it->second;
}

Range ASTNode::Range() {
  auto doc = root.lock()->doc;

  auto first = First();
  auto last = Last();

  auto start = doc->PointAt(first->offset);
  auto end = doc->PointAt(last->offset + last->length);

  return {start, end};
}

std::string ASTNode::Name() { return root.lock()->syntax->NameOf(id); }

std::string ASTNode::Text() {
  auto doc = root.lock()->doc;

  if (auto term = AsTerm())
    return doc->content.substr(term->offset, term->length);

  auto ntrm = AsNtrm();
  auto first = First();
  auto last = Last();

  return doc->content.substr(first->offset,
                             last->offset + last->length - first->offset);
}

std::optional<std::string> ASTNode::TextOf(std::string const& key) {
  auto attr = Attr(key);
  if (!attr) return std::nullopt;

  return attr->Text();
}

std::string ASTNode::Location() {
  auto doc = root.lock()->doc;

  if (auto ntrm = AsNtrm()) return First()->Location();

  auto term = AsTerm();
  auto point = doc->PointAt(term->offset);
  return fmt::format("{}:{}:{}", doc->path.value_or("<unknown-path>").string(),
                     point.line, point.column);
}

nlohmann::json ASTNode::Store(StoreOptions const& options) {
  if (auto term = AsTerm()) {
    if (!options.unfold) return term->Text();

    nlohmann::json attrs = term->attributes;
    if (options.text) attrs[*options.text] = term->Text();
    if (options.id) attrs[*options.id] = term->id;
    if (options.name) attrs[*options.name] = term->Name();
    if (options.range) attrs[*options.range] = term->Range().Store();
    if (attrs.size()) return attrs;

    return term->Text();
  }

  auto ntrm = AsNtrm();
  if (options.flatten) {
    auto sentense = nlohmann::json::array();
    for (auto const& symbol : ntrm->sentence) {
      auto item = symbol->Store(options);
      if (item.is_array()) {
        sentense.insert(sentense.end(), item.begin(), item.end());
      } else {
        sentense.push_back(item);
      }
    }
    return sentense;
  }

  if (!options.compact) {
    nlohmann::json sentence;
    for (auto const& symbol : ntrm->sentence) {
      sentence.push_back(symbol->Store(options));
    }
    return sentence;
  }

  auto root = ntrm->root.lock();
  auto syntax = root->syntax;
  auto origin_formula_id = ntrm->OriginFormula();
  auto const& origin_formula = syntax->formulas.at(origin_formula_id);
  Skeleton::Attributes const* hints{};
  if (options.skeleton) {
    auto const& structure = options.skeleton->structures.at(ntrm->id);
    if (origin_formula.form) {
      hints = &structure.forms.at(*origin_formula.form).attributes;
    } else {
      hints = &structure.attributes;
    }
  }

  nlohmann::json attrs;
  if (options.id) attrs[*options.id] = ntrm->id;
  if (options.name) attrs[*options.name] = ntrm->Name();
  if (options.range) attrs[*options.range] = ntrm->Range().Store();
  if (options.formula) attrs[*options.formula] = ntrm->formula;
  if (options.origin) attrs[*options.origin] = origin_formula_id;
  if (origin_formula.form && options.form)
    attrs[*options.form] = *origin_formula.form;

  for (auto const& [name, values] : ntrm->attributes) {
    auto is_single = (values.size() == 1);
    if (hints) {
      is_single = hints->at(name).is_single;
    }

    if (is_single) {
      attrs[name] = values.front()->Store(options);
      continue;
    }

    nlohmann::json attr;
    for (auto const& value : values) {
      attr.push_back(value->Store(options));
    }
    attrs[name] = attr;
  }

  return attrs;
}

ASTTerm ASTRootNode::Term(SymbolID id, size_t offset, size_t length) {
  auto term = std::make_shared<ASTTermNode>();
  term->root = std::dynamic_pointer_cast<ASTRootNode>(shared_from_this());
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
  ntrm->root = std::dynamic_pointer_cast<ASTRootNode>(shared_from_this());
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

  /**
   * 为终结符属性追加标注属性
   */
  for (auto& [name, attrs] : ntrm->attributes) {
    auto ait = f.attributes.find(name);
    if (ait == f.attributes.end()) continue;

    for (auto symbol : attrs) {
      auto term = symbol->AsTerm();
      if (!term) continue;

      for (auto const& [key, value] : ait->second) {
        term->attributes[key].merge_patch(value);
      }
    }
  }

  return ntrm;
}

}  // namespace alioth