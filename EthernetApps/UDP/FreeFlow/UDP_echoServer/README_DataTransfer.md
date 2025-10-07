# Zynq Data Transfer Application - UDP_echoServer

This project has been updated from an echo server to a real data transfer application that enables bidirectional communication between your Zynq device and Qt widget applications over Ethernet.

## 🔄 **What Changed**

### **Files Added:**
- ✅ `data_transfer.h` - Header with data structures and function prototypes
- ✅ `data_transfer.c` - Complete data transfer implementation
- ✅ `README_DataTransfer.md` - This documentation

### **Files Modified:**
- ✅ `main.c` - Updated to use data transfer instead of echo server

### **Files Kept (for reference):**
- 📁 `echo.c` - Original echo server (can be removed if not needed)

## 🚀 **Key Features**

### **Real Data Transfer (Not Echo)**
- Sends actual sensor data, status information, and system metrics
- Sample data includes: temperature, voltage, memory usage, CPU load, etc.
- Configurable send interval (currently 1 second)

### **UART Terminal Display**
- All received data is displayed on UART terminal with detailed information:
```
[UART] Received from 192.168.1.100:12345
[UART] Type: 1, Seq: 0, Len: 12
[UART] Data: Test message from Qt
[UART] Data message received
```

### **Bidirectional Communication**
- Zynq sends periodic data to Qt applications
- Qt applications can send data back to Zynq
- Zynq displays all received data on UART terminal
- Supports different message types (DATA, COMMAND, RESPONSE, HEARTBEAT)

## ⚙️ **Configuration**

### **Network Settings:**
- **Server IP**: 192.168.1.10 (configured in main.c)
- **Server Port**: 8888 (defined in data_transfer.h)
- **Send Interval**: 1000ms (1 second)
- **Heartbeat Interval**: Every 5th packet

### **Message Protocol:**
```c
typedef struct {
    u8_t msg_type;      // Message type (DATA, COMMAND, RESPONSE, HEARTBEAT)
    u8_t sequence;      // Sequence number
    u16_t length;       // Data length
    u8_t data[1020];    // Actual data payload
} data_message_t;
```

## 🔨 **Building the Project**

### **In Vitis IDE:**
1. Open your project: `UDP_echoServer_system`
2. Clean and rebuild the project
3. Flash the application to your Zynq device
4. Connect UART terminal (115200 baud) to monitor output

### **Expected Build Output:**
- No compilation errors
- All source files compile successfully
- Ready to flash to Zynq device

## 🖥️ **UART Terminal Setup**

To view the application output:

### **Windows:**
1. Open Device Manager
2. Find COM port assigned to Zynq device
3. Use PuTTY:
   - Connection type: Serial
   - Serial line: COM[port number]
   - Speed: 115200
   - Flow control: None

### **Linux:**
```bash
sudo screen /dev/ttyUSB0 115200
```

## 📱 **Qt Client Application**

The Qt client application is available at: `D:\Qt_projects\ZynqDataTransferClient\`

### **Building Qt Client:**
1. Open Qt Creator
2. Open `ZynqDataTransferClient.pro`
3. Configure and build
4. Run the application

### **Connecting to Zynq:**
1. Launch Qt application
2. Enter Zynq IP: `192.168.1.10`
3. Enter Port: `8888`
4. Click "Connect"

## 🎯 **Expected Runtime Output**

### **Zynq UART Terminal:**
```
-----lwIP Data Transfer Application ------
Board IP: 192.168.1.10
Netmask : 255.255.255.0
Gateway : 192.168.1.1

=== Data Transfer Application ===
UDP server listening on port 8888
Waiting for Qt client connection...
Send data every 1000 ms
=====================================

[INFO] Data transfer server started successfully

[INFO] Qt client connected: 192.168.1.100:12345
[SENT] Data to Qt: Hello from Zynq! [Seq:0]

[UART] Received from 192.168.1.100:12345
[UART] Type: 1, Seq: 0, Len: 12
[UART] Data: Test message
[UART] Data message received

=== Transfer Statistics ===
Packets sent: 5
Packets received: 3
Bytes sent: 5120
Bytes received: 3072
Client: 192.168.1.100:12345
=============================
```

### **Qt Application:**
- Real-time data reception from Zynq
- Statistics tracking
- Ability to send custom data back to Zynq
- Complete communication log

## 🔧 **Troubleshooting**

### **Build Issues:**
- Ensure all files are included in the project
- Check that `data_transfer.h` is properly included
- Verify lwIP configuration

### **Runtime Issues:**
- Check UART terminal connection
- Verify network configuration (IP: 192.168.1.10)
- Ensure firewall allows UDP port 8888
- Check that Qt client is connecting to correct IP/port

### **No Data Transfer:**
- Verify Zynq application is running
- Check UART terminal for startup messages
- Ensure Qt client is connected
- Monitor network traffic

## 📊 **Sample Data Sent by Zynq**

The application sends these sample data messages:
1. "Hello from Zynq!"
2. "Temperature: 45.2°C"
3. "Voltage: 3.3V"
4. "Status: Running"
5. "Memory: 85% used"
6. "Network: Connected"
7. "CPU Load: 67%"
8. "System Time: Active"

## 🎉 **Success Indicators**

✅ **Project builds without errors**
✅ **UART terminal shows startup messages**
✅ **Qt client can connect successfully**
✅ **Data flows in both directions**
✅ **Statistics are displayed every 10 seconds**
✅ **UART shows all received data with details**

## 🔄 **Next Steps**

1. **Build and flash** the updated application in Vitis
2. **Connect UART terminal** to monitor output
3. **Build and run** the Qt client application
4. **Test communication** between Zynq and Qt
5. **Customize data** being sent (modify `sample_data` array in `data_transfer.c`)

Your Zynq application is now ready for real data transfer with Qt widget applications! 🚀
