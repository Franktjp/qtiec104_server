#include "iec_base.h"

using namespace std;

iec_base::iec_base() {
    this->slavePort = SERVERPORT;  // set iec104 tcp port to 2404
    strncpy(this->slaveIP, "", 20);
    this->masterAddr = 0;   // originator address
    this->slaveAddr = 0;    // common address os ASDU
    this->vs = 0;
    this->vr = 0;
    this->isClockSYnc = false;
    this->ifCheckSeq = true;
    this->allowSend = true;

    this->numMsgUnack = 0;
    this->numMsgReceived = 0;

}

uint32_t iec_base::getSlavePort() {
    return this->slavePort;
}

void iec_base::setSlavePort(uint32_t port) {
    this->slavePort = port;
}

char* iec_base::getSlaveIP() {
    return this->slaveIP;
}

void iec_base::setSlaveIP(const char* ip) {
    strncpy(this->slaveIP, ip, 20);
}

uint16_t iec_base::getSlaveAddr() {
    return this->slaveAddr;
}

void iec_base::setSlaveAddr(uint16_t addr) {
    this->slaveAddr = addr;
}

/**
 * @brief iec_base::packetReadyTCP - tcp packets are ready to be read from the
 * connection with slave station.
 * TODO: 这是抄的,重写吧
 */
void iec_base::packetReadyTCP() {
    static bool brokenMsg = false;
    static apdu a;
    uint8_t* br;
    br = (uint8_t*)&a;
    int32_t bytesrec;  // TODO: readTCP返回结果，表示获取到的字节数
    uint8_t byt, len;  // 长度
    char buf[1000];

    while (true) {
        if (!brokenMsg) {
            // 找到START
            do {
                bytesrec = readTCP((char*)br, 1);
                if (bytesrec == 0) {
                    return;  // 该函数被调用说明这里不应该被执行到
                }
                byt = br[0];
            } while (byt != START);

            // 找到len
            bytesrec = readTCP((char*)br + 1, 1);
            if (bytesrec == 0) {
                return;
            }
        }

        len = br[1];
        if (len < 4) {  // 小于4说明是错误的帧，重新寻找
            brokenMsg = false;
            log.pushMsg("--> ERROR: invalid frame");
            continue;
        }

        // 读取除了68和length剩下的部分
        waitForReadyRead(len, 300);  // 等待len字节的数据被准备好
        bytesrec = readTCP((char*)br + 2, len);
        if (bytesrec == 0) {
            log.pushMsg("--> broken msg");
            brokenMsg = true;
            return;
        } else if (bytesrec < len) {  // 读取字节数小于len，重新读一次
            int rest = len - bytesrec;
            sprintf(buf, "--> should reread %d(%d in total)", rest, len);
            log.pushMsg(buf);

            // 第二次读
            waitForReadyRead(rest, 300);
            bytesrec = readTCP((char*)br + 2 + bytesrec, rest);
            sprintf(buf, "--> reread %d bytes in fact", bytesrec);
            if (bytesrec < rest) {
                log.pushMsg("--> broken msg");
                brokenMsg = true;
                return;
            }
        }

        // TODO: 可能需要验证ca

        brokenMsg = false;
        if (log.isLogging()) {
            int total = 25;  // 总共输出total字节
            sprintf(buf, "--> size:(%03d) ", int(len + 2));
            for (int i = 0; i < len + 2 && i < total; ++i) {
                sprintf(buf + strlen(buf), "%02x ", br[i]);
            }
            log.pushMsg(buf);
        }
        parse(&a, a.length + 2, true);
        break;
    }
}

void iec_base::send(const struct apdu& wapdu) {
    this->sendTCP(reinterpret_cast<const char*>(&wapdu), wapdu.length + 2);
}

void iec_base::onTcpConnect() {
    this->vr = 0;
    this->vs = 0;
    this->isConnected = true;
    this->log.pushMsg("connect success!");
}

void iec_base::onTcpDisconnect() {
    this->isConnected = false;
    this->log.pushMsg("disconnect success!");
}

