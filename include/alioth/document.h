#ifndef __ALIOTH_DOCUMNET_H__
#define __ALIOTH_DOCUMNET_H__

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace alioth {

/**
 * 源码结构
 */
struct Document;
using Doc = std::shared_ptr<Document>;
struct Point;
struct Range;

struct Document {
  std::string const content{};                        // 源码文本内容
  std::optional<std::filesystem::path> const path{};  // 源码路径

  /**
   * 计算文本中指定偏移量的行号和列号
   *
   * @param offset 偏移量
   */
  Point PointAt(size_t offset) const;

  /**
   * 创建源码结构
   *
   * @param content 源码文本内容
   * @param path 源码路径
   */
  static Doc Create(
      std::string const& content,
      std::optional<std::filesystem::path> const& path = std::nullopt);

  /**
   * 从文件加载源码
   *
   * @param path 源码路径
   * @param stdin 若指定此标志，且文件路径与stdin相同，则从标准输入流读取内容
   */
  static Doc Read(std::filesystem::path const& path,
                  std::optional<std::string> const& stdin = std::nullopt);

  /**
   * 从标准输入流读取全部内容
   */
  static Doc Read();
};

/**
 * 文档中的位置
 */
struct Point {
  size_t line{1};    // 行号从1开始
  size_t column{1};  // 列号从1开始

  /**
   * 将点转换为JSON格式
   */
  nlohmann::json Store() const;
};

/**
 * 文档中的范围
 */
struct Range {
  Point start{};  // 起始位置，包含
  Point end{};    // 结束位置，包含

  /**
   * 将范围转换为JSON格式
   */
  nlohmann::json Store() const;
};

}  // namespace alioth

#endif