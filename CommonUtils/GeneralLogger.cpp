#include "GeneralLogger.h"

#include <vector>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace CommonUtils
{

std::shared_ptr<spdlog::async_logger> GeneralLogger::generalLogger;
std::shared_ptr<spdlog::async_logger> GeneralLogger::traceLogger;

GeneralLogger::~GeneralLogger()
{
    GPINFO("General Logger Destructor");
    traceLogger->dump_backtrace();
    traceLogger->flush();
}

void GeneralLogger::init(const std::string &logNameBase)
{
    if (_isInited) return;
    _isInited = true;

    spdlog::init_thread_pool(8192, 1);

    constexpr size_t maxFileSize = 1024 * 1024 * 5;
    constexpr size_t maxNumFiles = 3;

    std::string logfileName = logNameBase + ".log";
    std::string tracefileName = logNameBase + "_trace.log";

    /**
     * General Logging: Debugs or higher go to the Console.  Infos
     * or higher go the log file.  Trace logs do not show up in
     * the general logger, see trace logger for that!
     */
    auto generalStdOutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    generalStdOutSink->set_level(spdlog::level::debug);
    auto generalRotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfileName, maxFileSize, maxNumFiles);
    generalRotatingSink->set_level(spdlog::level::info);

    std::vector<spdlog::sink_ptr> generalSinks{generalStdOutSink, generalRotatingSink};
    generalLogger = std::make_shared<spdlog::async_logger>(
        generalLoggerName, generalSinks.begin(), generalSinks.end(),
        spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
    generalLogger->set_pattern(logPattern);
    generalLogger->set_level(spdlog::level::debug);
    GPINFO("General Purpose Logger is Created! {}", logfileName);

    /**
     * Trace Logging: Trace Statements should go to the trace logger.
     * they do not show up in the std out or in a log file, but go to
     * a circular buffer in the log object and dumped when  a segfault
     * is caught. 
     */
    auto traceStdOutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto traceRotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(tracefileName, maxFileSize, maxNumFiles);
    std::vector<spdlog::sink_ptr> traceSinks{traceStdOutSink, traceRotatingSink};
    traceLogger = std::make_shared<spdlog::async_logger>(
        tracefileName, traceSinks.begin(), traceSinks.end(),
        spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
    traceLogger->set_level(spdlog::level::err);
    traceLogger->enable_backtrace(128);
    traceLogger->set_pattern(logPattern);
    GPINFO("Trace Logger is Created! {} ", tracefileName);
    GPTRACE("Trace Logger is Created!");

    spdlog::flush_every(std::chrono::seconds(1));
    spdlog::register_logger(generalLogger);
    spdlog::register_logger(traceLogger);

}

}
