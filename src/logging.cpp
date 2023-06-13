#include "alioth/logging.h"

#include <condition_variable>

#include "alioth/error_handle.h"
#include "spdlog/sinks/ringbuffer_sink.h"

namespace alioth {

namespace {

std::atomic_bool initialized_{false};
static std::vector<spdlog::sink_ptr> sinks_;
static std::mutex sinks_mutex_;

template <typename Fn>
auto UseSinks(Fn fn) {
  if (initialized_.load()) return fn(sinks_);

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  if (!initialized_ && sinks_.empty())
    sinks_.push_back(std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1024));
  return fn(sinks_);
}

class MerakLogger : public spdlog::logger {
 public:
  using spdlog::logger::logger;

  void sink_it_(spdlog::details::log_msg const& msg) override {
    if (initialized_.load()) return spdlog::logger::sink_it_(msg);

    return UseSinks([&](auto& sinks) {
      (void)sinks;
      return spdlog::logger::sink_it_(msg);
    });
  }
};

std::shared_ptr<spdlog::logger> GetLogger(std::string const& name) {
  return UseSinks([&](auto& sinks) {
    auto it = spdlog::get(name);
    if (it) return it;

    it = std::make_shared<MerakLogger>(name, sinks.begin(), sinks.end());
    spdlog::initialize_logger(it);
    return it;
  });
}

void DoInitLogging(std::vector<spdlog::sink_ptr>& localSinks,
                   std::vector<spdlog::sink_ptr> const& inputSinks) {
  if (initialized_.load()) return;

  for (auto sink : localSinks) {
    auto ringbuffer =
        std::dynamic_pointer_cast<spdlog::sinks::ringbuffer_sink_mt>(sink);
    if (!ringbuffer) continue;

    for (auto& buf : ringbuffer->last_raw()) {
      for (auto& sink : inputSinks) {
        sink->log(buf);
      }
    }
  }

  localSinks = inputSinks;
  spdlog::apply_all([&](auto logger) { logger->sinks() = localSinks; });

  initialized_.store(true);
}

}  // namespace

Logger::Logger(std::string const& name) : logger_(GetLogger(name)) {}

spdlog::logger* Logger::operator*() const { return get(); }
spdlog::logger* Logger::operator->() const { return get(); }
spdlog::logger* Logger::get() const { return logger_.get(); }

void InitLogging(std::vector<spdlog::sink_ptr> const& sinks) {
  if (initialized_.load()) return;

  return UseSinks(std::bind(DoInitLogging, std::placeholders::_1, sinks));
}

void PrintErrors(std::exception const& errors) {
  return PrintErrors(Logger("merak"), errors);
}

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(std::vector<std::string> const& errors) {
  return PrintErrors(Logger("merak"), errors);
}

/**
 * 打印相互关联的异常信息
 */
void PrintErrors(Logger const& logger, std::vector<std::string> const& errors) {
  if (errors.empty()) return;

  logger->error("{}", errors[0]);
  for (auto i = 1u; i < errors.size(); ++i) {
    logger->error("  caused by: {}", errors[i]);
  }
}

void PrintErrors(Logger const& logger, std::exception const& errors) {
  PrintErrors(logger, dump_errors(errors));
}

}  // namespace alioth