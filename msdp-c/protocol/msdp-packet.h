#ifndef PI_UPNP_MSDP_PACKET_H
#define PI_UPNP_MSDP_PACKET_H

#define MSDP_PKT_CALLBACK  0b00000101
#define MSDP_PKT_VIRTUALN  0b00001000
#define MSDP_PKT_RSVD_ACT  0b00000010
#define MSDP_PKT_SERVER    0b00000001
#define MSDP_PKT_CLIENT    0b00000010
#define MSDP_PKT_DAEMON    0b00000101

#define MSDP_PKT_RSVD_DEFINED_LEN 12
#define MSDP_PKT_UID_DEFINED_LEN  10
#define MSDP_PKT_DATA_DEFINED_LEN 224

#define PKT_IS_CODE(x) (((x) & 0b00011111) != 0)

#define MSDP_PKT_IS_CALLBACK(x)         MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_CALLBACK)
#define MSDP_PKT_IS_VIRTUALN(x)         MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_VIRTUALN)
#define MSDP_PKT_IS_RSVD_ACTIVE(x)      MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_RSVD_ACT)
#define MSDP_PKT_SYS_IS_SERVER(x)       MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_SERVER)
#define MSDP_PKT_SYS_IS_CLIENT(x)       MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_CLIENT)
#define MSDP_PKT_SYS_IS_DAEMON(x)       MSDP_PKT_FLAG_HELPER(x, MSDP_PKT_DAEMON)

#ifdef _WIN32
  #define MSDP_SYSTEM 1
#elif __MACH__
  #define MSDP_SYSTEM 2
#elif __unix__
  #define MSDP_SYSTEM 3
#elif BSD
  #define MSDP_SYSTEM 4
#else
  #define MSDP_SYSTEM 0
#endif

#ifndef __msdp__
  #define __msdp__ 0b00000001
#endif

#define MSDP_NET_DEFAULT_ID 0
#define MSDP_NET_TEST_ID    1

/**
 * The default packet-byte type defined as an unsigned char.
 */
typedef unsigned char uint8_t;

typedef char fr_byte_t;

#define MSDP_PKT_TO_UINT8(x) (uint8_t) (x)

#define MSDP_PKT_FLAG_HELPER(x, y) (((x) & (y)) != 0)
#define MSDP_PKT_FLAG_ADD(x, y) x |= (y)

/**
 * /brief
 * The packet-code stored in an enum type definition. The values are
 * defined as an uint8_t.
 */
typedef enum {
    OP_INI = 0b00000001,
    OP_ERR = 0b00000010,
    OP_QRY = 0b00000100,
    OP_RSP = 0b00001000,
    OP_LVE = 0b00010000
} fr_msdp_code_e;

typedef struct fr_eth_node {
    char *name;

    uint8_t ui8_addr[4];
} fr_eth_ip_addr;

typedef struct fr_msdp_container {

    /**
     * /brief
     * The sender address split into uint8 values and the string representation.
     */
    fr_eth_ip_addr from;

    /**
     * /brief
     * The code shows an interpreter how to handle the incoming packet. For instance,
     * a packet named with OP_ERR should be logged into somewhere.
     */
    fr_msdp_code_e code;

    /**
     * /brief
     * Network flags contain three options for now. The virtual network flag, a
     * callback flag and a bit that is set if all reserved bytes should be used.
     */
    uint8_t network_flags;

    /**
     * /brief
     * In order to identify the current network an id is transferred. It could be
     * used by the receiver to switch networks or to set a flag for a test network.
     */
    uint8_t net_id;

    /**
     * /brief
     * A placeholder for data/ options of future releases. Any data of new releases
     * should be placed here.
     */
    fr_byte_t reserved_bytes[MSDP_PKT_RSVD_DEFINED_LEN];

    /**
     * /brief
     * Defines the current working os.
     */
    uint8_t system;

    /**
     * /brief
     * The current version of this protocol.
     */
    uint8_t version;

    /**
     * /brief
     * Contains bits that indicate a different type of client behind the sender.
     */
    uint8_t client_type;

    /**
     * /brief
     * A small unified identifier to identify packets from different sources.
     */
    fr_byte_t uid[MSDP_PKT_UID_DEFINED_LEN];

    /**
     * /brief
     * The length of data. This number has to be between 0 and 224
     */
    uint8_t data_len;

    /**
     * /brief
     * The raw transmitted data-section.
     */
    fr_byte_t data[MSDP_PKT_DATA_DEFINED_LEN];

} fr_msdp_packet_t;


#endif //PI_UPNP_MSDP_PACKET_H
