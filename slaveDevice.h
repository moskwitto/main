#ifndef SLAVEDEVICE_H
#define SLAVEDEVICE_H

#include "NRF24Radio.h"

class SlaveDevice {
private:
    NRF24Radio& radio;
    long int tcpCaptureTime;

public:
    SlaveDevice(NRF24Radio& radio) : radio(radio) {}
    Message message;
    
    void slaveLoop(){
      if(radio.interruptFlag){
        radio.interruptFlag=false;
        bool tx_ok, tx_fail, rx_ready;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
          radio.txMicros=micros();
          // Serial.println("Slave: Sent! ");
          radio.instance->messageCount++;
          radio.instance->timeOut();
          radio.startListening();
        }


        if (tx_fail) {
          Serial.println("Slave: Failed! ");
          stage=Stage::RESET;
          radio.handleProtocol(message); //send Again
          return;
        }

        if (rx_ready) { //got ack, read it then continue with protocol
          radio.rxMicros=micros();
          // Serial.println("Slave: received! ");
          message=radio.receiveMessage();
          //get capture time if slave replies with TCP phase
          //this is master->slave-> master 
          if(strcmp(message.messageType,"DATA")==0){
            message.slaveCaptureTime=radio.instance->totalCapturetime;

            delay(100);
            
            Serial.print("Capture Time: ");
            Serial.println(radio.instance->captureTime);
            Serial.print("Capture TimeOVF: ");
            Serial.println(radio.instance->captureTimeOVF);
            Serial.print("Total capture Time Global ");
            Serial.println(totalCaptureTime);
            Serial.print("Time: ");
            Serial.println(message.slaveCaptureTime-message.masterCaptureTime);
            radio.handleProtocol(message);
            return;

          }

          if(strcmp(message.messageType,"RESET")==0){
            radio.handleProtocol(message);
            return;
          }
          
          //TCP stage proceeds
          radio.instance->totalCapturetime=totalCaptureTime;

          Serial.print("After TCP: ");
          Serial.println(totalCaptureTime);
          radio.handleProtocol(message);
          
        }
    }

    if(radio.instance->timeOutFlag){
      //ack not received
      stage=Stage::RESET;
      radio.instance->timeOutFlag=false;
      radio.handleProtocol(message);
    }
    }
};

#endif // SLAVEDEVICE_H
