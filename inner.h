#ifndef INNER_H
#define INNER_H

#include <cstdint>

#pragma pack(push)
#pragma pack(1)

/**
 * @brief The cp56time2a time scale struct
 */
struct cp56time2a {
    uint16_t mesc;    // milliseconds
    uint8_t min : 6;  // minutes
    uint8_t res1 : 1;
    uint8_t iv : 1;    // IV
    uint8_t hour : 6;  // hours
    uint8_t res2 : 1;
    uint8_t su : 1;     // SU
    uint8_t dmon : 5;   // day of month
    uint8_t dweek : 3;  // day of week
    uint8_t mon : 4;    // months
    uint8_t res3 : 4;
    uint8_t year : 7;  // years
    uint8_t res : 1;
};

/**
 * @brief The cp24time2a time sacle struct
 * NOTE: The cp24time2a abandons the 4th to 7th bytes of cp56time2a. So the size
 * of cp24time2a is 3 bytes.
 */
struct cp24time2a {
    uint16_t mesc;    // milliseconds
    uint8_t min : 6;  // minutes
    uint8_t res1 : 1;
    uint8_t iv : 1;  // IV
};

/**
 * @brief The 32-bit string state and change data unit
 * TODO:
 */
struct iec_stcd {
    union {
        uint16_t st;
        struct {
            uint8_t st1 : 1;
            uint8_t st2 : 1;
            uint8_t st3 : 1;
            uint8_t st4 : 1;
            uint8_t st5 : 1;
            uint8_t st6 : 1;
            uint8_t st7 : 1;
            uint8_t st8 : 1;
            uint8_t st9 : 1;
            uint8_t st10 : 1;
            uint8_t st11 : 1;
            uint8_t st12 : 1;
            uint8_t st13 : 1;
            uint8_t st14 : 1;
            uint8_t st15 : 1;
            uint8_t st16 : 1;
        };
    };

    union {
        uint16_t cd;
        struct {
            uint8_t cd1 : 1;
            uint8_t cd2 : 1;
            uint8_t cd3 : 1;
            uint8_t cd4 : 1;
            uint8_t cd5 : 1;
            uint8_t cd6 : 1;
            uint8_t cd7 : 1;
            uint8_t cd8 : 1;
            uint8_t cd9 : 1;
            uint8_t cd10 : 1;
            uint8_t cd11 : 1;
            uint8_t cd12 : 1;
            uint8_t cd13 : 1;
            uint8_t cd14 : 1;
            uint8_t cd15 : 1;
            uint8_t cd16 : 1;
        };
    };
};

/**
 * ASDU(application service data unit) defines.
 */

// 类型标识1：M_SP_NA_1 不带时标的单点信息
struct iec_type1 {
    uint8_t spi : 1;  // 单点信息
    uint8_t res : 3;
    uint8_t bl : 1;  // 被闭锁 / 未被闭锁
    uint8_t sb : 1;  // 被取代 / 未被取代
    uint8_t nt : 1;  // 当前值 / 非当前值
    uint8_t iv : 1;  // 有效 / 无效
};

// 类型标识2：M_SP_TA_1 带短时标的单点信息
struct iec_type2 {
    uint8_t spi : 1;  // 单点信息
    uint8_t res : 3;
    uint8_t bl : 1;   // 被闭锁 / 未被闭锁
    uint8_t sb : 1;   // 被取代 / 未被取代
    uint8_t nt : 1;   // 当前值 / 非当前值
    uint8_t iv : 1;   // 有效 / 无效
    cp24time2a time;  // 三个八位位组二进制时间
};

// 类型标识7：M_BO_NA_1 32位比特串
struct iec_type7 {
    union {
        struct iec_stcd stcd;  // 32位比特串
        uint32_t bsi;          // binary status information
    };

    uint8_t ov : 1;  // 溢出 / 未溢出
    uint8_t res : 3;
    uint8_t bl : 1;  // 被闭锁 / 未被闭锁
    uint8_t sb : 1;  // 被取代 / 未被取代
    uint8_t nt : 1;  // 当前值 / 非当前值
    uint8_t iv : 1;  // 有效 / 无效
};

// 类型标识8：M_BO_TA_1 带短时标的32位比特串
struct iec_type8 {
    union {
        struct iec_stcd stcd;
        uint32_t bsi;  // 二进制状态信息
    };

    uint8_t ov : 1;  // 溢出 / 未溢出
    uint8_t res : 3;
    uint8_t bl : 1;  // 被闭锁 / 未被闭锁
    uint8_t sb : 1;  // 被取代 / 未被取代
    uint8_t nt : 1;  // 当前值 / 非当前值
    uint8_t iv : 1;  // 有效 / 无效
    cp24time2a time;
};

// 类型标识45：C_SC_NA_1 单命令
struct iec_type45 {
    uint8_t scs : 1;  // 单命令状态（开 / 合）
    uint8_t res : 1;
    uint8_t qu : 5;  //
    uint8_t se : 1;  // 选择 = 1，执行 = 0
};

// 类型标识51：C_BO_NA_1 32比特串
struct iec_type51 {
    union {
        struct iec_stcd stcd;
        uint32_t bsi;  // 二进制状态信息
    };
};

// 类型标识100：C_IC_NA_1 召唤命令
struct iec_type100 {
    uint8_t qoi : 8;
};

/**
 * @brief The asdu_header struct. The data unit identifier is the header of
 * asdu.
 * TODO: Change the member.
 */
struct asdu_header {
    uint8_t type : 8;  // 标识类型(type)：1字节
    // 可变结构限定词(VSQ)：1字节
    uint8_t num : 7;  // 信息体数目(the number of information objects)
    uint8_t sq : 1;  // 地址的连续性(sequence)：连续 = 1，不连续 = 0
    // 传送原因(COT)：2字节
    uint8_t cot : 6;  // 传送原因(cause of transmission)
    uint8_t pn : 1;  // 肯定确认 = 0，否定确认 = 1(positive/negative confirm)
    uint8_t t : 1;  // 未实验 = 0，实验 = 1(test)
    uint8_t oa;     // 源发地址(originator address)：1字节
    // 公共地址
    uint16_t ca;  // ASDU公共地址(asdu common address)：2字节
};

/**
 * @brief The apdu struct.
 */
struct apdu {
    // ---------------- APCI begin ------------------------
    uint8_t start;   // The start character
    uint8_t length;  // The length of APDU
    uint16_t NS;     // Control domain(send number)
    uint16_t NR;     //    . . .      (receive number)
    // ---------------- APCI end --------------------------

    // ---------------- ASDU begin ------------------------
    struct asdu_header head;  // ASDU header
    union {
        /**
         * @brief type1: sequence
         */
        struct {
            /**
             * @brief ioa - information object address. We think the default
             * size of ioa is 3 bytes.
             */
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type1 obj[1];
        } sq1;

        /**
         * @brief type1: non sequence
         */
        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type1 obj;
        } nsq1;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type2 obj[1];
        } sq2;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type7 obj[1];
        } sq7;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type7 obj;
        } nsq7[1];

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type8 obj[1];
        } sq8;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type45 obj;
        } sq45;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type51 obj;
        } nsq51;

        struct {  // 单个信息对象
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type100 obj;
        } nsq100;

        unsigned char data[255];  // TODO:
    };
};

#pragma pack(pop)
#endif  // INNER_H
