#ifndef __logger_helper_impl__
#define __logger_helper_impl__

#include "logger.helper.hpp"

namespace alioth::logging {

helper::helper(logger& logger) : l(logger) {}
logger& helper::operator*() { return l; };
logger* helper::operator->() { return &l; };

/** begin-code-gen-mark:logging-helper-impl */
void helper::file_not_found(severity sev, range rng, const std::string& file) { l.emit(sev, rng, 0, {{"file",file},}); }
void helper::file_not_found(severity sev, const std::string& file) { l.emit(sev, range(), 0, {{"file",file},}); }

void helper::unrecognized_token(severity sev, range rng, const std::string& token, const std::string& expected) { l.emit(sev, rng, 1, {{"token",token},{"expected",expected},}); }
void helper::unrecognized_token(severity sev, const std::string& token, const std::string& expected) { l.emit(sev, range(), 1, {{"token",token},{"expected",expected},}); }
/** end-code-gen-mark:logging-helper-impl */
}  // namespace alioth::logging

#endif