#ifndef __resource__
#define __resource__

#include <string>

#include "agent.hpp"
#include "tlog.hpp"
#include "uriz.hpp"
#include "utils.hpp"

namespace alioth {

class source {};

class package {};

class repository {};

class project : public basic_thing {
   public:
    using ref = agent<project>;

   public:
    static ref open(const std::string& path);
};

}  // namespace alioth

#endif