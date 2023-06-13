#ifndef __ALIOTH_CLI_COMPILE_H__
#define __ALIOTH_CLI_COMPILE_H__

#include "cli/cli.h"

struct Compile : public cli::Command {
  int Run() override;

  cli::Opt gpath = Option({"-g", "--grammar"})
                       ->Required()
                       ->Argument("grammar-path")
                       ->Brief("grammar file path")
                       ->Doc(
                           "specify the path to the grammar file,"
                           " '-' represents stdin");
  cli::Opt disable_skeleton_hint =
      Option({"-S", "--no-skeleton"})->Brief("Disable skeleton hint");
  cli::Arg source = Named("source-path");
};

#endif