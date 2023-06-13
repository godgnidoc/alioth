#ifndef __CLI_COMMAND_INL__
#define __CLI_COMMAND_INL__

#include "cli/command.h"

namespace cli {

inline Opt Command::Option(std::vector<std::string> const& flags) {
  auto opt = std::make_shared<cli::Option>(flags);
  options_.push_back(opt);
  return opt;
}

inline Arg Command::Named(std::string const& name) {
  auto arg = std::make_shared<Argument>(name);
  arguments_.push_back(arg);
  return arg;
}

inline Arg Command::Some(std::string const& name) {
  auto arg = std::make_shared<Argument>(name + "...");
  arguments_.push_back(arg);
  return arg;
}

inline Arg Command::More() {
  auto arg = std::make_shared<Argument>("...");
  arguments_.push_back(arg);
  return arg;
}

inline std::string Command::Brief(std::string const& brief) {
  brief_ = brief;
  return brief;
}

inline std::string Command::Doc(std::string const& doc) {
  doc_ = doc;
  return doc;
}

}  // namespace cli

#endif