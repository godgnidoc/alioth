#include "aliox/skeleton.h"

#include <queue>
#include <vector>

namespace alioth {

nlohmann::json Skeleton::Store() const {
  nlohmann::json json;

  for (auto const& [symbol, structure] : structures) {
    nlohmann::json s;
    s["id"] = symbol;
    auto accepts = generic::collect(equivalence, [&](auto const& it) {
      if (it.second == symbol) return it.first;
      return symbol;
    });
    accepts.insert(symbol);
    s["accepts"] = accepts;
    for (auto const& [name, attr] : structure.attributes) {
      nlohmann::json a;
      a["optional"] = attr.is_optional;
      a["single"] = attr.is_single;
      for (auto const& id : attr.candidates) {
        a["candidates"].push_back(syntax->NameOf(id));
      }
      s["attributes"][name] = a;
    }
    for (auto const& [name, attr] : structure.common_attributes) {
      nlohmann::json a;
      a["optional"] = attr.is_optional;
      a["single"] = attr.is_single;
      for (auto const& id : attr.candidates) {
        a["candidates"].push_back(syntax->NameOf(id));
      }
      s["common_attributes"][name] = a;
    }
    for (auto const& [form, formed] : structure.forms) {
      nlohmann::json f;
      f["formulas"] = formed.formulas;
      for (auto const& [name, attr] : formed.attributes) {
        nlohmann::json a;
        a["optional"] = attr.is_optional;
        a["single"] = attr.is_single;
        for (auto const& id : attr.candidates) {
          a["candidates"].push_back(syntax->NameOf(id));
        }
        f["attributes"][name] = a;
      }
      s["forms"][form] = f;
    }
    auto symbol_name = syntax->NameOf(symbol);
    json[symbol_name] = s;
  }

  return json;
}

Skeleton::Structure const& Skeleton::StructOf(SymbolID symbol) const {
  auto eit = equivalence.find(symbol);
  if (eit != equivalence.end()) {
    symbol = eit->second;
  }
  return structures.at(symbol);
}

Skeleton Skeleton::Deduce(Syntax const& syntax) {
  Skeleton lang{syntax};

  DeduceStructures(lang);
  DeduceForms(lang);
  PropagateForms(lang);
  DeduceCommon(lang);
  ReplaceEquivalentCandidates(lang);
  StripIntermediate(lang);

  return lang;
}

void Skeleton::DeduceStructures(Skeleton& lang) {
  auto syntax = lang.syntax;

  /**
   * 全量分析阶段不能判断属性是否可选
   *
   * 反复填写属性特征直到没有变化
   */
  bool growing{};
  do {
    growing = false;
    for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
      auto const& f = syntax->formulas[formula];
      auto& structure = lang.structures[f.head];

      /**
       *  在一个产生式中已经出现过的属性名
       */
      std::set<std::string> seen{};
      for (auto const& symbol : f.body) {
        if (!symbol.attr) continue;
        auto attr_name = *symbol.attr;

        /**
         * 跟踪展开语法结构的属性结果
         */
        if (attr_name == "...") {
          auto unfold = lang.structures[symbol.id].attributes;
          for (auto const& [name, income] : unfold) {
            auto& attr = structure.attributes[name];

            if (seen.count(name)) {
              if (attr.is_single) growing = true;
              attr.is_single = false;
            } else {
              seen.insert(name);
            }

            if (!income.is_single) {
              if (attr.is_single) growing = true;
              attr.is_single = false;
            }

            for (auto const& id : income.candidates) {
              if (attr.candidates.count(id)) continue;
              attr.candidates.insert(id);
              growing = true;
            }
          }
          continue;
        }

        /**
         * 在同一个产生式中出现多次的属性名必然可重复
         */
        auto& attr = structure.attributes[attr_name];
        if (seen.count(attr_name)) {
          if (attr.is_single) growing = true;
          attr.is_single = false;
        } else {
          seen.insert(attr_name);
        }

        /**
         * 首次出现的属性名必然导致候选语法结构发生变化
         */
        if (!attr.candidates.count(symbol.id)) {
          attr.candidates.insert(symbol.id);
          growing = true;
        }
      }
    }
  } while (growing);
}

