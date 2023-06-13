#ifndef __ALIOTH_CLI_PARSE_H__
#define __ALIOTH_CLI_PARSE_H__

#include "cli/cli.h"

struct Parse : public cli::Command {
  int Run() override;

  cli::Arg source = Named("source-path");

  cli::Opt gpath = Option({"-g", "--grammar"})
                       ->Required()
                       ->Argument("grammar-path")
                       ->Brief("grammar file path")
                       ->Doc(
                           "specify the path to the grammar file,"
                           " '-' represents stdin");

  cli::Opt disable_compact = Option({"-C", "--no-compact"});

  cli::Opt hint_with_skeleton = Option({"-s", "--use-skeleton"});

  cli::Opt unfold_terminals = Option({"-u", "--unfold-terminals"});

  cli::Opt flatten_output = Option({"-f", "--flatten-output"});

  cli::Opt text_key = Option({"--text"})->Argument("text-key");
  cli::Opt id_key = Option({"--id"})->Argument("id-key");
  cli::Opt name_key = Option({"--name"})->Argument("name-key");
  cli::Opt range_key = Option({"--range"})->Argument("range-key");
  cli::Opt formula_key = Option({"--formula"})->Argument("formula-key");
  cli::Opt origin_key = Option({"--origin"})->Argument("origin-key");
  cli::Opt form_key = Option({"--form"})->Argument("form-key");
};

#endif