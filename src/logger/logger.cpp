#include "logger.h"

Logger::Logger() {

}


void Logger::log(std::string& msg, LoggerLevel ll) {
    switch (ll) {
        case(LoggerLevel::INFO):
            logger->info(msg);
            break;
        case(LoggerLevel::DEBUG):
            logger->debug(msg);
            break;
        case(LoggerLevel::WARNING):
            logger->warn(msg);
            break;
        case(LoggerLevel::ERROR):
            logger->error(msg);
            break;
    }
}