/*
 * Data Transfer Application Implementation
 * Real data communication between Zynq and Qt widget application
 */

#include "data_transfer.h"
#include <string.h>

/* Global variables */
struct udp_pcb *data_pcb;
transfer_stats_t stats;
ip_addr_t qt_client_ip;
u16_t qt_client_port = 0;
u8_t sequence_counter = 0;
// u32_t last_send_time = 0; // Removed - using counter instead

/* Sample data to send */
static const char *sample_data[] = {"Hello from Zynq!", "Temperature: 45.2°C",
                                    "Voltage: 3.3V",    "Status: Running",
                                    "Memory: 85% used", "Network: Connected",
                                    "CPU Load: 67%",    "System Time: Active"};
static u8_t sample_data_index = 0;

void print_app_header(void) {
  xil_printf("\r\n=== Data Transfer Application ===\r\n");
  xil_printf("UDP server listening on port %d\r\n", DATA_TRANSFER_PORT);
  xil_printf("Waiting for Qt client connection...\r\n");
  xil_printf("Send data every %d ms\r\n", SEND_INTERVAL_MS);
  xil_printf("=====================================\r\n\r\n");
}

static void reset_statistics(void) {
  stats.packets_sent = 0;
  stats.packets_received = 0;
  stats.bytes_sent = 0;
  stats.bytes_received = 0;
  stats.last_sequence_received = 0;
  stats.last_packet_time = 0;
  stats.connection_timeout_counter = 0;
}

void init_data_transfer(void) {
  reset_statistics();
  IP4_ADDR(&qt_client_ip, 0, 0, 0, 0); // Will be set when first packet received
  qt_client_port = 0;
  sequence_counter = 0;
  // last_send_time = 0; // Removed
}

static void udp_data_recv(void *arg, struct udp_pcb *tpcb, struct pbuf *p,
                          const ip_addr_t *addr, u16_t port) {
  if (p != NULL) {
    // Check if this is a new client or existing client
    if (qt_client_port == 0) {
      // First client connection
      qt_client_ip = *addr;
      qt_client_port = port;
      xil_printf("[INFO] Qt client connected: %s:%d\r\n", inet_ntoa(*addr),
                 port);
    } else if (!ip_addr_cmp(&qt_client_ip, addr) || qt_client_port != port) {
      // New client connection (different IP or port)
      xil_printf("[INFO] New Qt client connected: %s:%d (was %s:%d)\r\n",
                 inet_ntoa(*addr), port, inet_ntoa(qt_client_ip),
                 qt_client_port);
      qt_client_ip = *addr;
      qt_client_port = port;
    }

    process_received_data(p, addr, port);
    pbuf_free(p);
  }
}

void process_received_data(struct pbuf *p, const ip_addr_t *addr, u16_t port) {
  if (p->tot_len < sizeof(data_message_t)) {
    xil_printf("[ERROR] Received packet too small: %d bytes\r\n", p->tot_len);
    return;
  }

  data_message_t *msg = (data_message_t *)p->payload;

  // Update statistics
  stats.packets_received++;
  stats.bytes_received += p->tot_len;
  stats.last_packet_time = 0; // Simplified for now
  stats.connection_timeout_counter =
      0; // Reset timeout counter on received data

  // Display received data on UART terminal
  xil_printf("\r\n[UART] Received from %s:%d\r\n", inet_ntoa(*addr), port);
  xil_printf("[UART] Type: %d, Seq: %d, Len: %d\r\n", msg->msg_type,
             msg->sequence, msg->length);

  if (msg->length > 0 && msg->length <= sizeof(msg->data)) {
    xil_printf("[UART] Data: ");
    for (u16_t i = 0; i < msg->length; i++) {
      xil_printf("%c", msg->data[i]);
    }
    xil_printf("\r\n");
  }

  // Handle different message types
  switch (msg->msg_type) {
  case MSG_TYPE_DATA:
    xil_printf("[UART] Data message received\r\n");
    break;
  case MSG_TYPE_COMMAND:
    xil_printf("[UART] Command message received\r\n");
    break;
  case MSG_TYPE_RESPONSE:
    xil_printf("[UART] Response message received\r\n");
    break;
  case MSG_TYPE_HEARTBEAT:
    xil_printf("[UART] Heartbeat received\r\n");
    break;
  default:
    xil_printf("[UART] Unknown message type: %d\r\n", msg->msg_type);
    break;
  }

  // Send acknowledgment if needed
  if (msg->msg_type == MSG_TYPE_COMMAND) {
    data_message_t ack_msg;
    ack_msg.msg_type = MSG_TYPE_RESPONSE;
    ack_msg.sequence = msg->sequence;
    ack_msg.length = snprintf((char *)ack_msg.data, sizeof(ack_msg.data),
                              "Command %d processed", msg->sequence);

    struct pbuf *ack_pbuf =
        pbuf_alloc(PBUF_TRANSPORT, sizeof(data_message_t), PBUF_RAM);
    if (ack_pbuf != NULL) {
      memcpy(ack_pbuf->payload, &ack_msg, sizeof(data_message_t));
      udp_sendto(data_pcb, ack_pbuf, addr, port);
      pbuf_free(ack_pbuf);

      stats.packets_sent++;
      stats.bytes_sent += sizeof(data_message_t);
      xil_printf("[UART] Sent acknowledgment\r\n");
    }
  }
}

