#ifndef ROBOT_TAG_GAME_BASE_TERMINAL_COMM_INCLUDED
#define ROBOT_TAG_GAME_BASE_TERMINAL_COMM_INCLUDED

//#include "serial.hpp"

#include <QObject>
#include <QSerialPort>

namespace robot_tag_game {
namespace base {

#if 0
class CommunicateBroker : public QObject
{
public:
    CommunicateBroker(serial *serial, QObject * parent = 0);

public slots:
    void stop();

signals:
    void score(int id, int score);

private:
    serial *m_serial;
};

class CommunicateThread : public QThread
{
public:
    CommunicateThread(serial *serial, QObject * parent = 0 );
    CommunicateBroker *broker() { return m_broker; }
private:
    void run();
    CommunicateBroker *m_broker;
};

class Communicator : public QObject
{
public:
    Communicator( QObject * parent = 0 );
    void begin(const QString & port, int baud_rate);

signals:
    void score(int id, int score);

private:
    serial m_serial;
};
#endif

class Communicator : public QObject
{
    Q_OBJECT
public:
    Communicator( QObject * parent = 0 );

    bool begin(QIODevice *);
    void stop();


public slots:
    void beginSerial(const QSerialPortInfo & port, QSerialPort::BaudRate);


signals:
    void scoreChanged(int id, int score);

private slots:
    void onReadyRead();

private:
    static constexpr unsigned int max_line_size = 100;

    bool flushToEndOfLine();
    void processLine(QByteArray &);

    QSerialPort m_serial_port;
    QIODevice *m_source;
    QByteArray m_line;
    bool m_parse_line_error;
};

}
}

#endif // ROBOT_TAG_GAME_BASE_TERMINAL_COMM_INCLUDED
