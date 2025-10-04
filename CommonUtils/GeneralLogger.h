#ifndef GENERALLOGGER_H_
#define GENERALLOGGER_H_

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/async.h"


// I don't like macros either, but this is super convenient!
#define GPCRIT(...) SPDLOG_LOGGER_CRITICAL(CommonUtils::GeneralLogger::generalLogger, __VA_ARGS__);
#define GPERROR(...) SPDLOG_LOGGER_ERROR(CommonUtils::GeneralLogger::generalLogger, __VA_ARGS__);
#define GPINFO(...) SPDLOG_LOGGER_INFO(CommonUtils::GeneralLogger::generalLogger, __VA_ARGS__);
#define GPDEBUG(...) SPDLOG_LOGGER_DEBUG(CommonUtils::GeneralLogger::generalLogger, __VA_ARGS__);

// Traces get logged to a backtrace ring-buffer, and dumped on segfault
#define GPTRACE(...) SPDLOG_LOGGER_TRACE(CommonUtils::GeneralLogger::traceLogger, __VA_ARGS__);

namespace CommonUtils
{
// string views would be better, but char*'s pass into spdlog easier!
static constexpr const char * generalLoggerName = "generalLogger";
static constexpr const char * traceLoggerName = "traceLogger";

class GeneralLogger
{
public:
    virtual ~GeneralLogger();
    void init(const std::string &logNameBase);


    static std::shared_ptr<spdlog::async_logger> generalLogger;
    static std::shared_ptr<spdlog::async_logger> traceLogger;
private:
    static constexpr const char * logPattern = "%Y%m%d_%H%M%S.%e [%t][%s::%! %# %l] %v";
    std::atomic_bool _isInited = false;
};

}
#endif