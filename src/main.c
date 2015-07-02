// BGAPI を用いて iBeacon 風のアドバタイジングするサンプルコード
// by bunji2

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>

#include "cmd_def.h"
#include "uart.h"

#include "config_data.h"


#define CLARG_PORT 1
#define CLARG_ACTION 2

#define UART_TIMEOUT 1000

enum actions {
  action_none,
  action_beacon,
  action_info,
};
enum actions action = action_none;

typedef enum {
  state_disconnected,
  state_adverting,
  state_finish,
  state_last
} states;

states state = state_disconnected;
config_data_t cf;

void usage(char *exe)
{
  printf("%s <list|COMx <info|config_filename>>\n", exe);
}

void change_state(states new_state)
{
  state = new_state;
}

/**
 * Compare Bluetooth addresses
 *
 * @param first First address
 * @param second Second address
 * @return Zero if addresses are equal
 */
int cmp_bdaddr(bd_addr first, bd_addr second)
{
  int i;
  for (i = 0; i < sizeof(bd_addr); i++) {
    if (first.addr[i] != second.addr[i]) return 1;
  }
  return 0;
}

void print_bdaddr(bd_addr bdaddr)
{
  printf("%02x:%02x:%02x:%02x:%02x:%02x",
      bdaddr.addr[5],
      bdaddr.addr[4],
      bdaddr.addr[3],
      bdaddr.addr[2],
      bdaddr.addr[1],
      bdaddr.addr[0]);
}

void print_raw_packet(struct ble_header *hdr, unsigned char *data)
{
  int i;
  printf("Incoming packet: ");
  for (i = 0; i < sizeof(*hdr); i++) {
    printf("%02x ", ((unsigned char *)hdr)[i]);
  }
  for (i = 0; i < hdr->lolen; i++) {
    printf("%02x ", data[i]);
  }
  printf("\n");
}

void output(uint8 len1, uint8* data1, uint16 len2, uint8* data2)
{
  if (uart_tx(len1, data1) || uart_tx(len2, data2)) {
    printf("ERROR: Writing to serial port failed\n");
    exit(1);
  }
}

int read_message(int timeout_ms)
{
  unsigned char data[256]; // enough for BLE
  struct ble_header hdr;
  int r;
  const struct ble_msg *msg;

  r = uart_rx(sizeof(hdr), (unsigned char *)&hdr, timeout_ms);
  if (!r) {
    return -1; // timeout
  }
  else if (r < 0) {
    printf("ERROR: Reading header failed. Error code:%d\n", r);
    return 1;
  }

  if (hdr.lolen) {
    r = uart_rx(hdr.lolen, data, timeout_ms);
    if (r <= 0) {
      printf("ERROR: Reading data failed. Error code:%d\n", r);
      return 1;
    }
  }

  msg = (const struct ble_msg *)ble_get_msg_hdr(hdr);

#ifdef DEBUG
  print_raw_packet(&hdr, data);
#endif

  if (!msg) {
    printf("ERROR: Unknown message received\n");
    exit(1);
  }

  msg->handler(data);

  return 0;
}

void print_str_uint8array(uint8array data) {
  int i;
  for (i=0; i<data.len; i++) {
    printf("%c", data.data[data.len-i-1]);
  }
}

void print_hex_uint8array(uint8 *data, uint8 len) {
  int i;
  for (i=0; i<len; i++) {
    printf("%02hx", data[i]);
  }
}

void print_hex_uint8array_rev(uint8 *data, uint8 len) {
  int i;
  for (i=0; i<len; i++) {
    printf("%02hx", data[len-i-1]);
  }
}

