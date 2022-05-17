#ifndef LOGMSG_H
#define LOGMSG_H

#include <QList>
#include <QString>
#include <QTime>
//#include <time.h>
//#include <list>
//#include <string>

class Logger {
   public:
    // TODO: set log level(日志等级), such as trace跟踪, debug调试, info通知,
    // warning警告, error错误, fatal致命错误 and so on.
    // TODO: 定时保存到文件，不只是显示到界面，涉及到文件读写
    Logger();
    ~Logger();
    void setMaxMsg(unsigned int maxMsg);
    void setLevel(unsigned int level);  // set display level
    unsigned int getLevel();
    void activateLog();
    void deavtivateLog();
    void activateTime();
    void deactivateTime();
    bool haveMsg();
    bool isLogging();
    int count();
    void pushMsg(const char* msg, unsigned int level = 0);
    QString pullMsg();

   private:
    QList<QString> logList;
    QList<QTime> timeList;
    unsigned int maxMsg;  // the max number of messages
    bool isLogable;
    bool isRegTime;
    unsigned int
        level;  // 显示级别0=全部，1打开，逐步显示更多信息 display level 0=all,
                // 1 an on, display more information progressively
};

#endif  // LOGMSG_H
