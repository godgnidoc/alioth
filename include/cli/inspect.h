#ifndef __CLI_INSPECT_H__
#define __CLI_INSPECT_H__

#include "cli/application.h"
#include "cli/argument.h"
#include "cli/command.h"
#include "cli/option.h"

namespace cli {

struct Inspect {
  static std::string Name();
  static std::string Brief();
  static std::string Doc();
  static std::string Version();
  static std::string Author();
  static std::vector<Opt> Options();
  static Scope& Commands();

  static std::string NameOf(Cmd command);
  static std::vector<std::string> KeywordsOf(Cmd command);
  static std::vector<Opt> OptionsOf(Cmd command);
  static std::vector<Arg> ArgumentsOf(Cmd command);
  static std::optional<std::string> DocOf(Cmd command);
  static std::optional<std::string> BriefOf(Cmd command);

  static std::vector<std::string> FlagsOf(Opt option);
  static Arg ArgumentOf(Opt option);
  static bool IsRequired(Opt option);
  static bool IsRepeatable(Opt option);
  static std::optional<std::string> DocOf(Opt option);
  static std::optional<std::string> BriefOf(Opt option);

  static std::string NameOf(Arg argument);
  static bool IsSome(Arg argument);
  static bool IsMore(Arg argument);
};

}  // namespace cli

#endif