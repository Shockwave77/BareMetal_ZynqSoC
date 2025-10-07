#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QSpinBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void sendData();
    void sendCommand();
    void sendHeartbeat();
    void readPendingDatagrams();
    void updateConnectionStatus();

private:
    void setupUI();
    void logMessage(const QString &message);
    void processReceivedData(const QByteArray &data, const QHostAddress &sender, quint16 port);
    
    QUdpSocket *udpSocket;
    QTimer *heartbeatTimer;
    
    // UI Components
    QTextEdit *logTextEdit;
    QLineEdit *serverIpEdit;
    QSpinBox *serverPortSpinBox;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QLineEdit *dataLineEdit;
    QPushButton *sendDataButton;
    QPushButton *sendCommandButton;
    QPushButton *sendHeartbeatButton;
    QLabel *statusLabel;
    QLabel *packetsReceivedLabel;
    QLabel *packetsSentLabel;
    QLabel *bytesReceivedLabel;
    QLabel *bytesSentLabel;
    
    // Statistics
    int packetsReceived;
    int packetsSent;
    int bytesReceived;
    int bytesSent;
    int sequenceNumber;
    
    bool connected;
    QHostAddress serverAddress;
    quint16 serverPort;
};

#endif // MAINWINDOW_H
