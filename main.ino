#include "NRF24Radio.h"
#include "masterDevice.h"
#include "slaveDevice.h"

// Addresses for communication
const byte address[5] = "MYGPS";
const byte addressAck[5] = "YOGPS";

// Radio and device objects
NRF24Radio radio(CE_PIN, CSN_PIN);
#if IS_MASTER
MasterDevice master(radio);
#else
SlaveDevice slave(radio);
#endif

void setup() {
    delay(1000);
    Serial.begin(115200);
    radio.initialize(address, addressAck);
    Serial.print("Initialized as: ");
    Serial.println(ROLE);
    radio.printDetails();

    //Init for protocol
    Message message;
    radio.startListening();
    radio.handleProtocol(message);
}

void loop() {
#if IS_MASTER
    master.masterLoop();
    delay(1000); 
#else
    slave.slaveLoop();
#endif
}