void do_beacon() {
  // advertisement packet data in iBeacon format
  uint8 advdata [30] = {
    // LEN=2bytes
    0x02,
    // TYPE=Flags
    0x01,
    // VALUE=LE General Discovery, single mode device
    0x06,
    // LEN=26bytes
    0x1a,
    // TYPE=Manufacturer Specific Data
    0xff,
    // VALUE
    // Apple(0x004C)
    0x4c, 0x00,
    // iBeacon advertise(0x02)
    0x02,
    // size=21bytes
    0x15,
    // UUID=e2c56db5-dffb-48d2-b060-d0f5a71096e0
    0xe2, 0xc5, 0x6d, 0xb5, 0xdf, 0xfb, 0x48, 0xd2,
    0xb0, 0x60, 0xd0, 0xf5, 0xa7, 0x10, 0x96, 0xe0,
    // Major : 0x0000
    0x00, 0x00,
    // Minor : 0x0000
    0x00, 0x00,
    // TX power : -58
    0xc6
  };
  int i;
  
  for (i=0; i<16; i++) {
    advdata[9+i] = cf.uuid[i];
  }
  advdata[25] = cf.major>>8;
  advdata[26] = cf.major&0xFF;
  advdata[27] = cf.minor>>8;
  advdata[28] = cf.minor&0xFF;
  advdata[29] = cf.txpower;
  
  // Set advertisement parameters
  //   adv_interval_min: 160 (100ms)
  //   adv_interval_max: 160 (100ms)
  //   adv_channels: 7 (all three channels are used)
  // ble_cmd_gap_set_adv_parameters(160, 160, 7);
  ble_cmd_gap_set_adv_parameters(
    cf.adv_interval_min, cf.adv_interval_min, cf.adv_channels);

  // Set advertisement data
  //   set_scanrsp: 0 (sets advertisement data)
  //   adv_data_len: 30
  //   adv_data_data: advdata
  ble_cmd_gap_set_adv_data(0, 30, advdata);

  // Set to advertising mode
  //   discoverable mode: 4 (with user data)
  //   connectable mode: 0 (not connectable)
  ble_cmd_gap_set_mode(4, 0);

}


// ここから、このコードでは使用していない関数
void ble_rsp_system_get_info(
  const struct ble_msg_system_get_info_rsp_t *msg
){
}

void ble_evt_gap_scan_response(
  const struct ble_msg_gap_scan_response_evt_t *msg
){
}

void ble_evt_connection_status(
  const struct ble_msg_connection_status_evt_t *msg
){
}

void ble_evt_attclient_attribute_value(
  const struct ble_msg_attclient_attribute_value_evt_t *msg
){

}

void ble_evt_attclient_procedure_completed(
  const struct ble_msg_attclient_procedure_completed_evt_t *msg
){
}

void ble_evt_attclient_group_found(
  const struct ble_msg_attclient_group_found_evt_t *msg
){
/*
  uint16 uuid;
  
  printf("#ble_evt_attclient_group_found [%s]\n", state_names[state]);

  if (msg->uuid.len == 0) return;
  uuid = (msg->uuid.data[1] << 8) | msg->uuid.data[0];

  printf("service=0x%04x, handles=%d-%d\n", uuid, msg->start, msg->end);
*/
}

void ble_evt_attclient_find_information_found(
  const struct ble_msg_attclient_find_information_found_evt_t *msg
){
/*
  printf("#ble_evt_attclient_find_information_found [%s]\n",
    state_names[state]);
*/
}

void ble_evt_connection_disconnected(
  const struct ble_msg_connection_disconnected_evt_t *msg
) {
}

// ここまで、このコードでは使用していない関数


int main(int argc, char *argv[]) {
  char *uart_port = "";

  // 引数が不足しているかのチェック
  if (argc <= CLARG_PORT) {
    usage(argv[0]);
    return 1;
  }

  // COM ポート引数のチェック
  if (argc > CLARG_PORT) {
    if (strcmp(argv[CLARG_PORT], "list") == 0) {
      uart_list_devices(); // デバイスのリスト表示
      return 1;
    }
    else {
      uart_port = argv[CLARG_PORT];
    }
  }

  // アクション引数のチェック
  if (argc > CLARG_ACTION) {
    int i;
    for (i = 0; i < strlen(argv[CLARG_ACTION]); i++) {
      // 小文字にしておく
      argv[CLARG_ACTION][i] = tolower(argv[CLARG_ACTION][i]);
    }

    if (strcmp(argv[CLARG_ACTION], "info") == 0) {
      action = action_info;    // デバイス情報の表示
    } else {
      if (config_data_load(argv[CLARG_ACTION]) != 0) {
        config_data_print();
        action = action_beacon; // Beacon
      }
    }
  }
  if (action == action_none) {
    usage(argv[0]);
    return 1;
  }

  bglib_output = output;

  // COMポートのオープン
  if (uart_open(uart_port)) {
    printf("ERROR: Unable to open serial port\n");
    return 1;
  }

  // BLED のリセット
  ble_cmd_system_reset(0);
  uart_close();
  do {
//    usleep(500000); // 0.5s
    Sleep(500); // 0.5s
  } while (uart_open(uart_port));

  // アクションの実行
  if (action == action_info) {

    // [2] デバイスの情報表示の要求
    ble_cmd_system_get_info();

  } else if (action == action_beacon) {

    change_state(state_adverting);
    // [3] Beacon
    do_beacon();
  }

  // メッセージループ
  while (state != state_finish) {
    if (read_message(UART_TIMEOUT) > 0) break;
  }

  // COMポートのクローズ
  uart_close();

  return 0;
}