void send_data_to_qt(void) {
  if (qt_client_port == 0) {
    return; // No client connected
  }

  data_message_t msg;
  msg.msg_type = MSG_TYPE_DATA;
  msg.sequence = sequence_counter++;

  // Prepare sample data
  const char *data_str = sample_data[sample_data_index];
  sample_data_index =
      (sample_data_index + 1) % (sizeof(sample_data) / sizeof(sample_data[0]));

  msg.length = snprintf((char *)msg.data, sizeof(msg.data), "%s [Seq:%d]",
                        data_str, msg.sequence);

  // Create packet buffer
  struct pbuf *pbuf =
      pbuf_alloc(PBUF_TRANSPORT, sizeof(data_message_t), PBUF_RAM);
  if (pbuf == NULL) {
    xil_printf("[ERROR] Failed to allocate pbuf for sending\r\n");
    return;
  }

  // Copy message to buffer
  memcpy(pbuf->payload, &msg, sizeof(data_message_t));

  // Send packet
  err_t err = udp_sendto(data_pcb, pbuf, &qt_client_ip, qt_client_port);
  if (err == ERR_OK) {
    stats.packets_sent++;
    stats.bytes_sent += sizeof(data_message_t);
    xil_printf("[SENT] Data to Qt: %s\r\n", data_str);
  } else {
    xil_printf("[ERROR] Failed to send data: %d\r\n", err);
  }

  pbuf_free(pbuf);
}

void send_heartbeat(void) {
  if (qt_client_port == 0) {
    return; // No client connected
  }

  data_message_t msg;
  msg.msg_type = MSG_TYPE_HEARTBEAT;
  msg.sequence = sequence_counter++;
  msg.length = snprintf((char *)msg.data, sizeof(msg.data), "Heartbeat %d",
                        msg.sequence);

  struct pbuf *pbuf =
      pbuf_alloc(PBUF_TRANSPORT, sizeof(data_message_t), PBUF_RAM);
  if (pbuf != NULL) {
    memcpy(pbuf->payload, &msg, sizeof(data_message_t));
    udp_sendto(data_pcb, pbuf, &qt_client_ip, qt_client_port);
    pbuf_free(pbuf);

    stats.packets_sent++;
    stats.bytes_sent += sizeof(data_message_t);
  }
}

void display_statistics(void) {
  xil_printf("\r\n=== Transfer Statistics ===\r\n");
  xil_printf("Packets sent: %d\r\n", stats.packets_sent);
  xil_printf("Packets received: %d\r\n", stats.packets_received);
  xil_printf("Bytes sent: %d\r\n", stats.bytes_sent);
  xil_printf("Bytes received: %d\r\n", stats.bytes_received);
  xil_printf("Client: %s:%d\r\n",
             qt_client_port ? inet_ntoa(qt_client_ip) : "None", qt_client_port);
  xil_printf("Timeout counter: %d\r\n", stats.connection_timeout_counter);
  xil_printf("=============================\r\n\r\n");
}

int start_application(void) {
  err_t err;

  /* Initialize data transfer */
  init_data_transfer();

  /* Create UDP PCB */
  data_pcb = udp_new();
  if (!data_pcb) {
    xil_printf("[ERROR] Failed to create UDP PCB. Out of Memory\r\n");
    return -1;
  }

  /* Bind to port */
  err = udp_bind(data_pcb, IP_ADDR_ANY, DATA_TRANSFER_PORT);
  if (err != ERR_OK) {
    xil_printf("[ERROR] Unable to bind to port %d: err = %d\r\n",
               DATA_TRANSFER_PORT, err);
    udp_remove(data_pcb);
    return -1;
  }

  /* Set receive callback */
  udp_recv(data_pcb, udp_data_recv, NULL);

  xil_printf("[INFO] Data transfer server started successfully\r\n");
  return 0;
}

