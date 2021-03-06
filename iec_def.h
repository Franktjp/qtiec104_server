#ifndef IEC_DEF_H
#define IEC_DEF_H

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
    uint8_t hour : 5;  // hours
    uint8_t res2 : 2;
    uint8_t su : 1;     // SU
    uint8_t dmon : 5;   // day of month
    uint8_t dweek : 3;  // day of week
    uint8_t mon : 4;    // months
    uint8_t res3 : 4;
    uint8_t year : 7;  // years
    uint8_t res4 : 1;
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

// ????????????1???M_SP_NA_1 ???????????????????????????
struct iec_type1 {
    uint8_t spi : 1;  // ????????????
    uint8_t res : 3;
    uint8_t bl : 1;  // ????????? / ????????????
    uint8_t sb : 1;  // ????????? / ????????????
    uint8_t nt : 1;  // ????????? / ????????????
    uint8_t iv : 1;  // ?????? / ??????
};

// ????????????2???M_SP_TA_1 ???????????????????????????
struct iec_type2 {
    uint8_t spi : 1;  // ????????????
    uint8_t res : 3;
    uint8_t bl : 1;   // ????????? / ????????????
    uint8_t sb : 1;   // ????????? / ????????????
    uint8_t nt : 1;   // ????????? / ????????????
    uint8_t iv : 1;   // ?????? / ??????
    cp24time2a time;  // ?????????????????????????????????
};

// ????????????7???M_BO_NA_1 32????????????
struct iec_type7 {
    union {
        struct iec_stcd stcd;  // 32????????????
        uint32_t bsi;          // binary status information
    };

    uint8_t ov : 1;  // ?????? / ?????????
    uint8_t res : 3;
    uint8_t bl : 1;  // ????????? / ????????????
    uint8_t sb : 1;  // ????????? / ????????????
    uint8_t nt : 1;  // ????????? / ????????????
    uint8_t iv : 1;  // ?????? / ??????
};

// ????????????8???M_BO_TA_1 ???????????????32????????????
struct iec_type8 {
    union {
        struct iec_stcd stcd;
        uint32_t bsi;  // ?????????????????????
    };

    uint8_t ov : 1;  // ?????? / ?????????
    uint8_t res : 3;
    uint8_t bl : 1;  // ????????? / ????????????
    uint8_t sb : 1;  // ????????? / ????????????
    uint8_t nt : 1;  // ????????? / ????????????
    uint8_t iv : 1;  // ?????? / ??????
    cp24time2a time;
};

// ????????????13???M_ME_NC_1 ?????????, ????????????
struct iec_type13 {
    // ????????????
    float frac; // 4 bytes

    // QDS???????????????
    uint8_t ov : 1;  // ?????? / ?????????
    uint8_t res : 3;
    uint8_t bl : 1;  // ????????? / ????????????
    uint8_t sb : 1;  // ????????? / ????????????
    uint8_t nt : 1;  // ????????? / ????????????
    uint8_t iv : 1;  // ?????? / ??????
};


// ????????????45???C_SC_NA_1 ?????????
struct iec_type45 {
    uint8_t scs : 1;  // ????????????????????? / ??????
    uint8_t res : 1;
    uint8_t qu : 5;  //
    uint8_t se : 1;  // ?????? = 1????????? = 0
};

// ????????????46???C_DC_NA_1  ?????????
struct iec_type46 {
    uint8_t dcs : 2;    // ???????????????
    uint8_t qu : 5;
    uint8_t se : 1;     // ?????? = 1????????? = 0
};

// ????????????47???C_RC_NA_1  ???????????????
struct iec_type47 {
    uint8_t rcs : 2;    // ?????????????????????
    uint8_t qu : 5;
    uint8_t se : 1;     // ?????? = 1????????? = 0
};

// ????????????48???C_SE_NA_1 ????????????, ????????????
struct iec_type48 {
    uint16_t nva;       // ????????????
    uint8_t ql : 7;     // ????????????
    uint8_t se : 1;     // ?????? = 1????????? = 0
};

// ????????????49???C_SE_NB_1 ????????????, ????????????
struct iec_type49 {
    // TODO:
};

// ????????????51???C_BO_NA_1 32?????????
struct iec_type51 {
    union {
        struct iec_stcd stcd;
        uint32_t bsi;  // ?????????????????????
    };
};

// ????????????58???C_SC_TA_1 ?????????CP56Time2a????????????
struct iec_type58 {
    uint8_t scs : 1;  // ????????????????????? / ??????
    uint8_t res : 1;
    uint8_t qu : 5;  //
    uint8_t se : 1;  // ?????? = 1????????? = 0
    struct cp56time2a time;
};

// ????????????100???C_IC_NA_1 ????????????
struct iec_type100 {
    uint8_t qoi : 8;
};

// ????????????103???C_CS_NA_1 ??????????????????
struct iec_type103 {
    struct cp56time2a time;
};

/**
 * @brief The asdu_header struct. The data unit identifier is the header of
 * asdu.
 * TODO: Change the member.
 */
struct asdu_header {
    uint8_t type : 8;  // ????????????(type)???1??????
    // ?????????????????????(VSQ)???1??????
    uint8_t num : 7;  // ???????????????(the number of information objects)
    uint8_t sq : 1;  // ??????????????????(sequence)????????? = 1???????????? = 0
    // ????????????(COT)???2??????
    uint8_t cot : 6;  // ????????????(cause of transmission)
    uint8_t pn : 1;  // ???????????? = 0??????????????? = 1(positive/negative confirm)
    uint8_t t : 1;  // ????????? = 0????????? = 1(test)
    uint8_t oa;     // ????????????(originator address)???1??????
    // ????????????
    uint16_t ca;  // ASDU????????????(asdu common address)???2??????
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
        } nsq1[1];

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
            struct iec_type13 obj[1];
        } sq13;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type13 obj;
        } nsq13[1];

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type45 obj;
        } nsq45;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type46 obj;
        } nsq46;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type47 obj;
        } nsq47;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type48 obj;
        } nsq48;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type51 obj;
        } nsq51;

        struct {
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type58 obj;
        } nsq58;

        struct {  // ??????????????????
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type100 obj;
        } nsq100;

        struct {    // ??????????????????
            uint16_t ioa16;
            uint8_t ioa8;
            struct iec_type103 obj;
        } nsq103;

        unsigned char data[255];  // TODO:
    };
};

#pragma pack(pop)
#endif  // IEC_DEF_H
