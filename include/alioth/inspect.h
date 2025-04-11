#ifndef __ALIOTH_INSPECT_H__
#define __ALIOTH_INSPECT_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "alioth/ast.h"
#include "alioth/document.h"
#include "alioth/lexicon.h"
#include "alioth/skeleton.h"
#include "alioth/syntax.h"
#include "nlohmann/json.hpp"

namespace alioth {

/**
 * 文档中的位置
 */
struct Point {
  size_t line{1};    // 行号从1开始
  size_t column{1};  // 列号从1开始
};

/**
 * 文档中的范围
 */
struct Range {
  Point start{};  // 起始位置，包含
  Point end{};    // 结束位置，包含
};

/**
 * 将点转换为JSON格式
 * 
 * @param point 点
 */
nlohmann::json StorePoint(Point const& point);

/**
 * 将范围转换为JSON格式
 *
 * @param range 范围
 */
nlohmann::json StoreRange(Range const& range);

/**
 * 计算文本中指定偏移量的行号和列号
 *
 * @param offset 偏移量
 * @param doc 文档
 */
Point PointAt(size_t offset, Doc doc);

/**
 * 尝试将语法树节点转换为终结符，失败则返回nullptr
 *
 * @param node 语法树节点
 */
ASTNtrm AsNtrm(AST node);

/**
 * 尝试将语法树节点转换为非终结符，失败则返回nullptr
 *
 * @param node 语法树节点
 */
ASTTerm AsTerm(AST node);

/**
 * 获取语法树节点的某个属性的第一个值，若节点不是非终结符或属性不存在则返回空
 *
 * @param node 语法树节点
 * @param key 属性名称
 */
AST AttrOf(AST node, std::string const& key);

/**
 * 获取语法树节点的某个属性所有的值，若节点不是非终结符或属性不存在则返回空
 *
 * @param node 语法树节点
 * @param key 属性名称
 */
std::vector<AST> AttrsOf(AST node, std::string const& key);

/**
 * 计算符号在文本中的范围
 *
 * @param node 语法树节点
 */
Range RangeOf(AST node);

/**
 * 获取符号的名称
 *
 * @param node 语法树节点
 */
std::string NameOf(AST node);

/**
 * 获取符号的文本
 *
 * @param node 语法树节点
 */
std::string TextOf(AST node);

/**
 * 获取语法树节点的指定属性的文本内容
 *
 * 若节点不是非终结符或属性不存在则返回空
 *
 * @param node 语法树节点
 * @param key 属性名称
 */
std::optional<std::string> TextOf(AST node, std::string const& key);

/**
 * 获取符号的位置
 *
 * @param node 语法树节点
 */
std::string LocationOf(AST node);

/**
 * 获取符号的第一个终结符
 *
 * @param node 语法树节点
 */
ASTTerm FirstOf(AST node);

/**
 * 获取符号的最后一个终结符
 *
 * @param node 语法树节点
 */
ASTTerm LastOf(AST node);

/**
 * 获取语法树节点的所有终结符
 */
std::vector<ASTTerm> Tokenize(AST node);

/**
 * 获取符号的属性树
 *
 * @param node 语法树节点
 */
nlohmann::json AttrsOf(AST node);

/**
 * 获取词法规则或语法规则的语言名称
 * 
 * 约定使用词法规则的第一个上下文名称作为语言名称
 * 
 * @param g 词法或语法规则
 */
std::string NameOf(std::variant<Lex, Syntax> g);

/**
 * 打印符号
 *
 * @param symbol 符号ID
 * @param alphabeta 词法规则或语法规则
 */
std::string NameOf(SymbolID symbol, std::variant<Lex, Syntax> alphabeta);

/**
 * 获取符号的ID
 *
 * @param symbol 符号名
 * @param alphabeta 词法规则或语法规则
 */
SymbolID SymbolIdOf(std::string const& name,
                    std::variant<Lex, Syntax> alphabeta);

/**
 * 打印产生式
 *
 * @param syntax 语法规则
 * @param formula 产生式ID
 * @param point 产生式中的点
 */
std::string PrintFormula(FormulaID formula, Syntax syntax,
                         std::optional<size_t> point = std::nullopt);

/**
 * 打印语法规则的一个状态
 *
 * @param syntax 语法规则
 * @param state 状态ID
 */
std::string PrintState(StateID state, Syntax syntax);

/**
 * 获取语法规则的BNF格式描述
 *
 * 可以使用 https://jsmachines.sourceforge.net/machines/lalr1.html 查看
 *
 * @param syntax 语法规则
 */
std::string PrintSyntax(Syntax syntax);

/**
 * 打印语言骨架
 *
 * @param lang 语言骨架
 */
nlohmann::json StoreSkeleton(Skeleton const& lang);

/**
 * 将语法规则保存为JSON格式
 *
 * @param syntax 语法规则
 */
nlohmann::json StoreSyntax(Syntax syntax);

/**
 * 将词法规则保存为JSON格式
 *
 * @param lex 词法规则
 */
nlohmann::json StoreLex(Lex lex);

/**
 * 从JSON格式加载语法规则
 *
 * @param json JSON格式的语法规则
 */
Syntax LoadSyntax(nlohmann::json const& json);

/**
 * 从JSON格式加载词法规则
 *
 * @param json JSON格式的词法规则
 */
Lex LoadLex(nlohmann::json const& json);

}  // namespace alioth

#endif