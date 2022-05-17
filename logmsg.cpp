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
            timeList.push_back(QTime());
        }
    }
}

/**
 * @brief LogMsg::pullMsg 从队列中删除消息
 * @return
 */
QString Logger::pullMsg() {
    if (this->logList.empty() || !this->isLogable)
        return "";

    QString s = logList.front();
    logList.pop_front();

    if (isRegTime) {
        char buf[201];
        static QTime hora_ant;
        QTime hora = this->timeList.front();
        //        time_t hora = timeList.front();
        timeList.pop_front();
        if (hora != hora_ant) {
            struct tm* timeinfo;
            // FIXME:
            //            timeinfo = localtime(&hora);
            strftime(buf, 200, "%H:%M:%S", timeinfo);
            s = buf + s;
        } else {
            s = "         " + s;
        }
        hora_ant = hora;
    }
    return s;
}
