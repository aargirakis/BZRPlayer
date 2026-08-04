#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <utility>
#include "logger.h"

static queue<string> messageQueue;
static mutex queueMutex;
static mutex consoleMutex;

static void writeToConsole(const string &message, const Level level) {
    lock_guard lock(consoleMutex);

    ostream &out = level >= Level::ERR ? cerr : cout;
    ostringstream oss;

    oss << message;
    out << oss.str();
    out.flush();
}

vector<string> fetchLogMessages() {
    vector<string> logMessage; {
        lock_guard lock(queueMutex);

        while (!messageQueue.empty()) {
            logMessage.emplace_back(messageQueue.front());
            messageQueue.pop();
        }
    }

    return logMessage;
}

void log(const string &message, const Level level, string source) {
    if (const size_t pos = source.rfind("::"); pos != string::npos) {
        source = source.substr(0, pos);
    }

    string logMessage = "[";
    logMessage += LevelNames[to_underlying(level)];
    logMessage += "][" + source + "] " + message;

    writeToConsole(logMessage + '\n', level);

    lock_guard lock(queueMutex);
    messageQueue.emplace(move(logMessage));
}
