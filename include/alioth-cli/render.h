#ifndef __ALIOTH_CLI_RENDER_H__
#define __ALIOTH_CLI_RENDER_H__

#include "cli/cli.h"

struct Render : public cli::Command {
  int Run() override;

  cli::Opt tpath = Option({"-t", "--template"})
                       ->Required()
                       ->Argument("template-path")
                       ->Brief("template file path")
                       ->Doc("specify the path to the template file");
  cli::Opt disable_builtin_pipe =
      Option({"-P", "--no-builtin-pipe"})->Brief("Disable builtin pipes");
  cli::Arg source = Named("model-path");
};

#endif