/**
 * @brief iec_base::onTimeoutPerSecond - when the timer is up and modify the timeout.
 */
void iec_base::onTimeoutPerSecond() {
    if (isConnected) {
        if (t1Timeout > 0) {
            --t1Timeout;
            if (t1Timeout == 0) {   // 超时，重新连接
                // TODO:
                // 超时，主动关闭
                //            sendStartDtAct();   // t1Timeout < 0表示不需要处理
            }
        }

        if (t2Timeout > 0) {
            --t2Timeout;
            if (t2Timeout == 0) {
                sendMonitorMessage();
                t2Timeout = -1;
            }
        }

        if (t3Timeout > 0) {
            --t3Timeout;
            if (t3Timeout == 0) {
                sendTestfrAct();
            }
        }
    }
    // TODO:
}


void iec_base::sendStartDtCon() {
    struct apdu a;

    log.pushMsg("send STARTDTCON");
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STARTDTCON);
    a.NR = 0;
    send(a);
}

void iec_base::sendStopDtAct() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STOPDTACT);
    a.NR = 0;
    send(a);
    log.pushMsg("send STOPDTACT");
}

void iec_base::sendStopDtCon() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STOPDTCON);
    a.NR = 0;
    send(a);
    this->log.pushMsg("send STOPDTCON");
}

void iec_base::sendTestfrAct() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(TESTFRACT);
    a.NR = 0;
    send(a);
    this->log.pushMsg("send TESTFRACT");
}

void iec_base::sendTestfrCon() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(TESTFRCON);
    a.NR = 0;
    send(a);
    this->log.pushMsg("send TESTFRCON");
}

void iec_base::generalInterrogationCon() {
    struct apdu a;
    stringstream ss;

    a.start = START;
    a.length = 0x0E;
    a.NS = vs << 1;
    a.NR = vr << 1;
    a.head.type = INTERROGATION;
    a.head.num = 1;
    a.head.sq = 0;
    a.head.cot = ACTCONFIRM;  // 7: 激活确认
    //    a.head.t = 0;
    //    a.head.pn = 0;
    a.head.oa = masterAddr;
    a.head.ca = slaveAddr;
    a.nsq100.ioa16 = 0x00;
    a.nsq100.ioa8 = 0x00;
    a.nsq100.obj.qoi = 0x14;

    if (allowSend) {
        send(a);
        ++vs;
        ++numMsgUnack;
        if (numMsgUnack == SERVERK) {
            allowSend = false;
        }
    } else {
        ss.str("");
        ss << "WARNING: 已达到最大未确认报文数k(" << SERVERK << "), 停止报文发送";
        log.pushMsg(ss.str().c_str());
    }
}

void iec_base::generalInterrogationEnd() {
    struct apdu a;
    stringstream ss;

    a.start = START;
    a.length = 0x0E;
    a.NS = vs << 1;
    a.NR = vr << 1;
    a.head.type = INTERROGATION;  // TODO: 为何是INTERROGATION
    a.head.num = 1;
    a.head.sq = 0;
    a.head.cot = ACTTERM;  // cause: 10(激活终止)
    //    a.head.t = 0;
    //    a.head.pn = 0;
    //    a.head.oa = ...
    a.head.ca = masterAddr;
    a.nsq100.ioa16 = 0x00;
    a.nsq100.ioa8 = 0x00;
    a.nsq100.obj.qoi = 0x14;  // 召唤品质描述词

    if (allowSend) {
        send(a);
        ++vs;
        ++numMsgUnack;
        if (numMsgUnack == SERVERK) {
            allowSend = false;
        }
    } else {
        ss.str("");
        ss << "WARNING: 已达到最大未确认报文数k(" << SERVERK << "), 停止报文发送";
        log.pushMsg(ss.str().c_str());
    }
}

/**
 * @brief iec_base::sendTelemetering - 模拟并发送遥测数据
 * @param type  - 类型标识
 * @param sq    - 是否连续值
 */
