#ifndef __alioth__
#define __alioth__

#include <set>

#include "cli.hpp"

namespace alioth {

class compiler : public cli::application {
   public:
    compiler();

   protected:
    int compile(cli::commandline cmd);
    int common_entry(cli::commandline cmd, std::function<int(cli::commandline)> entry);
};

}  // namespace alioth

#endif