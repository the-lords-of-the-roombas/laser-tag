#include "main_win.hpp"
#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPlainTextEdit>

Q_DECLARE_METATYPE(QSerialPortInfo);

namespace robot_tag_game {
namespace base {

MainWindow::MainWindow():
    m_scores(4,0)
{
    auto port_list = QSerialPortInfo::availablePorts();

    m_port_selector = new QComboBox;
    foreach(const QSerialPortInfo & port_info, port_list)
    {
        m_port_selector->addItem(port_info.portName(), QVariant::fromValue(port_info));
    }

    auto start_btn = new QPushButton("Start");
    connect(start_btn, &QPushButton::clicked, this, &MainWindow::startSerial);

    auto status_label = new QLabel;
    status_label->setWordWrap(false);
    status_label->setAlignment(Qt::AlignHCenter);
    status_label->setText("Off");
    //status_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    {
        QPalette p;
        p.setColor(QPalette::Background, Qt::black);
        p.setColor(QPalette::Foreground, Qt::white);
        status_label->setPalette(p);
    }

    auto connect_layout = new QHBoxLayout;
    connect_layout->addWidget(m_port_selector);
    connect_layout->addWidget(start_btn);
    connect_layout->addWidget(status_label);


    auto grid = new QGridLayout();

    QFont labelFont;

    labelFont.setPointSize(30);

    for (int i = 0; i < 4; ++i)
    {
        auto label = new QLabel;
        label->setFont(labelFont);
        label->setAlignment(Qt::AlignCenter);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        label->setText(QString::number(m_scores[i]));

        grid->addWidget(label, i / 2, i % 2);

        m_labels.append(label);
    }

    m_msg_edit = new QPlainTextEdit;

    auto send_msg_btn = new QPushButton("Send");
    connect(send_msg_btn, &QPushButton::clicked, this, &MainWindow::sendMsg);

    auto vbox = new QVBoxLayout;
    vbox->addLayout(connect_layout);
    vbox->addLayout(grid);
    vbox->addWidget(m_msg_edit);
    vbox->addWidget(send_msg_btn);

    setLayout(vbox);
}
#if 0
void MainWindow::setBuffer( QByteArray *buf )
{
    m_comm.close();

    m_comm.setBuffer(buf);
    bool ok = m_comm.open(QIODevice::WriteOnly);
    if (!ok)
        qWarning() << "MainWindow: Failed to open buffer for writing!";
}
#endif

void MainWindow::setComm( QIODevice * dev )
{
    m_comm = dev;
}

void MainWindow::startSerial()
{
    QSerialPortInfo port_info =
            m_port_selector->currentData().value<QSerialPortInfo>();

    if (port_info.isNull())
    {
        qWarning() << "MainWindow: No selected port.";
        return;
    }

    emit start(port_info, QSerialPort::UnknownBaud);
}

void MainWindow::sendMsg()
{
    if (!m_comm->isOpen())
    {
        qWarning() << "MainWindow: Buffer is not open for writing!";
        return;
    }

    QString text = m_msg_edit->toPlainText();
    qDebug() << "Sending:";
    qDebug() << text;

    m_comm->write(text.toLatin1());
}

void MainWindow::updateScore(int id, int score)
{
    if (id < 0 || id > 3)
    {
        qWarning() << "Can not update score: id out of bounds: " << id;
        return;
    }

    m_scores[id] += score;

    auto label = m_labels[id];
    label->setText(QString::number(m_scores[id]));
}

}
}
