#include "irobot.hpp"
#include <Arduino.h>

irobot::irobot(HardwareSerial *serial, int brc_pin):
    m_serial(serial),
    m_brc_pin(brc_pin)
{
}

bool irobot::begin()
{
    pinMode(m_brc_pin, OUTPUT);

    // Wake up the robot.

    digitalWrite(m_brc_pin, LOW);
    delay(100);
    digitalWrite(m_brc_pin, HIGH);
    delay(2000);

    // Change baud rate to 19200.

    //digitalWrite(m_brc_pin, HIGH);
    for(int i = 0; i < 4; ++i)
    {
        _delay_ms(100);
        digitalWrite(m_brc_pin, LOW);
        _delay_ms(100);
        digitalWrite(m_brc_pin, HIGH);
    }

    _delay_ms(100);

    // Keep the pin low to keep the robot alive:

    digitalWrite(m_brc_pin, LOW);

    // Initialize Arduino Serial1

    m_serial->begin(19200);

    // Initiate conversation with the robot

    m_serial->write(irobot::op_start);

    return true;
}

void irobot::stop()
{
    digitalWrite(m_brc_pin, HIGH);
    m_serial->write(irobot::op_stop);
}

void irobot::send( opcode op )
{
    m_serial->write(op);
}

void irobot::send( opcode op, char byte )
{
    m_serial->write(op);
    m_serial->write(byte);
}

void irobot::send(opcode op, const uint8_t *data, size_t data_size )
{
    m_serial->write(op);
    m_serial->write(data, data_size);
}

size_t irobot::receive(uint8_t *data, size_t size )
{
    return m_serial->readBytes((char*) data, size);
}

void irobot::flush_received()
{
    while(m_serial->available())
    {
        m_serial->read();
    }
}
