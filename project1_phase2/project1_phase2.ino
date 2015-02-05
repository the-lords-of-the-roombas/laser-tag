
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

static const int task_count = 4;
static task_t tasks[task_count];

#if 1
static const int roomba_id = COP1;
static const uint8_t tx_channel = ROOMBA_FREQUENCIES[roomba_id];
static uint8_t * const tx_address = ROOMBA_ADDRESSES[roomba_id];
#else
static uint8_t tx_channel = test_channel;
static uint8_t* tx_address = test_address;
#endif

void empty_rx_buffer(void*)
{
  //Serial.println("rx");

  radiopacket_t rx_pkt;

  while(rxflag)
  {
      if (Radio_Receive(&rx_pkt) != RADIO_RX_MORE_PACKETS)
      {
        rxflag = 0;
      }
  }

}

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

  Radio_Set_Tx_Addr(tx_address);

  // store our address into the transmitted packet
  memcpy(system_state.tx_packet.payload.command.sender_address, my_addr, 5);

  joystick_init(&system_state);
  drive_init(&system_state);
  shoot_init(&system_state);

  scheduler_task_init(tasks+0, 0, 50e3, &joystick_read, &system_state);
  scheduler_task_init(tasks+1, 0e3, 50e3, &drive, &system_state);
  scheduler_task_init(tasks+2, 0e3, 50e3, &shoot, &system_state);
  scheduler_task_init(tasks+3, 0e3, 50e3, &empty_rx_buffer, &system_state);

  scheduler_init(tasks, task_count);
}


void loop()
{
  scheduler_run();
}

extern "C" {
void radio_rxhandler(uint8_t pipe_number)
{
  rxflag = 1;
}
}
