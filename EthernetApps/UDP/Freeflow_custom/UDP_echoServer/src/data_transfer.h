/*
 * Data Transfer Application Header
 * Real data communication between Zynq and Qt widget application
 */

#ifndef __DATA_TRANSFER_H_
#define __DATA_TRANSFER_H_

#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "xil_printf.h"
#include "platform.h"

/* Data transfer configuration */
#define DATA_TRANSFER_PORT 8888
#define MAX_DATA_SIZE 1024
#define SEND_INTERVAL_MS 1000  // Send data every 1 second

/* Message types for protocol */
typedef enum {
    MSG_TYPE_DATA = 0x01,
    MSG_TYPE_COMMAND = 0x02,
    MSG_TYPE_RESPONSE = 0x03,
    MSG_TYPE_HEARTBEAT = 0x04
} msg_type_t;

/* Data structure for messages */
typedef struct {
    u8_t msg_type;
    u8_t sequence;
    u16_t length;
    u8_t data[MAX_DATA_SIZE - 4]; // Total size minus header
} data_message_t;

/* Statistics structure */
typedef struct {
    u32_t packets_sent;
    u32_t packets_received;
    u32_t bytes_sent;
    u32_t bytes_received;
    u32_t last_sequence_received;
    u32_t last_packet_time;
    u32_t connection_timeout_counter;
} transfer_stats_t;

/* Function prototypes */
void print_app_header(void);
int start_application(void);
int transfer_data(void);

/* Data transfer functions */
void init_data_transfer(void);
void send_data_to_qt(void);
void process_received_data(struct pbuf *p, const ip_addr_t *addr, u16_t port);
void display_statistics(void);
void send_heartbeat(void);

/* External variables */
extern transfer_stats_t stats;
extern struct udp_pcb *data_pcb;
extern ip_addr_t qt_client_ip;
extern u16_t qt_client_port;

#endif /* __DATA_TRANSFER_H_ */
