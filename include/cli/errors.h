#ifndef __CLI_ERRORS_H__
#define __CLI_ERRORS_H__

#include <stdexcept>

namespace cli {

class RuntimeError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class CommandNotFound : public RuntimeError {
 public:
  CommandNotFound() : RuntimeError("Command not found") {}
};

class DuplicatedOption : public RuntimeError {
 public:
  DuplicatedOption(std::string const& flag)
      : RuntimeError("Duplicated option: " + flag) {}
};

class RequiredOptionMissing : public RuntimeError {
 public:
  RequiredOptionMissing(std::string const& flag)
      : RuntimeError("Required option missing: " + flag) {}
};

class OptionNotFound : public RuntimeError {
 public:
  OptionNotFound(std::string const& flag, std::string const& command)
      : RuntimeError("Command " + command + " does not have option: " + flag) {}
};

class RequiredArgumentMissing : public RuntimeError {
 public:
  RequiredArgumentMissing(std::string const& name, std::string const& command)
      : RuntimeError("Required argument missing: " + name + " in command " +
                     command) {}
};

class OptionRequiresArgument : public RuntimeError {
 public:
  OptionRequiresArgument(std::string const& flag, std::string const& command)
      : RuntimeError("Option " + flag + " requires argument in command " +
                     command) {}
};


class TooManyArguments : public RuntimeError {
 public:
  TooManyArguments(std::string const& command)
      : RuntimeError("Too many arguments in command " + command) {
  }
};

class ValidateError : public std::logic_error {
 public:
  using logic_error::logic_error;
};

class InvalidKeyword : public ValidateError {
 public:
  InvalidKeyword(std::string const& keyword, std::string const& command)
      : ValidateError("Invalid keyword: " + keyword + " of command " +
                      command) {}
};

class InvalidOptionFlag : public ValidateError {
 public:
  InvalidOptionFlag(std::string const& flag, std::string const& command)
      : ValidateError("Invalid option flag: " + flag + " in command " +
                      command) {}
};

class ConflictOptionFlag : public ValidateError {
 public:
  ConflictOptionFlag(std::string const& flag, std::string const& command)
      : ValidateError("Conflict option flag: " + flag + " in command " +
                      command) {}
};

class InvalidArgumentName : public ValidateError {
 public:
  InvalidArgumentName(std::string const& name, std::string const& command)
      : ValidateError("Invalid argument name: " + name + " in command " +
                      command) {}
};

class UnreachableArgument : public ValidateError {
 public:
  UnreachableArgument(std::string const& name, std::string const& command)
      : ValidateError("Unreachable argument: " + name + " in command " +
                      command) {}
};

}  // namespace cli

#endif