#include "xuartps_hw.h"

/* UART Buffer */
#define UART_BUFFER_SIZE 1024
static char uart_rx_buffer[UART_BUFFER_SIZE];
static u16_t uart_rx_index = 0;

/* Define STDOUT_BASEADDRESS if not defined */
#ifndef STDOUT_BASEADDRESS
#define STDOUT_BASEADDRESS 0xE0001000 // Default PS UART 1
#endif

void send_uart_data_to_qt(char *data_str) {
  if (qt_client_port == 0) {
    xil_printf("\r\n[WARNING] No Qt client connected. Data dropped.\r\n");
    return;
  }

  data_message_t msg;
  msg.msg_type = MSG_TYPE_DATA;
  msg.sequence = sequence_counter++;

  msg.length = snprintf((char *)msg.data, sizeof(msg.data), "%s", data_str);

  // Create packet buffer
  struct pbuf *pbuf =
      pbuf_alloc(PBUF_TRANSPORT, sizeof(data_message_t), PBUF_RAM);
  if (pbuf == NULL) {
    xil_printf("[ERROR] Failed to allocate pbuf for sending\r\n");
    return;
  }

  // Copy message to buffer
  memcpy(pbuf->payload, &msg, sizeof(data_message_t));

  // Send packet
  err_t err = udp_sendto(data_pcb, pbuf, &qt_client_ip, qt_client_port);
  if (err == ERR_OK) {
    stats.packets_sent++;
    stats.bytes_sent += sizeof(data_message_t);
    // xil_printf("[SENT] Data to Qt: %s\r\n", data_str); // Optional: don't
    // echo own send to avoid clutter
  } else {
    xil_printf("[ERROR] Failed to send data: %d\r\n", err);
  }

  pbuf_free(pbuf);
}

void check_uart_input(void) {
  /* Check if there is data in the UART FIFO */
  while (XUartPs_IsReceiveData(STDOUT_BASEADDRESS)) {
    u8_t c = XUartPs_ReadReg(STDOUT_BASEADDRESS, XUARTPS_FIFO_OFFSET);

    /* Echo back to terminal */
    outbyte(c);

    /* Handle Backspace */
    if (c == '\b' || c == 0x7F) {
      if (uart_rx_index > 0) {
        uart_rx_index--;
        xil_printf(" \b"); // Erase character on terminal
      }
    }
    /* Handle Enter key */
    else if (c == '\r' || c == '\n') {
      if (uart_rx_index > 0) {
        uart_rx_buffer[uart_rx_index] = '\0'; // Null terminate
        xil_printf("\r\n");                   // New line on terminal

        send_uart_data_to_qt(uart_rx_buffer);

        uart_rx_index = 0; // Reset buffer
      } else {
        xil_printf("\r\n");
      }
    }
    /* Store character */
    else {
      if (uart_rx_index < UART_BUFFER_SIZE - 1) {
        uart_rx_buffer[uart_rx_index++] = c;
      } else {
        // Buffer full, maybe auto-send or just beep?
        // For now, just ignore extra chars
      }
    }
  }
}

int transfer_data(void) {
  static u32_t counter = 0;

  /* Check for UART input every cycle */
  check_uart_input();

  // Check for connection timeout (if no data received for 5000 iterations)
  if (qt_client_port != 0 &&
      stats.connection_timeout_counter >
          500000) { // Increased timeout for manual typing
    xil_printf("[INFO] Client connection timeout - resetting client info\r\n");
    qt_client_ip.addr = 0;
    qt_client_port = 0;
    stats.connection_timeout_counter = 0;
    reset_statistics();
  }

  // Send sample data periodically (optional, keep for heartbeat/alive check)
  // Reduced frequency to not interfere with chat
  if (counter % 50000 == 0 && qt_client_port != 0) {
    // send_data_to_qt(); // Disable auto-sending sample data to focus on chat

    // Send heartbeat
    send_heartbeat();
  }

  // Display statistics every ~10 seconds
  if (counter % 100000 == 0) {
    // display_statistics(); // Disable frequent stats to keep terminal clean
    // for chat
  }

  // Increment timeout counter if no client connected
  if (qt_client_port != 0) {
    stats.connection_timeout_counter++;
  }

  counter++;
  return 0;
}
