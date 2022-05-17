#include "iec_base.h"
#include <QString>
#include <sstream>

using namespace std;

iec_base::iec_base() {
    this->slavePort = PORT;  // set iec104 tcp port to 2404
    qstrncpy(this->slaveIP, "", 20);
    this->masterAddr = 1;  // 公共地址

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
            sprintf(buf, "--> %03d: ", int(len + 2));
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
    sendStartDtAct();  // 请求建立通信链路(主站->从站)
    this->log.pushMsg("connect success!");
}

void iec_base::onTcpDisconnect() {
    this->isConnected = false;
    this->log.pushMsg("disconnect success!");
    qDebug() << "disconnect success";
}

/**
 * @brief iec_base::sendStartDtAct - send STARTDTACT from master to slave(68 04
 * 07 00 00 00)
 *
 */
void iec_base::sendStartDtAct() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STARTDTACT);
    a.NR = 0;
    send(a);
    // TODO: 是否将log交给上层处理？
    this->log.pushMsg("send STARTDTACT");
}

void iec_base::sendStartDtCon() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STARTDTCON);
    a.NR = 0;
    send(a);
    this->log.pushMsg("send STARTDTCON");
}

void iec_base::sendStopDtAct() {
    struct apdu a;
    a.start = START;
    a.length = 4;
    a.NS = uint16_t(STOPDTACT);
    a.NR = 0;
    send(a);
    this->log.pushMsg("send STOPDTACT");
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
    //    a.head.oa = ...
    a.head.ca = masterAddr;
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
    a.head.cot = ACTTERM;  // 10: 激活终止
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
                log.pushMsg("STARTDTCON");
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
                // case INTERROGATION:
                // 从站收到类型标识100的报文后：
                // 总召唤确认
                generalInterrogationCon();
                // 发送遥测与遥信数据
                // TODO:

                // 总召唤结束
                generalInterrogationEnd();
                break;
            default:
                break;
        }
    }
}
