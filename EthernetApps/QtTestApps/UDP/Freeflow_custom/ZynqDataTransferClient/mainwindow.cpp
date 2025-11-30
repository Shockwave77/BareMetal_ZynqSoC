#include "mainwindow.h"
#include <QDateTime>
#include <QNetworkDatagram>


// Message types (matching Zynq application)
enum MessageType {
  MSG_TYPE_DATA = 0x01,
  MSG_TYPE_COMMAND = 0x02,
  MSG_TYPE_RESPONSE = 0x03,
  MSG_TYPE_HEARTBEAT = 0x04
};

// Data structure (matching Zynq application)
struct DataMessage {
  quint8 msgType;
  quint8 sequence;
  quint16 length;
  char data[1020]; // Total size minus header
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), udpSocket(nullptr), heartbeatTimer(nullptr),
      packetsReceived(0), packetsSent(0), bytesReceived(0), bytesSent(0),
      sequenceNumber(0), connected(false), serverPort(8888) {
  setupUI();

  // Initialize UDP socket
  udpSocket = new QUdpSocket(this);
  connect(udpSocket, &QUdpSocket::readyRead, this,
          &MainWindow::readPendingDatagrams);

  // Initialize heartbeat timer
  heartbeatTimer = new QTimer(this);
  heartbeatTimer->setInterval(5000); // 5 seconds
  connect(heartbeatTimer, &QTimer::timeout, this, &MainWindow::sendHeartbeat);

  // Set default server IP
  serverIpEdit->setText("192.168.1.10");
  serverPortSpinBox->setValue(8888);

  logMessage("Zynq Data Transfer Client Started");
  logMessage("Ready to connect to Zynq server...");
}

MainWindow::~MainWindow() {
  if (connected) {
    disconnectFromServer();
  }
}

void MainWindow::setupUI() {
  setWindowTitle("Zynq Data Transfer Client");
  setFixedSize(800, 600);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

  // Connection Group
  QGroupBox *connectionGroup = new QGroupBox("Connection", this);
  QGridLayout *connectionLayout = new QGridLayout(connectionGroup);

  connectionLayout->addWidget(new QLabel("Server IP:"), 0, 0);
  serverIpEdit = new QLineEdit(this);
  connectionLayout->addWidget(serverIpEdit, 0, 1);

  connectionLayout->addWidget(new QLabel("Server Port:"), 0, 2);
  serverPortSpinBox = new QSpinBox(this);
  serverPortSpinBox->setRange(1, 65535);
  connectionLayout->addWidget(serverPortSpinBox, 0, 3);

  connectButton = new QPushButton("Connect", this);
  connect(connectButton, &QPushButton::clicked, this,
          &MainWindow::connectToServer);
  connectionLayout->addWidget(connectButton, 0, 4);

  disconnectButton = new QPushButton("Disconnect", this);
  disconnectButton->setEnabled(false);
  connect(disconnectButton, &QPushButton::clicked, this,
          &MainWindow::disconnectFromServer);
  connectionLayout->addWidget(disconnectButton, 0, 5);

  statusLabel = new QLabel("Status: Disconnected", this);
  connectionLayout->addWidget(statusLabel, 1, 0, 1, 6);

  mainLayout->addWidget(connectionGroup);

  // Data Transfer Group
  QGroupBox *dataGroup = new QGroupBox("Data Transfer", this);
  QVBoxLayout *dataLayout = new QVBoxLayout(dataGroup);

  QHBoxLayout *sendLayout = new QHBoxLayout();
  dataLineEdit = new QLineEdit(this);
  dataLineEdit->setPlaceholderText("Enter data to send to Zynq...");
  sendLayout->addWidget(dataLineEdit);

  sendDataButton = new QPushButton("Send Data", this);
  sendDataButton->setEnabled(false);
  connect(sendDataButton, &QPushButton::clicked, this, &MainWindow::sendData);
  sendLayout->addWidget(sendDataButton);

  sendCommandButton = new QPushButton("Send Command", this);
  sendCommandButton->setEnabled(false);
  connect(sendCommandButton, &QPushButton::clicked, this,
          &MainWindow::sendCommand);
  sendLayout->addWidget(sendCommandButton);

  sendHeartbeatButton = new QPushButton("Send Heartbeat", this);
  sendHeartbeatButton->setEnabled(false);
  connect(sendHeartbeatButton, &QPushButton::clicked, this,
          &MainWindow::sendHeartbeat);
  sendLayout->addWidget(sendHeartbeatButton);

  dataLayout->addLayout(sendLayout);

  // Statistics Group
  QGroupBox *statsGroup = new QGroupBox("Statistics", this);
  QGridLayout *statsLayout = new QGridLayout(statsGroup);

  statsLayout->addWidget(new QLabel("Packets Received:"), 0, 0);
  packetsReceivedLabel = new QLabel("0", this);
  statsLayout->addWidget(packetsReceivedLabel, 0, 1);

  statsLayout->addWidget(new QLabel("Packets Sent:"), 0, 2);
  packetsSentLabel = new QLabel("0", this);
  statsLayout->addWidget(packetsSentLabel, 0, 3);

  statsLayout->addWidget(new QLabel("Bytes Received:"), 1, 0);
  bytesReceivedLabel = new QLabel("0", this);
  statsLayout->addWidget(bytesReceivedLabel, 1, 1);

  statsLayout->addWidget(new QLabel("Bytes Sent:"), 1, 2);
  bytesSentLabel = new QLabel("0", this);
  statsLayout->addWidget(bytesSentLabel, 1, 3);

  // Chat / Last Message Display
  QGroupBox *chatGroup = new QGroupBox("Last Message Received", this);
  QVBoxLayout *chatLayout = new QVBoxLayout(chatGroup);
  lastReceivedLabel = new QLabel("Waiting for messages...", this);
  lastReceivedLabel->setStyleSheet(
      "QLabel { font-size: 14pt; font-weight: bold; color: #0055aa; }");
  lastReceivedLabel->setWordWrap(true);
  chatLayout->addWidget(lastReceivedLabel);

  dataLayout->addWidget(statsGroup);
  dataLayout->addWidget(chatGroup); // Add chat group to layout
  mainLayout->addWidget(dataGroup);

  // Log Group
  QGroupBox *logGroup = new QGroupBox("Communication Log", this);
  QVBoxLayout *logLayout = new QVBoxLayout(logGroup);

  logTextEdit = new QTextEdit(this);
  logTextEdit->setReadOnly(true);
  logTextEdit->setMaximumHeight(200);
  logLayout->addWidget(logTextEdit);

  mainLayout->addWidget(logGroup);
}