void iec_base::sendTelemetering(uint8_t type, bool sq) {
    stringstream ss;
    struct apdu* papdu;
    int num;
    int len;

    switch (type) {
    case M_ME_NC_1: {   // 13: 测量值, 短浮点数
        num = 2;
        if (sq) {
            // 计算报文大小
            len = sizeof((*papdu).start) + sizeof((*papdu).length) +
                    sizeof((*papdu).NR) + sizeof((*papdu).NS) +
                    sizeof((*papdu).head) + 3 /* 信息对象地址 */ +
                    num * sizeof(struct iec_type13);

            papdu = (struct apdu*) new uint8_t[len];

            papdu->length = len - 2;

            papdu->sq13.ioa16 = 0x0001;
            papdu->sq13.ioa8 = 0x00;
            papdu->sq13.obj[0].frac = 1.23;
            papdu->sq13.obj[0].ov = 0;
            papdu->sq13.obj[0].res = 0;
            papdu->sq13.obj[0].bl = 0;
            papdu->sq13.obj[0].sb = 0;
            papdu->sq13.obj[0].nt = 0;
            papdu->sq13.obj[0].iv = 0;

            papdu->sq13.obj[1].frac = 1.23;
            papdu->sq13.obj[1].ov = 1;
            papdu->sq13.obj[1].res = 0;
            papdu->sq13.obj[1].bl = 1;
            papdu->sq13.obj[1].sb = 1;
            papdu->sq13.obj[1].nt = 1;
            papdu->sq13.obj[1].iv = 1;
        } else {
            len = sizeof((*papdu).start) + sizeof((*papdu).length) +
                    sizeof((*papdu).NR) + sizeof((*papdu).NS) +
                    sizeof((*papdu).head) + num * (3 /* 信息对象地址 */ + sizeof(struct iec_type13));

            papdu = (struct apdu*) new uint8_t[len];

            papdu->length = len - 2;

            papdu->nsq13[0].ioa16 = 0x0001;
            papdu->nsq13[0].ioa8 = 0x00;
            papdu->nsq13[0].obj.frac = 1.23;
            papdu->nsq13[0].obj.ov = 0;
            papdu->nsq13[0].obj.res = 0;
            papdu->nsq13[0].obj.bl = 0;
            papdu->nsq13[0].obj.sb = 0;
            papdu->nsq13[0].obj.nt = 0;
            papdu->nsq13[0].obj.iv = 0;

            papdu->nsq13[1].ioa16 = 0x0001;
            papdu->nsq13[1].ioa8 = 0x00;
            papdu->nsq13[1].obj.frac = 1.23;
            papdu->nsq13[1].obj.ov = 0;
            papdu->nsq13[1].obj.res = 0;
            papdu->nsq13[1].obj.bl = 0;
            papdu->nsq13[1].obj.sb = 0;
            papdu->nsq13[1].obj.nt = 0;
            papdu->nsq13[1].obj.iv = 0;
        }

        papdu->start = START;
        papdu->NS = vs << 1;
        papdu->NR = vr << 1;
        papdu->head.type = type;
        papdu->head.num = 2;
        papdu->head.sq = sq ? 1 : 0;
        papdu->head.cot = INTROGEN; // 传输原因：响应站召唤
        papdu->head.oa = slaveAddr;
        papdu->head.ca = masterAddr; // ASDU公共地址
    }
        break;
    default:
        ss.str("");
        ss << "ERROR: type " << uint32_t(type) << " is not implemented!";
        log.pushMsg(ss.str().c_str());
        return;
        break;
    }

    if (allowSend) {
        send(*papdu);
        ++vs;
        ++numMsgUnack;
        if (numMsgUnack == SERVERK) {
            allowSend = false;
        }
    } else {
        ss.str("");
        ss << "WARNING: 已达到最大未确认报文数k(" << SERVERK << "), 停止报文发送";
        log.pushMsg(ss.str().c_str());
    }

    delete[] papdu;
}

/**
 * @brief iec_base::sendTelecommunitcating - 模拟并发送遥信数据
 * TODO: Add more types.
 * @param type  - 类型标识
 * @param sq    - 是否连续值
 */
