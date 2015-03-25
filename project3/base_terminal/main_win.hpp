#ifndef ROBOT_TAG_GAME_BASE_TERMINAL_MAIN_WINDOW_INCLUDED
#define ROBOT_TAG_GAME_BASE_TERMINAL_MAIN_WINDOW_INCLUDED

#include <QVector>
#include <QWidget>
#include <QLabel>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QIODevice>
#include <QByteArray>
#include <QBuffer>
#include <QPlainTextEdit>
#include <QComboBox>

namespace robot_tag_game {
namespace base {

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow();
    QVector<QLabel*> m_labels;
    QVector<int> m_scores;
    QSize sizeHint() const { return QSize(400,400); }
    //void setBuffer( QByteArray *buf );
    void setComm( QIODevice * );

public slots:
    void updateScore(int id, int score);

private slots:
    void startSerial();
    void sendMsg();

signals:
    void start(const QSerialPortInfo & port, QSerialPort::BaudRate);

private:
    //QBuffer m_comm;
    QIODevice *m_comm;
    QComboBox *m_port_selector;
    QPlainTextEdit *m_msg_edit;
};

}
}

#endif // ROBOT_TAG_GAME_BASE_TERMINAL_MAIN_WINDOW_INCLUDED
