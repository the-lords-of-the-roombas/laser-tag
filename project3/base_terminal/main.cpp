#include "comm.hpp"
#include "main_win.hpp"

#include <QApplication>
#include <QByteArray>
#include <QBuffer>

using namespace robot_tag_game::base;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QByteArray array(10,0);
    QBuffer buf(&array);
    buf.open(QIODevice::ReadWrite);

    Communicator * comm = new Communicator;
    MainWindow * window = new MainWindow;

    QObject::connect(window, &MainWindow::start, comm, &Communicator::beginSerial);
    QObject::connect(comm, &Communicator::scoreChanged,
                     window, &MainWindow::updateScore);

    //comm->begin(&buf);
    //window->setComm(&buf);

    window->show();

    return app.exec();
}
