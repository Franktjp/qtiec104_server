#ifndef IEC_BASE_H
#define IEC_BASE_H

#include "iec_def.h"
#include "logmsg.h"

#pragma pack(push)
#pragma pack(1)

struct iec_obj {
    uint32_t address;  // 3字节地址(4字节空间)
    cp24time2a time;   // 3字节时标

    float value;  // value 4 bytes float

    // asdu header
    uint8_t type : 8;
    // uint8_t num : 7;	// 信息体数目
    // uint8_t sq : 1;		// 连续 = 1，不连续 = 0
    uint8_t cause : 6;  // 传送原因
    // uint8_t pn : 1;		// 肯定确认 = 0，否定确认 = 1
    // uint8_t t : 1;		// 未实验 = 0，实验 = 1
    // uint8_t oa;			// 源发地址：1字节
    uint16_t ca : 16;  // common of ASDU
    // 6 bytes

    uint8_t spi : 1;  // 单点信息
    uint8_t bl : 1;   // 被闭锁 / 未被闭锁
    uint8_t sb : 1;   // 被取代 / 未被取代
    uint8_t nt : 1;   // 当前值 / 非当前值
    uint8_t iv : 1;   // 有效 / 无效
    uint8_t ov : 1;   // 溢出 / 未溢出
    uint8_t scs : 1;  // 单命令状态（开 / 合）
    uint8_t se : 1;   // 选择 = 1，执行 = 0
    // 8 bits

    uint8_t qu : 5;
    // 5bits

    // others
    uint8_t pn : 1;  // P/N位 P=0表示肯定确认，P=1表示否定确认

    uint32_t bsi;  // 二进制状态信息
};

#pragma pack(pop)

class iec_base {
public:
    // 传送原因 cause of transmition (standard)
    static const uint32_t UNUSED = 0;        // 没有使用
    static const uint32_t CYCLIC = 1;        // 周期、循环
    static const uint32_t BGSCAN = 2;        // 背景扫描
    static const uint32_t SPONTANEOUS = 3;   // 突发(自发)
    static const uint32_t INITIALIZED = 4;   // 初始化
    static const uint32_t REQUEST = 5;       // 请求或者被请求
    static const uint32_t ACTIVATION = 6;    // 激活
    static const uint32_t ACTCONFIRM = 7;    // 激活确认
    static const uint32_t DEACTIVATION = 8;  // 停止激活
    static const uint32_t DEACTCONFIRM = 9;  // 停止激活确认
    static const uint32_t ACTTERM = 10;      // 激活终止
    static const uint32_t RETREM = 11;    // 远方命令引起的返送信息
    static const uint32_t RETLOC = 12;    // 当地命令引起的返送信息
    static const uint32_t FILE = 13;      // 文件传输
    static const uint32_t INTROGEN = 20;  // 响应站召唤

    // 报文类型
    static const uint32_t INTERROGATION = 0x64; // 100 总召唤
    // 起动字符
    static const uint32_t START = 0x68;
    static const uint32_t RESET = 0x69;

    static const uint32_t POSITIVE = 0;
    static const uint32_t NEGATIVE = 1;

    static const uint32_t SELECT = 1;   // 选择 / 执行
    static const uint32_t EXECUTE = 0;

    // U格式
    static const uint32_t SUPERVISORY = 0x01;  // S帧
    static const uint32_t STARTDTACT = 0x07;
    static const uint32_t STARTDTCON = 0x0B;
    static const uint32_t STOPDTACT = 0x13;
    static const uint32_t STOPDTCON = 0x23;
    static const uint32_t TESTFRACT = 0x43;
    static const uint32_t TESTFRCON = 0x83;

