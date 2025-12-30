#include <iconv.h>
#include <iostream>
#include <sstream>
#include "various.h"

using namespace std;

QString groupDigits(const size_t number) {
    QString num = QString::number(number);

    for (int i = num.length(); i > 0; i -= 3) {
        num.insert(i, " ");
    }

    return num.left(num.length() - 1);
}

QString msToNiceStringExact(unsigned int lenMs, bool displayMilliseconds) {
    string songLength;

    int ms;
    int sec;
    int min;
    int hour;
    string strMs = "";
    string strSec;
    string strMin;

    unsigned int length = lenMs / 1000;

    if (displayMilliseconds) {
        ms = lenMs % 1000;
    }

    sec = length % 60;
    min = length / 60 % 60;
    hour = length / 3600;
    stringstream ss2;
    stringstream ss3;
    stringstream ss4;

    if (displayMilliseconds) {
        if (ms < 10) {
            ss2 << "00" << ms;
        } else if (ms < 100) {
            ss2 << "0" << ms;
        } else {
            ss2 << ms;
        }
    }

    if (sec < 10) {
        ss3 << "0" << sec;
    } else {
        ss3 << sec;
    }

    if (displayMilliseconds) {
        ss2 >> strMs;
        strMs = "." + strMs;
    }

    ss3 >> strSec;


    if (hour > 0) {
        stringstream ss5;
        string strHour;

        if (min < 10) {
            ss4 << "0" << min;
            ss4 >> strMin;
        } else {
            ss4 << min;
            ss4 >> strMin;
        }

        ss5 << hour;
        ss5 >> strHour;
        songLength = strHour + ":" + strMin + ":" + strSec + strMs;
    } else {
        ss4 << min;
        ss4 >> strMin;
        songLength = strMin + ":" + strSec + strMs;
    }

    if (lenMs == -1) {
        if (displayMilliseconds) {
            songLength = "??:??.???";
        } else {
            songLength = "??:??";
        }
    }

    return songLength.c_str();
}

QString fromUtf8OrLatin1(const string &str) {
    const QByteArray byteArray = QByteArray::fromStdString(str);

    if (QString utf8str = QString::fromUtf8(byteArray); byteArray == utf8str.toUtf8()) {
        return utf8str;
    }

    return QString::fromLatin1(str);
}

string convertToUtf8(const string &input, const char *encoding) {
    const auto cd = iconv_open(utf8Ignore, encoding);

    if (cd == reinterpret_cast<iconv_t>(-1)) {
        logError(string("Iconv error opening descriptor: ") + strerror(errno), Various::getClassName());
        return input;
    }

    auto inputBytes = input.size();
    auto inputPtr = const_cast<char *>(input.data());
    constexpr unsigned int UTF8_CHAR_MAX_SIZE = 4;
    size_t outputBytes = inputBytes * UTF8_CHAR_MAX_SIZE;
    string output(outputBytes, NULL);
    auto outputPtr = output.data();

    const size_t result = iconv(cd, &inputPtr, &inputBytes, &outputPtr, &outputBytes);

    if (iconv_close(cd) == -1) {
        logError(string("Iconv descriptor deallocation error: ") + strerror(errno), Various::getClassName());
    }

    if (result == -1) {
        logError(string("Iconv conversion error from ") + encoding + " to " + utf8Ignore + ": " + strerror(errno),
                 Various::getClassName());
        return input;
    }

    output.resize(output.size() - outputBytes);
    return output;
}
