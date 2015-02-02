
#include "radio.h"
#include "state.h"
#include "joystick.h"
#include "drive.h"
#include "scheduler.h"

volatile uint8_t rxflag = 0;
 
// packets are transmitted to this address
static uint8_t station_addr[5] = { 0xAB, 0xAB, 0xAB, 0xAB, 0xAB };
 
// this is this radio's address
static uint8_t my_addr[5] = { 0x41, 0x32, 0x54, 0x32, 0x10 };

static state system_state;

static const int task_count = 2;
static task_t tasks[task_count];

void setup()
{
  // configure radio

  pinMode(10, OUTPUT);

  digitalWrite(10, LOW);
  delay(100);
  digitalWrite(10, HIGH);
  delay(100);

  Radio_Init();

  // configure the receive settings for radio pipe 0
  Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
  // configure radio transceiver settings
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
  // set destination address
  Radio_Set_Tx_Addr(station_addr);

  // store our address into the transmitted packet
  for (int i = 0; i < 5; ++i)
    system_state.tx_packet.payload.command.sender_address[i] = my_addr[i];

  joystick_init(&system_state);
  drive_init(&system_state);

  scheduler_task_init(tasks+0, 0, 50e3, &joystick_read, &system_state);
  scheduler_task_init(tasks+1, 25e3, 50e3, &drive, &system_state);
}

void loop()
{
  // FIXME: use scheduler

  scheduler_run();
}
 
void radio_rxhandler(uint8_t pipe_number)
{
  rxflag = 1;
}