void MainWindow::connectToServer() {
  QString ipString = serverIpEdit->text().trimmed();
  if (ipString.isEmpty()) {
    QMessageBox::warning(this, "Error", "Please enter server IP address");
    return;
  }

  serverAddress = QHostAddress(ipString);
  if (serverAddress.isNull()) {
    QMessageBox::warning(this, "Error", "Invalid IP address");
    return;
  }

  serverPort = static_cast<quint16>(serverPortSpinBox->value());

  // Bind to a local port for receiving
  if (!udpSocket->bind(QHostAddress::Any, 0)) {
    QMessageBox::critical(this, "Error", "Failed to bind UDP socket");
    return;
  }

  connected = true;
  connectButton->setEnabled(false);
  disconnectButton->setEnabled(true);
  sendDataButton->setEnabled(true);
  sendCommandButton->setEnabled(true);
  sendHeartbeatButton->setEnabled(true);

  statusLabel->setText(QString("Status: Connected to %1:%2")
                           .arg(serverAddress.toString())
                           .arg(serverPort));

  logMessage(QString("Connected to Zynq server at %1:%2")
                 .arg(serverAddress.toString())
                 .arg(serverPort));

  // Send initial heartbeat
  sendHeartbeat();

  // Start heartbeat timer
  heartbeatTimer->start();
}

void MainWindow::disconnectFromServer() {
  if (heartbeatTimer->isActive()) {
    heartbeatTimer->stop();
  }

  connected = false;
  connectButton->setEnabled(true);
  disconnectButton->setEnabled(false);
  sendDataButton->setEnabled(false);
  sendCommandButton->setEnabled(false);
  sendHeartbeatButton->setEnabled(false);

  statusLabel->setText("Status: Disconnected");

  logMessage("Disconnected from Zynq server");

  udpSocket->close();
}

void MainWindow::sendData() {
  if (!connected)
    return;

  QString dataString = dataLineEdit->text().trimmed();
  if (dataString.isEmpty()) {
    QMessageBox::warning(this, "Error", "Please enter data to send");
    return;
  }

  DataMessage msg;
  msg.msgType = MSG_TYPE_DATA;
  msg.sequence = static_cast<quint8>(sequenceNumber++);

  QByteArray dataBytes = dataString.toUtf8();
  msg.length = static_cast<quint16>(qMin(dataBytes.size(), 1020));
  memcpy(msg.data, dataBytes.constData(), msg.length);

  QByteArray packet(reinterpret_cast<const char *>(&msg), sizeof(msg));

  qint64 bytesWritten =
      udpSocket->writeDatagram(packet, serverAddress, serverPort);
  if (bytesWritten == packet.size()) {
    packetsSent++;
    bytesSent += static_cast<int>(bytesWritten);
    packetsSentLabel->setText(QString::number(packetsSent));
    bytesSentLabel->setText(QString::number(bytesSent));

    logMessage(QString("Sent data: %1").arg(dataString));
    dataLineEdit->clear();
  } else {
    logMessage("Failed to send data");
  }
}

