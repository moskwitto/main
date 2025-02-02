#include "Print.h"
#ifndef MASTERDEVICE_H
#define MASTERDEVICE_H

#include "NRF24Radio.h"

class MasterDevice {
private:
    NRF24Radio& radio;

public:
    MasterDevice(NRF24Radio& radio) : radio(radio) {}

    void masterLoop(){
      Message message;

      if(radio.interruptFlag){
        radio.interruptFlag=false;
        bool tx_ok, tx_fail, rx_ready;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
          radio.txMicros=micros();
          Serial.println("Master: Sent! ");
          radio.startListening();//listen for ack
          radio.instance->messageCount++;
          radio.timeOut();//start timeout counter
        }
        if (tx_fail) {
          Serial.println("Master: Failed! ");
          stage=Stage::RESET;
          radio.handleProtocol(message);//try and send Again
        }
        if (rx_ready) { //got ack, read it then continue with protocol
          radio.rxMicros=micros();
          Serial.println("Master: received! ");
          message=radio.receiveMessage();
          radio.handleProtocol(message);
          //get capture time if slave replies with TCP phase
          //this is master->slave-> master 
          if(strcmp(message.messageType,"TCP")){
            message.masterCaptureTime=radio.captureTime;
          }
          else{
            Serial.println("waiting....");
          }

      }
    }

    if(radio.timeOutFlag){
      //ack not received
      radio.timeOutFlag=false;
      stage=Stage::RESET;
      radio.handleProtocol(message);
    }

    }

    void receiveMessage(){
      //radio.startListening();
      radio.receiveMessage();
    }
};

#endif // MASTERDEVICE_H
