#include "alioth/skeleton.h"

#include <queue>
#include <vector>

#include "alioth/inspect.h"

namespace alioth {

Skeleton Skeleton::Deduce(Syntax const& syntax) {
  Skeleton lang{syntax};

  DeduceStructures(lang);
  DeduceForms(lang);
  PropagateForms(lang);
  StripIntermediate(lang);
  DeduceCommon(lang);

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

    if (!structure.formed_attributes.count(*f.form)) {
      structure.formed_attributes.emplace(*f.form, formula_atts);
      continue;
    }

    auto& formed = structure.formed_attributes.at(*f.form);
    std::set<std::string> seen{};
    for (auto const& [name, income] : formula_atts) {
      seen.insert(name);
      auto optional = !formed.count(name);

      auto& attr = formed[name];
      attr.is_single = attr.is_single && income.is_single;
      attr.is_optional = attr.is_optional || optional;
      attr.candidates.insert(income.candidates.begin(),
                             income.candidates.end());
    }
    for (auto& [name, attr] : formed) {
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

    for (auto const& [form, formed] : from.formed_attributes) {
      if (!to.formed_attributes.count(form)) {
        to.formed_attributes.emplace(form, formed);
        continue;
      }

      auto& to_formed = to.formed_attributes.at(form);
      for (auto const& [name, attr] : formed) {
        if (!to_formed.count(name)) {
          to_formed[name] = attr;
          continue;
        }

        /**
         * 合并已存在的属性结构
         */
        auto& to_attr = to_formed.at(name);
        to_attr.is_single = to_attr.is_single && attr.is_single;
        to_attr.is_optional = to_attr.is_optional || attr.is_optional;
        to_attr.candidates.insert(attr.candidates.begin(),
                                  attr.candidates.end());
      }
    }
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
    common = structure.second.formed_attributes.begin()->second;
    for (auto& [form, formed] : structure.second.formed_attributes) {
      std::set<std::string> drop{};
      for (auto it : common) drop.insert(it.first);
      for (auto& [name, attr] : formed) {
        if (!common.count(name)) continue;

        auto cttr = common.at(name);
        auto same = cttr.is_single == attr.is_single;
        same = same && cttr.is_optional == attr.is_optional;
        same = same && cttr.candidates == attr.candidates;
        if (!same) {
          common.erase(name);
          continue;
        }
        drop.erase(name);
      }
      for (auto const& name : drop) common.erase(name);
    }
  }
}

}  // namespace alioth