void iec_base::sendTelecommunitcating(uint8_t type, bool sq) {
    stringstream ss;
    struct apdu* papdu;
    int num;
    int len;

    switch(type) {
    case M_SP_NA_1: {   // 1: single-point information 不带时标的单点信息
        num = 2;
        if (sq) {
            // 计算报文大小
            len = sizeof((*papdu).start) + sizeof((*papdu).length) +
                    sizeof((*papdu).NR) + sizeof((*papdu).NS) +
                    sizeof((*papdu).head) + 3 /* 信息对象地址 */ +
                    num * sizeof(struct iec_type1);

            papdu = (struct apdu*) new uint8_t[len];

            papdu->length = len - 2;

            papdu->sq1.ioa16 = 0x0001;
            papdu->sq1.ioa8 = 0x00;
            papdu->sq1.obj[0].spi = 0;
            papdu->sq1.obj[0].res = 0;
            papdu->sq1.obj[0].bl = 0;
            papdu->sq1.obj[0].sb = 0;
            papdu->sq1.obj[0].nt = 0;
            papdu->sq1.obj[0].iv = 0;

            papdu->sq1.obj[1].spi = 0;
            papdu->sq1.obj[1].res = 0;
            papdu->sq1.obj[1].bl = 0;
            papdu->sq1.obj[1].sb = 0;
            papdu->sq1.obj[1].nt = 0;
            papdu->sq1.obj[1].iv = 0;
        } else {
            len = sizeof((*papdu).start) + sizeof((*papdu).length) +
                    sizeof((*papdu).NR) + sizeof((*papdu).NS) +
                    sizeof((*papdu).head) + num * (3 /* 信息对象地址 */ + sizeof(struct iec_type1));

            papdu = (struct apdu*) new uint8_t[len];

            papdu->length = len - 2;

            papdu->nsq1[0].ioa16 = 0x0001;
            papdu->nsq1[0].ioa8 = 0x00;
            papdu->nsq1[0].obj.spi = 0;
            papdu->nsq1[0].obj.res = 0;
            papdu->nsq1[0].obj.bl = 0;
            papdu->nsq1[0].obj.sb = 0;
            papdu->nsq1[0].obj.nt = 0;
            papdu->nsq1[0].obj.iv = 0;

            papdu->nsq1[1].ioa16 = 0x0001;
            papdu->nsq1[1].ioa8 = 0x00;
            papdu->nsq1[1].obj.spi = 0;
            papdu->nsq1[1].obj.res = 0;
            papdu->nsq1[1].obj.bl = 0;
            papdu->nsq1[1].obj.sb = 0;
            papdu->nsq1[1].obj.nt = 0;
            papdu->nsq1[1].obj.iv = 0;
        }

        papdu->start = START;
        papdu->NS = vs << 1;
        papdu->NR = vr << 1;
        papdu->head.type = type;
        papdu->head.num = 2;
        papdu->head.sq = sq ? 1 : 0;
        papdu->head.cot = INTROGEN; // 传输原因：响应站召唤
        papdu->head.oa = slaveAddr;
        papdu->head.ca = masterAddr; // ASDU公共地址
    }
        break;

    default:
        ss.str("");
        ss << "ERROR: type " << uint32_t(type) << " is not implemented!";
        log.pushMsg(ss.str().c_str());
        return;
        break;
    }

    if (allowSend) {
        send(*papdu);
        ++vs;
        ++numMsgUnack;
        if (numMsgUnack == SERVERK) {
            allowSend = false;
        }
    } else {
        ss.str("");
        ss << "WARNING: 已达到最大未确认报文数k(" << SERVERK << "), 停止报文发送";
        log.pushMsg(ss.str().c_str());
    }

    delete[] papdu;
}

/**
 * @brief iec_base::sendClockSyncCon - 时钟同步确认报文
 * TODO: address and so on.
 */
