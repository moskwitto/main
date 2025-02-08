#include "NRF24Radio.h"
#include "masterDevice.h"
#include "slaveDevice.h"

// Radio and device objects
NRF24Radio radio(CE_PIN, CSN_PIN);
#if IS_MASTER
// Addresses for communication
const byte address[5] = "MYGPS";
const byte addressAck[5] = "YOGPS";
MasterDevice master(radio);
#else
// Addresses for communication
const byte addressAck[5] = "MYGPS";
const byte address[5] = "YOGPS";
SlaveDevice slave(radio);
#endif

void setup() {
    delay(1000);
    Serial.begin(115200);
    radio.initialize(address, addressAck);
    Serial.print("Initialized as: ");
    Serial.println(ROLE);
    radio.printDetails();
    if(strcmp(ROLE,"Slave")==0){
      radio.startListening();

    }
    else{
      //Init for protocol
      Message message;
      radio.handleProtocol(message);
    }
     
}

void loop() {
#if IS_MASTER
    master.masterLoop();
#else
    slave.slaveLoop();
#endif
}
