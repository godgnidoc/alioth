#ifndef __ALIOTH_CLI_FRAMEWORK_H__
#define __ALIOTH_CLI_FRAMEWORK_H__

#include "cli/cli.h"

struct Framework : public cli::Command {
  int Run() override;

  cli::Arg gpath = Named("grammar-path");
  cli::Opt opath = Option({"-o", "--output"})
                       ->Required()
                       ->Argument("output-dir")
                       ->Brief("output directory");
};

#endif