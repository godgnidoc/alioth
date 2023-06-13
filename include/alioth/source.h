#ifndef __ALIOTH_SOURCE_H__
#define __ALIOTH_SOURCE_H__

#include <memory>
#include <string>

namespace alioth {

/**
 * Source
 *
 * 提供基于字符串的源码对象
 */
struct Source {
  std::string const content;
  std::string const path;

  /**
   * 创建源码对象
   *
   * @param content 源码内容
   * @param path 源码路径
   */
  static std::shared_ptr<Source> Create(std::string const& content,
                                        std::string const& path = "unknown");

  /**
   * 加载源码文件
   */
  static std::shared_ptr<Source> Load(std::string const& path);
};

using SourceRef = std::shared_ptr<Source>;

}  // namespace alioth

#endif