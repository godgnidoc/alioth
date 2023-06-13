#ifndef __CLI_INSPECT_INL__
#define __CLI_INSPECT_INL__

#include "cli/inspect.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

namespace cli {

inline std::string Inspect::Name() { return Application::Inst().name_; }

inline std::string Inspect::Brief() { return Application::Inst().brief_; }

inline std::string Inspect::Doc() { return Application::Inst().doc_; }

inline std::string Inspect::Version() { return Application::Inst().version_; }

inline std::string Inspect::Author() { return Application::Inst().author_; }

inline std::vector<Opt> Inspect::Options() {
  return Application::Inst().options_;
}

inline Scope& Inspect::Commands() { return Application::Inst(); }

inline std::string Inspect::NameOf(Cmd command) {
  return fmt::format("{}", fmt::join(command->keywords_, " "));
}

inline std::vector<std::string> Inspect::KeywordsOf(Cmd command) {
  return command->keywords_;
}

inline std::vector<Opt> Inspect::OptionsOf(Cmd command) {
  return command->options_;
}

inline std::vector<Arg> Inspect::ArgumentsOf(Cmd command) {
  return command->arguments_;
}

inline std::optional<std::string> Inspect::DocOf(Cmd command) {
  return command->doc_;
}

inline std::optional<std::string> Inspect::BriefOf(Cmd command) {
  return command->brief_;
}

inline std::vector<std::string> Inspect::FlagsOf(Opt option) {
  return option->flags_;
}

inline Arg Inspect::ArgumentOf(Opt option) { return option->argument_; }

inline bool Inspect::IsRequired(Opt option) { return option->required_; }

inline bool Inspect::IsRepeatable(Opt option) { return option->repeatable_; }

inline std::optional<std::string> Inspect::DocOf(Opt option) {
  return option->doc_;
}

inline std::optional<std::string> Inspect::BriefOf(Opt option) {
  return option->brief_;
}

inline std::string Inspect::NameOf(Arg argument) { return argument->name_; }

inline bool Inspect::IsSome(Arg argument) {
  auto const& name = argument->name_;
  auto it = name.rbegin();
  if (*it++ != '.') return false;
  if (*it++ != '.') return false;
  if (*it++ != '.') return false;
  return it != name.rend() && *it != '.';
}

inline bool Inspect::IsMore(Arg argument) { return argument->name_ == "..."; }

}  // namespace cli

#endif