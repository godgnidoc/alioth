#ifndef __ALIOTH_CLI_SKELETON_H__
#define __ALIOTH_CLI_SKELETON_H__

#include "cli/cli.h"

struct Skeleton : public cli::Command {
  int Run() override;

  cli::Arg gpath = Named("grammar-path");
};

#endif