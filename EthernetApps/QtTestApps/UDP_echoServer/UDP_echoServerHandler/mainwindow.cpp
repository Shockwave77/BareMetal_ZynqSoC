#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress);
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);

    // Example: send a test packet to the Zynq board (replace with your board's IP)
    QHostAddress zynqAddress("192.168.1.10"); // <-- Set your Zynq board IP here
    QByteArray testData = "Hello from Qt UDP client!";
    sendEchoTestPacket(zynqAddress, 7, testData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendEchoTestPacket(const QHostAddress &address, quint16 port, const QByteArray &data)
{
    udpSocket->writeDatagram(data, address, port);
    ui->textEdit->append("Sent UDP packet to " + address.toString() + ":" + QString::number(port) + ", data: " + QString::fromUtf8(data));
}

void MainWindow::onReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        ui->textEdit->append("Received UDP packet from " + sender.toString() + ":" + QString::number(senderPort) + ", data: " + QString::fromUtf8(datagram));
    }
}

void MainWindow::onSendButtonClicked()
{
    QString text = ui->lineEdit->text();
    if (text.isEmpty()) return;
    QHostAddress zynqAddress("192.168.1.10"); // <-- Set your Zynq board IP here
    sendEchoTestPacket(zynqAddress, 7, text.toUtf8());
    ui->lineEdit->clear();
}
