#include "comm.hpp"
#include "protocol.h"

#include <QDebug>
#include <QBuffer>
#include <QSerialPortInfo>

namespace robot_tag_game {
namespace base {

#if 0
CommunicateBroker::CommunicateBroker(serial *serial, QObject * parent):
    QObject(parent),
    m_serial(serial)
{}

CommunicateThread::CommunicateThread(serial *serial, QObject * parent = 0 ):
    QThread(parent)
{
    m_broker = new CommunicateBroker(serial, this);
}

void CommunicateThread::run()
{
// TODO
}

Communicator::Communicator(QObject * parent):
    QObject(parent)
{

}

bool begin(const QString & port, int baud_rate)
{
    serial::error_code error =
            m_serial.open(port.toLatin1().constData(), baud_rate);

    return !error;
}
#endif

Communicator::Communicator( QObject * parent ):
    QObject(parent),
    m_source(nullptr),
    m_parse_line_error(false)
{
    connect(&m_serial_port, &QIODevice::readyRead,
            this, &Communicator::onReadyRead);
}

bool Communicator::begin(QIODevice *source)
{
    if (m_source)
    {
        return false;
    }

    m_source = source;
    m_parse_line_error = false;

    connect(m_source, &QIODevice::readyRead,
            this, &Communicator::onReadyRead);

    //return source->open(QIODevice::ReadOnly);
    return true;
}

void Communicator::beginSerial(const QSerialPortInfo & port_info,
                               QSerialPort::BaudRate rate)
{
    if (m_source)
    {
        return;
    }

    qDebug() << "Communicator: connecting to:" << port_info.portName();

    // Make sure to flush until next line.
    m_parse_line_error = true;

    m_serial_port.setPort(port_info);
    m_serial_port.setBaudRate(rate);

    bool ok = m_serial_port.open(QIODevice::ReadOnly);
    if (!ok)
    {
        qWarning() << "Communicator: Failed to open serial port.";
    }
    else
    {
        qDebug() << "Communicator: Serial port open.";
        m_source = &m_serial_port;
    }
}

void Communicator::stop()
{
    if (m_serial_port.isOpen())
        m_serial_port.close();

    if (!m_source)
        return;

    if (m_source != &m_serial_port)
        disconnect(m_source, &QIODevice::readyRead,
                   this, &Communicator::onReadyRead);

    m_source = nullptr;
}

bool Communicator::flushToEndOfLine()
{
    char c;
    while(m_source->getChar(&c))
    {
        if(c == '\n')
            return true;
    }
    return false;
}

void Communicator::onReadyRead()
{
    qDebug() << "Communicator: got new data.";

    while(m_source->bytesAvailable())
    {
        if (m_parse_line_error)
        {
            if(flushToEndOfLine())
            {
                m_parse_line_error = false;
            }
            continue;
        }

        QByteArray data = m_source->readLine(max_line_size - m_line.size());
        m_line.append(data);

        qDebug() << "Communicator: got:" << QString(data);

        if (m_line.endsWith('\n'))
        {
            processLine(m_line);
            m_line.clear();
        }
        else if (m_line.size() == max_line_size)
        {
            qWarning() << "Line too large.";

            m_parse_line_error = true;

            m_line.clear();
        }
    }
}

void Communicator::processLine( QByteArray & line )
{
    qDebug() << "Communicator: processing line:" << QString(line);

    QBuffer buf(&line);
    buf.open(QIODevice::ReadOnly);

    QTextStream stream(&buf);

    unsigned int type = -1;
    stream >> type;

    switch(type)
    {
    case terminal::msg_score:
    {
        int id;
        int score;
        stream >> id;
        stream >> score;
        emit scoreChanged(id, score);
        break;
    }
    default:
        qWarning() << "Unknown message type: " << type;
    }
}

}
}
