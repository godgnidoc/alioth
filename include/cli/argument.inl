#ifndef __CLI_ARGUMENT_INL__
#define __CLI_ARGUMENT_INL__

#include "cli/argument.h"

namespace cli {

inline Argument::Argument(std::string const& name) : name_{name} {}

inline bool Argument::HasValue() const { return !values_.empty(); }

inline std::string Argument::Value() const { return values_.at(0); }

inline std::vector<std::string> Argument::Values() const { return values_; }

}  // namespace cli

#endif