#ifndef MASTERDEVICE_H
#define MASTERDEVICE_H

#include "NRF24Radio.h"

class MasterDevice {
private:
    NRF24Radio& radio;
    unsigned int messageCount = 0;

public:
    MasterDevice(NRF24Radio& radio) : radio(radio) {}

    void handleProtocol() {
        Message dataToSend;
        snprintf(dataToSend.messageType, sizeof(dataToSend.messageType), "DATA");
        dataToSend.count = messageCount++;
        dataToSend.masterCaptureTime = micros();

        radio.radio.stopListening();
        radio.radio.write(&dataToSend, sizeof(dataToSend));
        radio.radio.startListening();

        Serial.println("Master sent a message.");
    }
};

#endif // MASTERDEVICE_H
