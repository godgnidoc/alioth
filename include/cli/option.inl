#ifndef __CLI_OPTION_INL__
#define __CLI_OPTION_INL__

#include "cli/argument.h"
#include "cli/option.h"

namespace cli {

inline Option::Option(std::vector<std::string> const& flags) : flags_{flags} {}

inline Opt Option::Required() {
  required_ = true;
  return shared_from_this();
}

inline Opt Option::Repeatable() {
  repeatable_ = true;
  return shared_from_this();
}

inline Opt Option::Argument(std::string const& name) {
  argument_ = std::make_shared<cli::Argument>(name);
  return shared_from_this();
}

inline Opt Option::Brief(std::string const& brief) {
  brief_ = brief;
  return shared_from_this();
}

inline Opt Option::Doc(std::string const& doc) {
  doc_ = doc;
  return shared_from_this();
}

inline bool Option::HasValue() const { return !values_.empty(); }

inline std::string Option::Value() const { return values_.at(0); }

inline std::vector<std::string> Option::Values() const { return values_; }

}  // namespace cli

#endif