void iec_base::sendClockSyncCon() {
    struct apdu wapdu;
    stringstream ss;
    time_t t = time(nullptr);
    struct tm* timeinfo = localtime(&t);
    uint32_t addr24 = 0;  // 24位信息对象地址

    wapdu.start = START;
    wapdu.length = sizeof(wapdu.NR) + sizeof(wapdu.NS) + sizeof(wapdu.head) +
            sizeof(wapdu.nsq103);
    wapdu.NS = vs << 1;
    wapdu.NR = vr << 1;
    wapdu.head.type = C_CS_NA_1;
    wapdu.head.num = 1;  // 单信息元素
    wapdu.head.sq = 0;   // sq = 0
    wapdu.head.cot = ACTIVATION;  // 控制方向传送原因=激活
    wapdu.head.pn = 0;
    wapdu.head.t = 0;
    wapdu.head.oa = slaveAddr;
    wapdu.head.ca = masterAddr;

    // TODO:
    wapdu.nsq103.ioa16 = (uint16_t)addr24;
    wapdu.nsq103.ioa8 = (uint8_t)(addr24 >> 16);
    //    cmd.nsq103.ioa16 = (obj->address & 0xFFFF);
    //    cmd.nsq103.ioa8 = (obj->address >> 16);

    wapdu.nsq103.obj.time.year = timeinfo->tm_year % 100;  // [0...99]
    wapdu.nsq103.obj.time.mon = timeinfo->tm_mon + 1;
    wapdu.nsq103.obj.time.dmon = timeinfo->tm_mday;
    wapdu.nsq103.obj.time.dweek = timeinfo->tm_wday;
    wapdu.nsq103.obj.time.hour = timeinfo->tm_hour;
    wapdu.nsq103.obj.time.min = timeinfo->tm_min;
    wapdu.nsq103.obj.time.mesc = timeinfo->tm_sec * 1000;
    wapdu.nsq103.obj.time.res1 = wapdu.nsq103.obj.time.res2 =
            wapdu.nsq103.obj.time.res3 = wapdu.nsq103.obj.time.res4 = 0;
    wapdu.nsq103.obj.time.iv = 0;
    wapdu.nsq103.obj.time.su = 0;


    if (allowSend) {
        send(wapdu);
        ++vs;
        ++numMsgUnack;
        if (numMsgUnack == SERVERK) {
            allowSend = false;
        }
    } else {
        ss.str("");
        ss << "WARNING: 已达到最大未确认报文数k(" << SERVERK
           << "), 停止报文发送";
        log.pushMsg(ss.str().c_str());
    }

    return;

    // TODO: 啥意思捏，为啥return了
    if (log.isLogging()) {
        ss.str("");
        ss << "CLOCK SYNCHRONIZATION\n    ADDRESS" << (uint32_t)addr24
           << ", TYPE: " << (uint32_t)wapdu.head.type
           << ", COT: " << (uint32_t)wapdu.head.cot;

        ss << "\n    TIME.YEAR" << (uint32_t)wapdu.nsq103.obj.time.year  // 年 2022
           << ", TIME.MONTH" << (uint32_t)wapdu.nsq103.obj.time.mon      // 月 5
           << ", TIME.DAY" << (uint32_t)wapdu.nsq103.obj.time.dmon       // 日 24
           << ", TIME.HOUR" << (uint32_t)wapdu.nsq103.obj.time.hour      // 时 14
           << ", TIME.MIN" << (uint32_t)wapdu.nsq103.obj.time.min        // 分 53
           << ", TIME.SRC" << (uint32_t)timeinfo->tm_sec;             // 秒 21
        log.pushMsg(ss.str().c_str());
    }
}

/**
 * @brief iec_base::sendMonitorFrame
 */
void iec_base::sendMonitorMessage() {
    struct apdu wapdu;
    stringstream ss;
    wapdu.start = START;
    wapdu.length = 4;
    wapdu.NS = SUPERVISORY;
    wapdu.NR = vr << 1;
    send(wapdu);
    ss.str("");
    ss << "send monitor frame(S), the N(R) is " << wapdu.NR;
    log.pushMsg(ss.str().c_str());
}

/**
 * @brief iec_base::parse
 * @param papdu
 * @param size
 * @param isSend - If we want to send the result to server. This parameter
 */