void Skeleton::DeduceForms(Skeleton& lang) {
  auto syntax = lang.syntax;

  /**
   * 基于全量属性结构分析每个产生式的属性结构
   * 此阶段仍然不能判断属性是否可选
   */
  std::vector<Attributes> formula_attributes(syntax->formulas.size());
  for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
    auto& attrs = formula_attributes.at(formula);
    auto const& f = syntax->formulas.at(formula);

    std::set<std::string> seen{};
    for (auto const& symbol : f.body) {
      if (!symbol.attr) continue;

      auto attr_name = *symbol.attr;
      if (attr_name == "...") {
        auto unfold = lang.structures.at(symbol.id).attributes;
        for (auto const& [name, income] : unfold) {
          auto& attr = attrs[name];

          if (seen.count(name) || !income.is_single) {
            attr.is_single = false;
          }
          seen.insert(name);

          attr.candidates.insert(income.candidates.begin(),
                                 income.candidates.end());
        }
        continue;
      }

      auto& attr = attrs[attr_name];
      if (seen.count(attr_name)) attr.is_single = false;
      seen.insert(attr_name);
      attr.candidates.insert(symbol.id);
    }
  }

  /**
   * 将产生式属性结构按照句型分组合并，填写句型属性结构
   * 在此阶段可以判断属性是否可选
   */
  for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
    auto const& f = syntax->formulas.at(formula);
    auto& structure = lang.structures[f.head];

    if (!f.form) continue;
    auto formula_atts = formula_attributes.at(formula);

    if (!structure.forms.count(*f.form)) {
      structure.forms.emplace(*f.form, Form{
                                           .attributes = formula_atts,
                                           .formulas = {formula},
                                       });
      continue;
    }

    auto& formed = structure.forms.at(*f.form);
    formed.formulas.insert(formula);
    std::set<std::string> seen{};
    for (auto const& [name, income] : formula_atts) {
      seen.insert(name);
      auto optional = !formed.attributes.count(name);

      auto& attr = formed.attributes[name];
      attr.is_single = attr.is_single && income.is_single;
      attr.is_optional = attr.is_optional || optional;
      attr.candidates.insert(income.candidates.begin(),
                             income.candidates.end());
    }
    for (auto& [name, attr] : formed.attributes) {
      if (seen.count(name)) continue;
      attr.is_optional = true;
    }
  }
}

void Skeleton::PropagateForms(Skeleton& lang) {
  auto syntax = lang.syntax;

  std::map<SymbolID, std::set<FormulaID>> unfolding;
  for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
    auto& f = syntax->formulas.at(formula);
    if (f.form) continue;
    if (f.body.size() != 1) continue;
    if (f.body.front().attr.value_or("") != "...") continue;

    unfolding[f.head].insert(formula);
  }

  /**
   * 传递链会形成树状结构，从叶子节点向上传递
   */
  std::queue<FormulaID> tasks{};
  while (!unfolding.empty()) {
    for (auto it : unfolding) {
      bool tail = true;
      for (auto formula : it.second) {
        auto& f = syntax->formulas.at(formula);
        if (!unfolding.count(f.body.front().id)) continue;
        tail = false;
        break;
      }
      if (!tail) continue;
      for (auto formula : it.second) {
        tasks.push(formula);
      }
      unfolding.erase(it.first);
      break;
    }
  }

  /**
   * 逐层传递句型
   */
  while (!tasks.empty()) {
    auto formula = tasks.front();
    tasks.pop();
    auto& f = syntax->formulas.at(formula);
    auto& to = lang.structures[f.head];
    auto& from = lang.structures[f.body.front().id];

    for (auto const& [form, formed] : from.forms) {
      if (!to.forms.count(form)) {
        to.forms.emplace(form, formed);
        continue;
      }

      auto& to_formed = to.forms.at(form);
      /**
       * 传播候选产生式
       */
      to_formed.formulas.insert(formed.formulas.begin(), formed.formulas.end());

      /**
       * 传播属性结构
       */
      for (auto const& [name, attr] : formed.attributes) {
        if (!to_formed.attributes.count(name)) {
          to_formed.attributes[name] = attr;
          continue;
        }

        /**
         * 合并已存在的属性结构
         */
        auto& to_attr = to_formed.attributes.at(name);
        to_attr.is_single = to_attr.is_single && attr.is_single;
        to_attr.is_optional = to_attr.is_optional || attr.is_optional;
        to_attr.candidates.insert(attr.candidates.begin(),
                                  attr.candidates.end());
      }
    }
  }
}

