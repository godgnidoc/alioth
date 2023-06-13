#ifndef __ALIOTH_CLI_TOKENIZE_H__
#define __ALIOTH_CLI_TOKENIZE_H__

#include "cli/cli.h"

struct Tokenize : public cli::Command {
  int Run() override;

  cli::Arg path = Named("source-path");
  cli::Opt gpath = Option({"-g", "--grammar"})
                       ->Required()
                       ->Argument("grammar-path")
                       ->Brief("grammar file path")
                       ->Doc("specify the path to the grammar file");
};

#endif