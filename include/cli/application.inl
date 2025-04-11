#ifndef __CLI_APPLICATION_INL__
#define __CLI_APPLICATION_INL__

#include <algorithm>

#include "cli/application.h"

namespace cli {

inline int Application::Run(int argc, char** argv) {
  return Run({argv, argv + argc});
}

inline int Application::Run(std::vector<std::string> args) {
  /**
   * 第一步，拆分短选项
   */
  for (auto i = 0UL; i < args.size(); ++i) {
    auto const& arg = args[i];
    if (arg == kEOO) break;
    if (arg.front() != '-') continue;
    if (arg.size() <= 2) continue;
    if (arg[1] == '-') continue;

    auto flags = arg.substr(1);
    args.erase(args.begin() + i);
    for (auto c : flags) {
      args.insert(args.begin() + i, "-" + std::string(1, c));
    }
  }

  std::vector<bool> consumed(args.size(), false);
  consumed.front() = true;  // argv[0] 是应用名称

  auto target = &Inspect::Commands();

  /**
   * 第二步，消费全部关键字找到目标命令
   */
  for (auto i = 0UL; i < args.size(); ++i) {
    auto const& arg = args[i];
    if (arg == kEOO) break;
    if (arg.front() == '-') continue;

    auto it = target->subcommands.find(arg);
    if (it != target->subcommands.end()) {
      target = &it->second;
      consumed[i] = true;
    }
  }

  if (!target->command) {
    throw CommandNotFound();
  }

  auto options = Inspect::Options();
  auto cmd_options = Inspect::OptionsOf(target->command);
  options.insert(options.end(), cmd_options.begin(), cmd_options.end());

  /**
   * 第三步，消费选项
   */
  for (auto i = 0UL; i < args.size(); ++i) {
    auto const& arg = args[i];
    if (arg == kEOO) break;
    if (arg.front() != '-' || arg.size() < 2) continue;
    if (consumed[i]) continue;

    for (auto opt : options) {
      auto flags = Inspect::FlagsOf(opt);
      auto it = std::find(flags.begin(), flags.end(), arg);
      if (it == flags.end()) continue;

      consumed[i] = true;
      if (!Inspect::IsRepeatable(opt) && opt->HasValue()) {
        throw DuplicatedOption(arg);
      }

      if (Inspect::ArgumentOf(opt)) {
        auto j = i + 1;
        if (j >= args.size() || consumed[j]) {
          throw OptionRequiresArgument(arg, Inspect::NameOf(target->command));
        }
        consumed[j] = true;
        opt->values_.push_back(args[j]);
      } else {
        opt->values_.push_back(arg);
      }
      break;
    }

    if (!consumed[i]) {
      throw OptionNotFound(arg, Inspect::NameOf(target->command));
    }
  }

  /**
   * 检查缺失的必须选项
   */
  for (auto opt : options) {
    if (Inspect::IsRequired(opt) && !opt->HasValue()) {
      throw RequiredOptionMissing(Inspect::FlagsOf(opt).front());
    }
  }

  /**
   * 第四步消费参数
   */
  auto crgs = Inspect::ArgumentsOf(target->command);
  auto cit = crgs.begin();

  for (auto i = 0UL; i < args.size(); ++i) {
    auto const& arg = args[i];
    if (arg == kEOO) continue;
    if (consumed[i]) continue;

    if (cit == crgs.end()) {
      throw TooManyArguments(Inspect::NameOf(target->command));
    }

    (*cit)->values_.push_back(arg);
    consumed[i] = true;
    if (!Inspect::IsSome(*cit) && !Inspect::IsMore(*cit)) {
      ++cit;
    }
  }

  /**
   * 检查缺失的必须参数
   */
  for (auto crg : crgs) {
    if (Inspect::IsMore(crg)) continue;

    if (!crg->HasValue()) {
      throw RequiredArgumentMissing(Inspect::NameOf(crg),
                                    Inspect::NameOf(target->command));
    }
  }

  /**
   * 执行命令
   */
  return target->command->Run();
}

inline void Application::Validate() {
  // 命令关键字只能由字母和数字组成
  // 选项标志不能重复
  // 长短选项要满足格式要求
  // 可变参数之后不能再定义其它参数
  // TODO 用到再说，不着急
}

inline void Application::Name(std::string const& name) { Inst().name_ = name; }

inline void Application::Brief(std::string const& brief) {
  Inst().brief_ = brief;
}

inline void Application::Doc(std::string const& doc) { Inst().doc_ = doc; }

inline void Application::Version(std::string const& version) {
  Inst().version_ = version;
}

inline void Application::Author(std::string const& author) {
  Inst().author_ = author;
}

inline void Application::Command(Cmd command,
                                 std::vector<std::string> const& keywords) {
  Scope* scope = &Inst();
  for (auto const& keyword : keywords) {
    auto it = scope->subcommands.find(keyword);
    if (it == scope->subcommands.end()) {
      scope->subcommands[keyword] = Scope();
    }
    scope = &scope->subcommands[keyword];
  }

  scope->command = command;
  command->keywords_ = keywords;
}

inline Opt Application::Option(std::vector<std::string> const& flags) {
  auto opt = std::make_shared<cli::Option>(flags);
  Inst().options_.push_back(opt);
  return opt;
}

inline Application& Application::Inst() {
  static Application inst;
  return inst;
}

}  // namespace cli

#endif