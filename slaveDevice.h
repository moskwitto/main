#ifndef SLAVEDEVICE_H
#define SLAVEDEVICE_H

#include "NRF24Radio.h"

class SlaveDevice {
private:
    NRF24Radio& radio;

public:
    SlaveDevice(NRF24Radio& radio) : radio(radio) {}
    
    void slaveLoop(){
      Message message;
      if(radio.interruptFlag){
        radio.interruptFlag=false;
        bool tx_ok, tx_fail, rx_ready;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
          radio.txMicros=micros();
          Serial.println("Slave: Sent! ");
          radio.startListening();//listen for ack
        }
        if (tx_fail) {
          Serial.println("Slave: Failed! ");
          stage=Stage::RESET;
          radio.handleProtocol(message);//try and send Again
        }
        if (rx_ready) { //got ack, read it then continue with protocol
          radio.rxMicros=micros();
          Serial.println("Slave: received! ");
          message=radio.receiveMessage();
          radio.handleProtocol(message);
          //get capture time if slave replies with TCP phase
          //this is master->slave-> master 
          if(strcmp(message.messageType,"DATA")){
            message.slaveCaptureTime=radio.captureTime;
          }
          else{
            Serial.println("waiting....");
          }

        }
    }

    if(radio.timeOutFlag){
      //ack not received
      stage=Stage::RESET;
      radio.handleProtocol(message);
    }

    }

};

#endif // SLAVEDEVICE_H
