#ifndef LOGMSG_H
#define LOGMSG_H

#include <time.h>
#include <list>
#include <string>

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
    std::string pullMsg();

private:
    std::list<std::string> logList;
    std::list<time_t> timeList;
    uint32_t maxMsg;  // the max number of messages
    bool isLogable;
    bool isRegTime;
    uint32_t level;  // 显示级别0=全部，1打开，逐步显示更多信息 display level 0=all,
    // 1 an on, display more information progressively
};

#endif  // LOGMSG_H
