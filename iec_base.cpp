#include "iec_base.h"
#include <QString>
#include <sstream>

using namespace std;

iec_base::iec_base() {
    this->slavePort = SERVERPORT;  // set iec104 tcp port to 2404
    qstrncpy(this->slaveIP, "", 20);
    this->masterAddr = 0;   // originator address
    this->slaveAddr = 0;    // common address os ASDU
    this->vs = this->vr = 0;
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
    qstrncpy(this->slaveIP, ip, 20);
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
        parse(&a, a.length + 2);
        break;
    }
}

void iec_base::send(const struct apdu& wapdu) {
    this->sendTCP(reinterpret_cast<const char*>(&wapdu), wapdu.length + 2);
}

void iec_base::onTcpConnect() {
    this->vr = this->vs = 0;
    this->isConnected = true;
    this->log.pushMsg("connect success!");
}

void iec_base::onTcpDisconnect() {
    this->isConnected = false;
    this->log.pushMsg("disconnect success!");
    qDebug() << "disconnect success";
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
    a.start = START;
    a.length = 0x0E;
    a.NS = vs;
    a.NR = vr;
    a.head.type = INTERROGATION;  // TODO: 为何是INTERROGATION
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

    send(a);
    vs += 2;
}

void iec_base::generalInterrogationEnd() {
    struct apdu a;
    a.start = START;
    a.length = 0x0E;
    a.NS = vs;
    a.NR = vr;
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

    send(a);
    vs += 2;
}

/**
 * @brief iec_base::sendTelemetering - 模拟并发送遥测数据
 * @param type  - 类型标识
 * @param sq    - 是否连续值
 */
void iec_base::sendTelemetering(uint8_t type, bool sq) {
    char buf[100];
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
        papdu->NS = vs;
        papdu->NR = vr;
        papdu->head.type = type;
        papdu->head.num = 2;
        papdu->head.sq = sq ? 1 : 0;
        papdu->head.cot = INTROGEN;
        papdu->head.oa = slaveAddr;
        papdu->head.ca = masterAddr; // ASDU公共地址
    }
        break;
    default:
        sprintf(buf, "ERROR: type %d is not implemented!", uint32_t(type));
        log.pushMsg(buf);
        return;
        break;
    }

    send(*papdu);
    vs += 2;

    delete[] papdu;
}

/**
 * @brief iec_base::sendTelecommunitcating - 模拟并发送遥信数据
 * TODO: Add more types.
 * @param type  - 类型标识
 * @param sq    - 是否连续值
 */
void iec_base::sendTelecommunitcating(uint8_t type, bool sq) {
    char buf[100];
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
        papdu->NS = vs;
        papdu->NR = vr;
        papdu->head.type = type;
        papdu->head.num = 2;
        papdu->head.sq = sq ? 1 : 0;
        papdu->head.cot = INTROGEN;
        papdu->head.oa = slaveAddr;
        papdu->head.ca = masterAddr; // ASDU公共地址
    }
        break;

    default:
        sprintf(buf, "ERROR: type %d is not implemented!", uint32_t(type));
        log.pushMsg(buf);
        return;
        break;
    }

    send(*papdu);
    vs += 2;

    delete[] papdu;
}

void iec_base::parse(struct apdu* papdu, int size) {
    struct apdu wapdu;  // 缓冲区组装发送apdu
    uint16_t vr_new;    // TODO
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
        case STARTDTCON:
            // TODO:
            log.pushMsg("receice STARTDTCON");
            break;
        case STOPDTACT: {  // 停止数据传送命令
            log.pushMsg("receive STOPDTACT");
            sendStopDtCon();
        } break;
        case STOPDTCON:  // 停止数据传送命令确认
            // TODO:
            break;
        case TESTFRACT: {
            // 链路测试命令
            log.pushMsg("   TESTFRACT");
            sendTestfrCon();
        } break;
        case TESTFRCON:  // 链路测试命令确认
            break;
        default:
            log.pushMsg("ERROR: unknown control message");
            break;
        }
    } else {
        // TODO:
        // code...

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
        case M_SP_TA_1:  // 2: single-point information with time
            // tag(cp24time2a)
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
        case M_BO_TA_1:  // 8: bitstring of 32 bits with time
            // tag(cp24time2a)
            break;
        case C_SC_NA_1:  // 45: single command
            break;
        case C_BO_NA_1:  // 51: bitstring of 32 bit command
            break;
        case M_EI_NA_1:  // 70:end of initialization(初始化结束)
            break;

        case C_IC_NA_1:  // 100: general interrogation
            // 从站：总召唤确认、发送遥测与遥信数据、总召唤结束
            // 总召唤确认
            generalInterrogationCon();
            // 发送遥测与遥信数据
            sendTelecommunitcating(1, true);    // 遥信
            sendTelemetering(13, false);        // 遥测
            // 总召唤结束
            generalInterrogationEnd();
            break;
        default:
            break;
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