    // I格式的类型标识
    static const uint32_t M_SP_NA_1 = 1;  // single-point information
    static const uint32_t M_SP_TA_1 =
            2;  // single-point information with time tag(cp24time2a)
    static const uint32_t M_DP_NA_1 = 3;  // double-point information
    static const uint32_t M_ST_NA_1 = 5;  // step position information
    static const uint32_t M_BO_NA_1 = 7;  // bitstring of 32 bits
    static const uint32_t M_BO_TA_1 =
            8;  // bitstring of 32 bits with time tag(cp24time2a)
    static const uint32_t M_ME_NA_1 = 9;   // normalized value
    static const uint32_t M_ME_NB_1 = 11;  // scaled value
    static const uint32_t M_ME_NC_1 = 13;  // floating point
    static const uint32_t M_IT_NA_1 = 15;  // integrated totals
    static const uint32_t M_PS_NA_1 =
            20;  // Packed single point information with status change detection
    static const uint32_t M_SP_TB_1 =
            30;  // single-point information with time tag(cp56time2a)
    static const uint32_t M_DP_TB_1 =
            31;  // double-point information with time tag
    static const uint32_t M_ST_TB_1 =
            32;  // step position information with time tag
    static const uint32_t M_BO_TB_1 = 33;  // bitstring of 32 bits with time tag
    static const uint32_t M_ME_TD_1 = 34;  // normalized value with time tag
    static const uint32_t M_ME_TE_1 = 35;  // scaled value with time tag
    static const uint32_t M_ME_TF_1 = 36;  // floating point with time tag
    static const uint32_t M_IT_TB_1 = 37;  // integrated totals with time tag
    static const uint32_t M_EP_TD_1 =
            38;  // Event of protection equipment with CP56Time2a time tag
    static const uint32_t M_EP_TE_1 = 39;  // Packed start events of protection
    // equipment with CP56Time2a time tag
    static const uint32_t M_EP_TF_1 =
            40;  // Packed output circuit information of protection equipment with
    // CP56Time2a time tag
    static const uint32_t C_SC_NA_1 = 45;  // single command
    static const uint32_t C_DC_NA_1 = 46;  // double command
    static const uint32_t C_RC_NA_1 = 47;  // regulating step command
    static const uint32_t C_SE_NA_1 = 48;  // set-point normalised command
    static const uint32_t C_SE_NB_1 = 49;  // set-point scaled command
    static const uint32_t C_SE_NC_1 =
            50;  // set-point short floating point command
    static const uint32_t C_BO_NA_1 = 51;  // Bitstring of 32 bit command
    static const uint32_t C_SC_TA_1 = 58;  // single command with time tag
    static const uint32_t C_DC_TA_1 = 59;  // double command with time tag
    static const uint32_t C_RC_TA_1 =
            60;  // regulating step command with time tag
    static const uint32_t C_SE_TA_1 =
            61;  // set-point normalised command with time tag
    static const uint32_t C_SE_TB_1 =
            62;  // set-point scaled command with time tag
    static const uint32_t C_SE_TC_1 =
            63;  // set-point short floating point command with time tag
    static const uint32_t C_BO_TA_1 =
            64;  // Bitstring of 32 bit command with time tag
    static const uint32_t M_EI_NA_1 = 70;   // end of initialization
    static const uint32_t C_IC_NA_1 = 100;  // general interrogation (GI)
    static const uint32_t C_CI_NA_1 = 101;  // counter interrogation
    static const uint32_t C_RD_NA_1 = 102;  // read command
    static const uint32_t C_CS_NA_1 = 103;  // clock synchronization command
    static const uint32_t C_RP_NA_1 = 105;  // reset process command
    static const uint32_t C_TS_TA_1 =
            107;  // test command with time tag CP56Time2a
    static const uint32_t P_ME_NA_1 =
            110;  // Parameter of measured values, normalized value
    static const uint32_t P_ME_NB_1 =
            111;  // Parameter of measured values, scaled value
    static const uint32_t P_ME_NC_1 =
            112;  // Parameter of measured values, short floating point number
    static const uint32_t P_AC_NA_1 = 113;  // Parameter activation

    // TCP连接使用固定端口号
    static const uint32_t SERVERPORT = 2404;

private:
    uint32_t slavePort;  // tcp port of slave, defaults to 9090
    char slaveIP[20];    // slave ip address

    //TODO: ca and oa
    // 对于从站来说，公共地址ca应该是masteraddr oa应该是slaveaddr
    uint16_t masterAddr;    // master link address(originator address, oa)
    uint16_t slaveAddr;     // slave link address(common address of ASDU, ca)

    bool isConnected;  // tcp or udp connect status: true->connected, false->not connected

    // TODO:
    uint16_t vs;  // 发送包数量
    uint16_t vr;  // 接受包数量

public:
    Logger log;


public:
    // functions
    iec_base();
    // parse APDU
    void parse(struct apdu* papdu, int sz);
    void send(const struct apdu& wapdu);
    void packetReadyTCP();
    void showFrame(const char* buf, int size, bool isSend);


    uint32_t getSlavePort();
    void setSlavePort(uint32_t port);
    char* getSlaveIP();
    void setSlaveIP(const char* ip);
    uint16_t getSlaveAddr();
    void setSlaveAddr(uint16_t addr);


    // 回调函数和事件处理函数通常以on开头
    void onTcpConnect();
    void onTcpDisconnect();

protected:
    //
    virtual int readTCP(char* buf, int size) = 0;  // 返回0失败，返回一个正数成功
    virtual void sendTCP(const char* buf, int size) = 0;

    // TODO: 是的，显示数据
    virtual void dataIndication(struct iec_obj* /* obj */,
                                unsigned int /* numpoints */) = 0;

    // tcp connect
    virtual void tcpConnect() = 0;
    virtual void tcpDisconnect() = 0;
    // TODO: udp connect
    virtual void udpConnect() = 0;
    virtual void udpDisconnect() = 0;

    // return the bumber of bytes that are available for reading
    virtual int bytesAvailable() = 0;
    // wait until bytes data is ready or msecs milliseconds have passed
    virtual void waitForReadyRead(int bytes, int msecs) = 0;

private:
    void sendStartDtCon();  // 确认建立通信链路（从站->主站）
    void sendStopDtAct();   // 请求停止通信链路（主站->从站）
    void sendStopDtCon();   // 确认停止通信链路（从站->主站）
    void sendTestfrAct();   // 测试通信链路
    void sendTestfrCon();   // 测试通信链路确认
    void generalInterrogationCon();  // 总召唤确认
    void generalInterrogationEnd();  // 总召唤结束
    void sendTelemetering(uint8_t type, bool sq);        // 模拟并发送遥测数据
    void sendTelecommunitcating(uint8_t type, bool sq);  // 模拟并发送遥信数据


};

#endif  // IEC_BASE_H
