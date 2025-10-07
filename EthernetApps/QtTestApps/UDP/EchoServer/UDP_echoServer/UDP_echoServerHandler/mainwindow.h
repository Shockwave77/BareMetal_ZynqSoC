#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;
    void sendEchoTestPacket(const QHostAddress &address, quint16 port, const QByteArray &data);

private slots:
    void onReadyRead();
    void onSendButtonClicked();
};
#endif // MAINWINDOW_H