void Skeleton::ReplaceEquivalentCandidates(Skeleton& lang) {
  auto syntax = lang.syntax;

  std::map<SymbolID, SymbolID> mappings;  // lower -> higher
  for (auto formula = 0UL; formula < syntax->formulas.size(); ++formula) {
    auto& f = syntax->formulas.at(formula);
    if (f.form) continue;
    if (f.body.size() != 1) continue;
    if (f.body.front().attr.value_or("") != "...") continue;

    /**
     * 文法中大概不会出现多种不同的完全展开式，暂不考虑如下可能
     *
     * ntrmA -> ...lower;
     * ntrmB -> ...lower;
     */
    mappings[f.body.front().id] = f.head;
  }

  // 登记符号等价性
  for (auto const& [from, to] : mappings) {
    auto it = lang.equivalence.find(from);
    if (it != lang.equivalence.end()) continue;

    std::set<SymbolID> seen{};
    seen.insert(from);
    auto phinal = to;
    while (mappings.count(phinal) && !(phinal)) {
      seen.insert(phinal);
      auto eit = lang.equivalence.find(phinal);
      if (eit != lang.equivalence.end()) {
        phinal = eit->second;
      } else {
        phinal = mappings.at(phinal);
      }
    }
    for (auto const& id : seen) {
      lang.equivalence[id] = phinal;
    }
  }

  auto upgrade = [&](Attribute& attr) {
    std::vector<SymbolID> candidates{};
    candidates.insert(candidates.end(), attr.candidates.begin(),
                      attr.candidates.end());
    attr.candidates.clear();

    for (auto candidate : candidates) {
      auto eit = lang.equivalence.find(candidate);
      if (eit != lang.equivalence.end()) {
        candidate = eit->second;
      }
      attr.candidates.insert(candidate);
    }
  };

  for (auto& it : lang.structures) {
    for (auto& ait : it.second.attributes) upgrade(ait.second);
    for (auto& fit : it.second.forms)
      for (auto& ait : fit.second.attributes) upgrade(ait.second);
    for (auto& ait : it.second.common_attributes) upgrade(ait.second);
  }
}

void Skeleton::StripIntermediate(Skeleton& lang) {
  std::set<SymbolID> intermediate{};
  for (auto const& [id, _] : lang.structures) {
    intermediate.insert(id);
  }

  std::vector<SymbolID> tasks{};
  auto root = lang.syntax->lex->terms.size();
  tasks.push_back(root);

  while (!tasks.empty()) {
    auto id = tasks.back();
    tasks.pop_back();

    intermediate.erase(id);

    auto& structure = lang.structures.at(id);
    for (auto const& [name, attr] : structure.attributes) {
      for (auto const& candidate : attr.candidates) {
        if (!intermediate.count(candidate)) continue;

        tasks.push_back(candidate);
      }
    }
  }

  for (auto drop : intermediate) {
    lang.structures.erase(drop);
  }
}

void Skeleton::DeduceCommon(Skeleton& lang) {
  for (auto& structure : lang.structures) {
    auto& common = structure.second.common_attributes;
    common = structure.second.forms.begin()->second.attributes;
    for (auto& [form, formed] : structure.second.forms) {
      std::set<std::string> drop{};
      for (auto it : common) drop.insert(it.first);
      for (auto& [name, attr] : formed.attributes) {
        if (!common.count(name)) continue;
        drop.erase(name);

        auto& cttr = common.at(name);
        cttr.is_single = cttr.is_single && attr.is_single;
        cttr.is_optional = cttr.is_optional || attr.is_optional;
        cttr.candidates.insert(attr.candidates.begin(), attr.candidates.end());
      }
      for (auto const& name : drop) common.erase(name);
    }
  }
}

}  // namespace alioth