#ifndef SLAVEDEVICE_H
#define SLAVEDEVICE_H

#include "NRF24Radio.h"

class SlaveDevice {
private:
    NRF24Radio& radio;

public:
    SlaveDevice(NRF24Radio& radio) : radio(radio) {}

    void handleReceivedMessage() {
        if (radio.radio.available()) {
            Message dataReceived;
            radio.radio.read(&dataReceived, sizeof(dataReceived));

            Serial.print("Slave received message: ");
            Serial.println(dataReceived.messageType);

            // Respond back
            Message response;
            snprintf(response.messageType, sizeof(response.messageType), "ACK");
            response.count = dataReceived.count;

            radio.radio.stopListening();
            radio.radio.write(&response, sizeof(response));
            radio.radio.startListening();
        }
    }
};

#endif // SLAVEDEVICE_H
