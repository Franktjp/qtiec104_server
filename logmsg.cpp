#include "logmsg.h"
using namespace std;

Logger::Logger() {
    this->maxMsg = 1000;
    this->isLogable = true;
    this->isRegTime = false;
    this->level = 0;
}

Logger::~Logger() {
    this->logList.clear();  // clear lists, sync
    this->timeList.clear();
}

void Logger::setMaxMsg(unsigned int maxMsg) {
    this->maxMsg = maxMsg;
}

void Logger::setLevel(unsigned int level) {
    this->level = level;
}

unsigned int Logger::getLevel() {
    return this->level;
}

void Logger::activateLog() {
    this->isLogable = true;
}

void Logger::deavtivateLog() {
    this->logList.clear();
    this->timeList.clear();
    this->isLogable = false;
}

void Logger::activateTime() {
    this->logList.clear();  // clear lists, sync
    this->timeList.clear();
    this->isRegTime = true;
}

void Logger::deactivateTime() {
    this->isRegTime = false;
}

void Logger::clear() {
    this->logList.clear();
    this->timeList.clear();
}

bool Logger::haveMsg() {
    return !this->logList.empty();
}

bool Logger::isLogging() {
    return this->isLogable;
}

int Logger::count() {
    return this->logList.size();
}

/**
 * @brief LogMsg::pushMsg 将消息放入队列
 * @param msg
 * @param level
 */
void Logger::pushMsg(const char* msg, unsigned int level) {
    if (this->isLogable && this->level <= level &&
            this->logList.size() < this->maxMsg) {
        this->logList.push_back(msg);
        if (this->isRegTime) {
            timeList.push_back(time(NULL));
        }
    }
}

/**
 * @brief LogMsg::pullMsg 从队列中删除消息
 * @return
 */
string Logger::pullMsg() {
    if (logList.empty() || !isLogable)
        return "";

    string s;
    time_t t_front = NULL;

    s = logList.front(); // 返回列表中第一个元素并删除列表中该元素
    logList.pop_front();

    if (isRegTime) {
        char buf[201];
        static time_t t_pre;
        t_front = timeList.front();
        timeList.pop_front();
        if (t_front != t_pre) {
            struct tm* timeinfo;
            timeinfo = localtime(&t_front);
            strftime(buf, sizeof(buf), "%H:%M:%S", timeinfo);
            s = buf + string(" - ") + s;
        } else {
            s = "           " + s;
        }
        t_pre = t_front;
    }
    saveMsg(s);
    return s;
}

/**
 * @brief Logger::saveMsg - Save message to file. The log files are in a folder
 * called build***
 * @param msg
 */
void Logger::saveMsg(const std::string& msg) {
    time_t rawtime;
    static mutex mx;
    struct tm* timeInfo;
    std::string day;
    std::string file;
    std::ofstream outFile;

    mx.lock();

    time(&rawtime);
    timeInfo = localtime(&rawtime);
    day = to_string(timeInfo->tm_year + 1900) + string("_") +
            to_string(timeInfo->tm_mon + 1) + string("_") +
            to_string(timeInfo->tm_mday);
    file = string("log/log_") + day +
            string(".txt");  // such as `log_2022_6_9.txt`
    outFile.open(file, ios::out | ios::app);
    if (outFile.is_open()) {
        // NOTE: 检测目录是否存在是可以的，但是不同系统下方式不同，因此省略吧
        outFile << day << ": " << msg << "\n";
        outFile.flush();
        outFile.close();
    }

    mx.unlock();
}
