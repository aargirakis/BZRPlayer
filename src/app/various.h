#ifndef VARIOUS_H
#define VARIOUS_H

#include <QString>
#include "info.h"
#include "logger.h"

using namespace std;

#define PROVIDE_CLASS_NAME() \
    static constexpr const char* getClassName() { \
        return parseClassName(__PRETTY_FUNCTION__); \
    }

consteval const char *parseClassName(const char *s) {
    const string_view f(s);
    const auto end = f.rfind('(');
    const auto sep = f.rfind("::", end);
    const size_t start = f.rfind(' ', sep);
    return f.substr(start + 1, sep - start - 1).data();
}

class Various {
public:
    Various() = delete;

    PROVIDE_CLASS_NAME()
};

#ifndef NDEBUG
#define logDebugQ(message, source) logQ(message, Level::DEBUG, source)
#else
#define logDebugQ(message, source) do { } while (0)
#endif
#define logInfoQ(message, source) logQ(message, Level::INFO, source)
#define logWarningQ(message, source) logQ(message, Level::WARN, source)
#define logErrorQ(message, source) logQ(message, Level::ERR, source)
#define logFatalQ(message, source) logQ(message, Level::FATAL, source)

constexpr auto utf8Ignore = "UTF-8//IGNORE";
constexpr auto cp932 = "CP932";
constexpr auto windows1252 = "WINDOWS-1252";

typedef struct {
    QString year;
    QString filename;
    QString path;
    QString title;
    QString extension;
    QString id;
    Info *info;
    unsigned int startTime;
    unsigned int startSubsong;
    signed int startSubsongPlayList;
    int subsongs;
    unsigned int length;
    bool unknownLength;
    bool seekable;
} Song;

struct Equalizer {
    QString name;
    float eq32;
    float eq64;
    float eq125;
    float eq250;
    float eq500;
    float eq1000;
    float eq2000;
    float eq4000;
    float eq8000;
    float eq16000;
};

QString groupDigits(size_t number);

QString msToNiceStringExact(unsigned int lenMs, bool displayMilliseconds);

QString fromUtf8OrLatin1(const string &str);

string convertToUtf8(const string &input, const char *encoding);

inline void logQ(const QString &message, const Level level, const string &source) {
    log(message.toStdString(), level, source);
}

#endif // VARIOUS_H
