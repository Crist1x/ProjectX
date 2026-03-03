#ifndef PROJECT_LOGGER_H
#define PROJECT_LOGGER_H

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <memory>

enum class LoggerLevel {
    INFO, DEBUG, WARNING, ERROR,
};

class Logger {
private:
    std::shared_ptr<spdlog::logger> logger;
    Logger();
public:
    void log(std::string& msg, LoggerLevel ll);
    Logger& getLogger(const std::string& path = "log.txt");
};

#endif