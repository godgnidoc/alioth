#ifndef __ALIOTH_LEX_BUILDER_H__
#define __ALIOTH_LEX_BUILDER_H__

#include <map>
#include <memory>
#include <vector>

#include "alioth/lex/lex-fwd.h"
#include "alioth/regex.h"

namespace alioth::lex {

/**
 * Lex Builder
 *
 * 提供构造Lex的接口
 */
class Builder {
 public:
  Builder();

  Builder(Builder const&) = delete;
  Builder(Builder&&) = delete;

  Builder& operator=(Builder const&) = delete;
  Builder& operator=(Builder&&) = delete;

  /**
   * 添加一个词法单元
   * 先添加的词法单元优先级更高
   *
   * @param name 词法单元名称
   * @param pattern 正则表达式
   * @param context 词法单元上下文，空表示任何上下文
   */
  Builder& AddToken(std::string const& name, Regex const& pattern,
                    std::set<std::string> context = {});

  /**
   * 构建Lex
   */
  std::shared_ptr<Lex> Build();

 private:
  /**
   * 获取或添加上下文
   * 若上下文已存在，则返回其ID
   * 若上下文不存在，则添加上下文并返回其ID
   *
   * @param name 上下文名称
   */
  int GetOrAddContext(std::string const& name);

  /**
   * 获取或添加上下文
   * 若上下文已存在，则返回其ID
   * 若上下文不存在，则添加上下文并返回其ID
   *
   * @param names 上下文名称集合
   */
  std::set<int> GetOrAddContexts(std::set<std::string> const& names);

  /**
   * 添加词法单元
   * 若词法单元已存在，则抛出异常
   *
   * @param name 词法单元名称
   */
  int AddToken(std::string const& name);

 private:
  /**
   * 上下文表，（下标）为上下文ID
   */
  std::vector<std::string> contexts_;

  /**
   * 词法单元表，（下标+1）为词法单元ID
   */
  std::vector<std::string> tokens_;

  /**
   * 全文正则表达式
   */
  Regex regex_;

  /**
   * firstpos与上下文的映射
   */
  std::map<std::shared_ptr<regex::LeafNode>, std::set<int>> firstpos_ctx_map_;
};

}  // namespace alioth::lex

#endif