
#include "radio.h"
#include "state.h"
#include "joystick.h"
#include "drive.h"
#include "shoot.h"
#include "scheduler.h"
#include "cops_and_robbers.h"

volatile uint8_t rxflag = 0;
 
// packets are transmitted to this address
static uint8_t test_channel = 102;
static uint8_t test_address[5] = { 0xAB, 0xAB, 0xAB, 0xAB, 0xAB };
 
// this is this radio's address
static uint8_t my_addr[5] = { 0x88, 0x13, 0x98, 0x87, 0x17 };

static state system_state;

static const int task_count = 3;
static task_t tasks[task_count];

static const int roomba_id = COP1;
//static const uint8_t tx_channel = ROOMBA_FREQUENCIES[roomba_id];
//static uint8_t * const tx_address = ROOMBA_ADDRESSES[roomba_id];
static uint8_t tx_channel = 104;
static uint8_t tx_address[] = {0xAA,0xAA,0xAA,0xAA,0xAA};
//static uint8_t tx_channel = test_channel;
//static uint8_t* tx_address = test_address;

void setup()
{
  //Serial.begin(9600);

  pinMode(13, OUTPUT);

  // configure radio

  pinMode(11, OUTPUT);

  digitalWrite(11, LOW);
  delay(100);
  digitalWrite(11, HIGH);
  delay(100);

  Radio_Init(tx_channel);

  // configure the receive settings for radio pipe 0
  Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
  // configure radio transceiver settings
  // TODO: Check data rate!
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
  // set destination address

  // store our address into the transmitted packet
  memcpy(system_state.tx_packet.payload.command.sender_address, my_addr, 5);

#if 0
  joystick_init(&system_state);
  drive_init(&system_state);
  shoot_init(&system_state);

  scheduler_task_init(tasks+0, 0, 200e3, &joystick_read, &system_state);
  scheduler_task_init(tasks+1, 50e3, 200e3, &drive, &system_state);
  scheduler_task_init(tasks+2, 25e3, 200e3, &shoot, &system_state);

  scheduler_init(tasks, task_count);
#endif
}

void test_debug_station()
{
  static uint16_t count = 0;
  ++count;

  radiopacket_t &pkt = system_state.tx_packet;

  pkt.type = MESSAGE;

  memcpy(pkt.payload.message.address, my_addr, 5);


  sprintf((char*)pkt.payload.message.messagecontent,
           "zzz");
  sprintf((char*)pkt.payload.message.messagecontent + 3,
           "%d", count);

  Radio_Set_Tx_Addr(tx_address);

  int result = Radio_Transmit(&pkt, RADIO_WAIT_FOR_TX);
#if 1
  digitalWrite(13, HIGH);
  delay (10);
  digitalWrite(13, LOW);

  if (result == RADIO_TX_SUCCESS)
  {
    delay(100);
    digitalWrite(13, HIGH);
    delay (10);
    digitalWrite(13, LOW);
  }
#endif

  //digitalWrite(13, (count % 2 == 0 )? HIGH : LOW);
#if 1
  while(rxflag)
  {
      // remember always to read the packet out of the radio, even
      // if you don't use the data.
      if (Radio_Receive(&system_state.rx_packet) != RADIO_RX_MORE_PACKETS)
      {
        // if there are no more packets on the radio, clear the receive flag;
        // otherwise, we want to handle the next packet on the next loop iteration.
        rxflag = 0;
      }
  }
#endif

  delay(1000);
}

void test_roomba()
{
  radiopacket_t &pkt = system_state.tx_packet;

#if 0
  static bool direction = false;
  direction = !direction;

  uint16_t speed = 50;
  uint16_t radius = direction ? 1 : -1;

  char *speed_bytes = (char*)(&speed);
  char *radius_bytes = (char*)(&radius);


  pkt.type = COMMAND;
  memcpy(pkt.payload.command.sender_address, my_addr, 5);
  pkt.payload.command.command = 137;
  pkt.payload.command.num_arg_bytes = 4;
  pkt.payload.command.arguments[0] = speed_bytes[0];
  pkt.payload.command.arguments[1] = speed_bytes[1];
  //pkt.payload.command.arguments[2] = radius_bytes[0];
  //pkt.payload.command.arguments[3] = radius_bytes[1];
#if 0
  if (direction)
  {
    pkt.payload.command.arguments[2] = 0;
    pkt.payload.command.arguments[3] = 1;
  }
  else
  {
    pkt.payload.command.arguments[2] = 0xFF;
    pkt.payload.command.arguments[3] = 0xFF;
  }
#endif

#endif

#if 1
  pkt.type = IR_COMMAND;
#endif

  Radio_Set_Tx_Addr(tx_address);

  int result = Radio_Transmit(&pkt, RADIO_WAIT_FOR_TX);
#if 1
  digitalWrite(13, HIGH);
  delay (10);
  digitalWrite(13, LOW);

  if (result == RADIO_TX_SUCCESS)
  {
    delay(100);
    digitalWrite(13, HIGH);
    delay (10);
    digitalWrite(13, LOW);
  }
#endif

  while(rxflag)
  {
      if (Radio_Receive(&system_state.rx_packet) != RADIO_RX_MORE_PACKETS)
      {
        rxflag = 0;
      }
  }

  delay(1000);
}

void loop()
{
  //test_debug_station();

  test_roomba();

  //scheduler_run();
}

extern "C" {
void radio_rxhandler(uint8_t pipe_number)
{
  rxflag = 1;
}
}