void iec_base::parse(struct apdu* papdu, int size, bool isSend) {
    struct apdu wapdu;  // 缓冲区组装发送apdu
    uint16_t vrReceived;
    stringstream ss;

    if (papdu->start != START) {
        log.pushMsg("ERROR: no start in frame");
        return;
    }

    if (papdu->head.ca != masterAddr && papdu->length > 6) {
        log.pushMsg("ERROR: parse ASDU with unexpected origin");
        return;
    }

    if (size == 6) {  // U格式APDU长度为6（2 + 4）
        switch (papdu->NS) {
        case STARTDTACT: {
            // 子站：发送确认建立通信链路给主站
            log.pushMsg("receive STARTDTACT");
            sendStartDtCon();
        } break;
        case STOPDTACT: {  // 停止数据传送命令
            log.pushMsg("receive STOPDTACT");
            sendStopDtCon();
        } break;
        case STOPDTCON:  // 停止数据传送命令确认
            // TODO:
            break;
        case TESTFRACT: {
            // 链路测试命令
            log.pushMsg("receive TESTFRACT");
            sendTestfrCon();
        } break;
        case SUPERVISORY: { // S帧
            log.pushMsg("INFO: receive SUPERVISORY");
            if (vs >= papdu->NR) {  // NOTE: 考虑到NR可能比当前vs小
                numMsgUnack = vs - papdu->NR;
                if (numMsgUnack < SERVERK) {
                    allowSend = true;
                }
            } else {
                ss.str("");
                ss << "ERROR: wrong N(R) of SUPERVISORY message=" << papdu->NR
                   << ", while current vs is " << vs;
                log.pushMsg(ss.str().c_str());
                // TODO: disconnect
                return;
            }
        }
            break;
        default:
            log.pushMsg("ERROR: unknown control message");
            break;
        }
    } else {
        // TODO:
        // code...

        // check vr
        vrReceived = papdu->NS >> 1;
        if (vrReceived != vr) {
            ss.str("");
            ss << "ERROR: N(R) is " << uint32_t(this->vr) << ", but the N(S) of I message is " << uint32_t(vrReceived);
            log.pushMsg(ss.str().c_str());
            if (ifCheckSeq) {
                ss.str("");
                ss << "WARNING: start to disconnect tcp";
                log.pushMsg(ss.str().c_str());
                tcpDisconnect();
                return;
            }
        } else {
            ++vr;
        }

        // check vs
        if (vs >= papdu->NR) {
            numMsgUnack = vs - papdu->NR;
            if (numMsgUnack < SERVERK) {
                allowSend = true;
            }
        } else {
            ss.str("");
            // TODO: 输出好难看
            ss << "ERROR: wrong N(R) of type(" << papdu->head.type
               << ") msg, the N(R) is " << papdu->NR << ", while current vs is "
               << vs;
            log.pushMsg(ss.str().c_str());
            // TODO: disconnect
            return;
        }

        ss.str("");
        ss << "     COT: " << papdu->head.ca << "; TYP: " << papdu->head.type
           << "; COT: " << int(papdu->head.cot)
           << "; SQ: " << (unsigned int)(papdu->head.sq)
           << "; NUM: " << papdu->head.num;

        log.pushMsg(ss.str().c_str());

        switch (papdu->head.type) {
        case M_SP_NA_1: {  // 1: single-point information
            struct iec_type1* pobj;
            // TODO:

        } break;
        case M_SP_TA_1:  // 2: single-point information with time tag(cp24time2a)
            break;
        case M_BO_NA_1: {  // 7: bitstring of 32 bits
            // 监视方向过程信息的应用服务数据单元
            struct iec_type7* pobj;
            struct iec_obj* piecarr = new iec_obj[papdu->head.num];
            uint32_t addr24 = 0;  // 24位信息对象地址

            for (int i = 0; i < papdu->head.num; ++i) {
                if (papdu->head.sq) {
                    pobj = &papdu->sq7.obj[i];
                    if (i == 0) {
                        addr24 = papdu->sq7.ioa16 +
                                ((uint32_t)papdu->sq7.ioa8 << 16);
                    } else {
                        ++addr24;
                    }
                } else {
                    pobj = &papdu->nsq7[i].obj;
                    addr24 = papdu->nsq7[i].ioa16 +
                            ((uint32_t)papdu->nsq7[i].ioa8 << 16);
                }

                piecarr[i].address = addr24;
                piecarr[i].type = papdu->head.type;
                piecarr[i].ca = papdu->head.ca;
                piecarr[i].cause = papdu->head.cot;
                piecarr[i].pn = papdu->head.pn;
                piecarr[i].bsi = pobj->bsi;
                piecarr[i].value = (float)pobj->bsi;
                piecarr[i].ov = pobj->ov;
                piecarr[i].bl = pobj->bl;
                piecarr[i].sb = pobj->sb;
                piecarr[i].nt = pobj->nt;
                piecarr[i].iv = pobj->iv;
            }

            dataIndication(piecarr, papdu->head.num);
            delete[] piecarr;
        } break;
        case M_BO_TA_1:  // 8: bitstring of 32 bits with time tag(cp24time2a)
            break;
        case C_SC_NA_1:  // 45: single command
            break;
        case C_BO_NA_1:  // 51: bitstring of 32 bit command
            break;
        case M_EI_NA_1:  // 70:end of initialization(初始化结束)
            break;

        case C_IC_NA_1: {
            // 100: general interrogation
            // 从站：总召唤确认、发送遥测与遥信数据、总召唤结束
            // 总召唤确认
            generalInterrogationCon();
            // 发送遥测与遥信数据
            sendTelecommunitcating(1, true);    // 遥信
            sendTelemetering(13, false);        // 遥测
            // 总召唤结束
            generalInterrogationEnd();
        }
            break;
        case C_CS_NA_1: {   // 103: clock sync
            struct iec_obj* piecarr = new iec_obj[1];
            uint32_t addr24 = 0;  // 24位信息对象地址

            /**
             * NOTE:被控站初始化后，在时钟同步命令前上传的所有带时标报文其时标中的无效（ invalid）位应
             * 置Ⅰ(即时标无效)，其后置О（即时标有效);若被控站在站对时周期内未收到控制站发出的“时钟同步
             * 命令”，则 invalid位也应置l;
             */
            isClockSYnc = true;

            addr24 = papdu->nsq103.ioa16 + ((uint32_t)papdu->nsq103.ioa8 << 16);
            piecarr[0].address = addr24;
            piecarr[0].type = papdu->head.type;
            piecarr[0].ca = papdu->head.ca;
            piecarr[0].cause = papdu->head.cot;
            piecarr[0].pn = papdu->head.pn;
            piecarr[0].time = papdu->nsq103.obj.time;

            dataIndication(piecarr, 1);
            delete[] piecarr;

            // send confirm message
            sendClockSyncCon();
        }
            break;
        default:
            break;
        }
    }

    // TODO: 修改策略
    if (t2Timeout < 0) {
        t2Timeout = T2;
    } else if (t2Timeout == 0) {
        t2Timeout = -1;
        sendMonitorMessage();
    } else {    // t2Timeout > 0
        --t2Timeout;
    }

    t3Timeout = T3;

    // check parameter `k` and `w`
    if (isSend) {
        ++numMsgReceived;
        if (numMsgReceived == SERVERW) {
            sendMonitorMessage();
            numMsgReceived = 0;
        }
    }
}

void iec_base::showFrame(const char* buf, int size, bool isSend) {
    char buffer[200];

    if (log.isLogging()) {
        memset(buffer, 0, sizeof(buffer));
        if (isSend) {
            sprintf(buffer, "send --> size: (%03d) ", size);
        } else {
            sprintf(buffer, "receive <-- size: (%03d) ", size);
        }
        int cnt = 20, i;
        for (i = 0; i < size && i < cnt; ++i) {
            sprintf(buffer + strlen(buffer), "%02x ", buf[i]);
        }
        if (size > cnt) {
            sprintf(buffer + strlen(buffer), "...");
        }
        log.pushMsg(buffer);
    }
}