void MainWindow::sendCommand() {
  if (!connected)
    return;

  QString commandString = dataLineEdit->text().trimmed();
  if (commandString.isEmpty()) {
    QMessageBox::warning(this, "Error", "Please enter command to send");
    return;
  }

  DataMessage msg;
  msg.msgType = MSG_TYPE_COMMAND;
  msg.sequence = static_cast<quint8>(sequenceNumber++);

  QByteArray dataBytes = commandString.toUtf8();
  msg.length = static_cast<quint16>(qMin(dataBytes.size(), 1020));
  memcpy(msg.data, dataBytes.constData(), msg.length);

  QByteArray packet(reinterpret_cast<const char *>(&msg), sizeof(msg));

  qint64 bytesWritten =
      udpSocket->writeDatagram(packet, serverAddress, serverPort);
  if (bytesWritten == packet.size()) {
    packetsSent++;
    bytesSent += static_cast<int>(bytesWritten);
    packetsSentLabel->setText(QString::number(packetsSent));
    bytesSentLabel->setText(QString::number(bytesSent));

    logMessage(QString("Sent command: %1").arg(commandString));
    dataLineEdit->clear();
  } else {
    logMessage("Failed to send command");
  }
}

void MainWindow::sendHeartbeat() {
  if (!connected)
    return;

  DataMessage msg;
  msg.msgType = MSG_TYPE_HEARTBEAT;
  msg.sequence = static_cast<quint8>(sequenceNumber++);

  QString heartbeatData =
      QString("Heartbeat from Qt client %1").arg(msg.sequence);
  QByteArray dataBytes = heartbeatData.toUtf8();
  msg.length = static_cast<quint16>(qMin(dataBytes.size(), 1020));
  memcpy(msg.data, dataBytes.constData(), msg.length);

  QByteArray packet(reinterpret_cast<const char *>(&msg), sizeof(msg));

  udpSocket->writeDatagram(packet, serverAddress, serverPort);

  packetsSent++;
  bytesSent += static_cast<int>(packet.size());
  packetsSentLabel->setText(QString::number(packetsSent));
  bytesSentLabel->setText(QString::number(bytesSent));

  logMessage(QString("Sent heartbeat #%1").arg(msg.sequence));
}

void MainWindow::readPendingDatagrams() {
  while (udpSocket->hasPendingDatagrams()) {
    QNetworkDatagram datagram = udpSocket->receiveDatagram();
    processReceivedData(datagram.data(), datagram.senderAddress(),
                        datagram.senderPort());
  }
}

void MainWindow::processReceivedData(const QByteArray &data,
                                     const QHostAddress &sender, quint16 port) {
  if (data.size() < static_cast<int>(sizeof(DataMessage))) {
    logMessage("Received packet too small");
    return;
  }

  const DataMessage *msg =
      reinterpret_cast<const DataMessage *>(data.constData());

  packetsReceived++;
  bytesReceived += data.size();
  packetsReceivedLabel->setText(QString::number(packetsReceived));
  bytesReceivedLabel->setText(QString::number(bytesReceived));

  QString messageType;
  switch (msg->msgType) {
  case MSG_TYPE_DATA:
    messageType = "DATA";
    break;
  case MSG_TYPE_COMMAND:
    messageType = "COMMAND";
    break;
  case MSG_TYPE_RESPONSE:
    messageType = "RESPONSE";
    break;
  case MSG_TYPE_HEARTBEAT:
    messageType = "HEARTBEAT";
    break;
  default:
    messageType = "UNKNOWN";
    break;
  }

  QString receivedData =
      QString::fromUtf8(msg->data, static_cast<int>(msg->length));

  // Update Last Received Label if it's a data message
  if (msg->msgType == MSG_TYPE_DATA) {
    lastReceivedLabel->setText(receivedData);
  }

  logMessage(QString("Received %1 from %2:%3 (Seq:%4) - %5")
                 .arg(messageType)
                 .arg(sender.toString())
                 .arg(port)
                 .arg(msg->sequence)
                 .arg(receivedData));
}

void MainWindow::updateConnectionStatus() {
  // This could be used to check connection health
  // For now, it's a placeholder for future enhancements
}

void MainWindow::logMessage(const QString &message) {
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
  logTextEdit->append(QString("[%1] %2").arg(timestamp, message));

  // Auto-scroll to bottom
  QTextCursor cursor = logTextEdit->textCursor();
  cursor.movePosition(QTextCursor::End);
  logTextEdit->setTextCursor(cursor);
}
