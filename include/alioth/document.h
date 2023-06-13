#ifndef __ALIOTH_DOCUMNET_H__
#define __ALIOTH_DOCUMNET_H__

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace alioth {

/**
 * 源码结构
 */
struct Document;
using Doc = std::shared_ptr<Document>;

struct Document {
  std::string const content{};                        // 源码文本内容
  std::optional<std::filesystem::path> const path{};  // 源码路径

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
   */
  static Doc Read(std::filesystem::path const& path);

  /**
   * 从标准输入流读取全部内容
   */
  static Doc Read();
};

}  // namespace alioth

#endif