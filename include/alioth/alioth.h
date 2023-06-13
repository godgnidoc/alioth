#ifndef __ALIOTH_H__
#define __ALIOTH_H__

#include <filesystem>

namespace alioth {

/**
 * 获取用户的 Alioth 主目录
 */
std::filesystem::path AliothHome();

}  // namespace alioth

#endif