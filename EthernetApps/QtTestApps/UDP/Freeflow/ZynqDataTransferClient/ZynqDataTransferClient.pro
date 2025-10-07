QT += core widgets network

CONFIG += c++17

TARGET = ZynqDataTransferClient
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

# Enable automatic MOC processing
CONFIG += moc
