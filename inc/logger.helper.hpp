#ifndef __logger_helper__
#define __logger_helper__

#include "logging.hpp"

namespace alioth::logging {
class helper {
   private:
    logger& l;

   public:
    helper(logger& logger);
    logger& operator *();
    logger* operator ->();
    /** begin-code-gen-mark:logging-helper-def */
    /** file $1$c$(file)$0 not found */
    void file_not_found(severity sev, range rng, const std::string& file);
    /** file $1$c$(file)$0 not found */
    void file_not_found(severity sev, const std::string& file);
    
    /** unrecognized token found $1$c$(token)$0, $1$g$(expected)$0 */
    void unrecognized_token(severity sev, range rng, const std::string& token, const std::string& expected);
    /** unrecognized token found $1$c$(token)$0, $1$g$(expected)$0 */
    void unrecognized_token(severity sev, const std::string& token, const std::string& expected);
    /** end-code-gen-mark:logging-helper-def */
};
}  // namespace alioth::logging

#endif