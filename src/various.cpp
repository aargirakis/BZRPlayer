#include "various.h"
#include <QStringList>
#include <sstream>
//various global helper functions
inline const char * const BoolToString(bool b)
{
  return b ? "true" : "false";
}
QString groupDigits(int number)
{
    QString num = QString::number(number);
    for(int i = num.length();i>0;i-=3)
    {
        num.insert(i," ");
    }
    return num.left(num.length()-1);
}
/*!
        Takes a time in the format HH:MM:SS and returns the time in milliseconds
*/
unsigned int stringTimeToMs(QString strTime)
{
    QStringList list = strTime.split(":");

    if(list.size()!=3) return 0; //wrong format

    return list.at(0).toUInt()*60*1000+ //minutes
            list.at(1).toUInt()*1000+	//seconds
            list.at(2).toUInt()*10;		//100th of a second
}

QString msToNiceStringExact(unsigned int lenms, bool displayMilliseconds)
{
    string songLength="";

    int ms;
    int sec;
    int min;
    int hour;
    string strMs="";


    string strSec;
    string strMin;
    string strHour;

    unsigned int length=lenms/1000;

    if(displayMilliseconds)
    {
        ms=lenms%1000;
    }
    sec=length%60;
    min=length/60%60;
    hour=length/3600;
    stringstream ss2;
    stringstream ss3;
    stringstream ss4;
    stringstream ss5;

    if(displayMilliseconds)
    {
        if(ms<10)
        {
            ss2 << "00" << ms;
        }
        else if(ms<100)
        {
            ss2 << "0" << ms;
        }
        else
        {
            ss2 << ms;
        }
    }

    if(sec<10)
    {
        ss3 << "0" << sec;
    }
    else
    {
        ss3 << sec;
    }


    if(displayMilliseconds)
    {
        ss2 >> strMs;
        strMs="." + strMs;
    }
    ss3 >> strSec;



    if(hour>0)
    {
        if(min<10)
        {
            ss4 << "0" << min;
            ss4 >> strMin;
        }
        else
        {
            ss4 << min;
            ss4 >> strMin;
        }
        ss5 << hour;
        ss5 >> strHour;
        songLength = strHour + ":" + strMin + ":" + strSec+ strMs;
    }
    else
    {
        ss4 << min;
        ss4 >> strMin;
        songLength = strMin + ":" + strSec+strMs;
    }
    if(lenms==0xffffffff)
    {
        if(displayMilliseconds)
        {
            songLength="??:??.???";
        }
        else
        {
            songLength="??:??";
        }
    }
    return QString(songLength.c_str());
}
