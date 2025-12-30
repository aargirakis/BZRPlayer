#ifndef LOGGER_H
#define LOGGER_H

#include <vector>

using namespace std;

enum class Level { DEBUG, INFO, WARN, ERR, FATAL };

constexpr const char *LevelNames[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};

#ifndef NDEBUG
#define logDebug(message, source) log(message, Level::DEBUG, source)
#else
#define logDebug(message, source) do { } while (0)
#endif
#define logInfo(message, source) log(message, Level::INFO, source)
#define logWarning(message, source) log(message, Level::WARN, source)
#define logError(message, source) log(message, Level::ERR, source)
#define logFatal(message, source) log(message, Level::FATAL, source)

struct LogMessage {
    string message;
    Level level;
    string source;
};

vector<string> fetchLogMessages();

void log(const string &message, Level level, string source); //TODO pass source by ref

#endif //LOGGER_H
