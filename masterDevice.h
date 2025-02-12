#include "Print.h"
#ifndef MASTERDEVICE_H
#define MASTERDEVICE_H

#include "NRF24Radio.h"

class MasterDevice {
private:
    NRF24Radio& radio;
    long int tcpCaptureTime;

public:
    MasterDevice(NRF24Radio& radio) : radio(radio) {}
    Message message;

    void masterLoop(){
      if(radio.interruptFlag){
        radio.interruptFlag=false;
        bool tx_ok, tx_fail, rx_ready;
        radio.radio.whatHappened(tx_ok, tx_fail, rx_ready);

        if (tx_ok) {
          radio.startListening();
          radio.txMicros=micros();
          // Serial.println("Master: Sent! ");
          // radio.instance->messageCount++;
          radio.timeOut();//start timeout counter

          if(stage==Stage::DATA){
            radio.instance->totalCapturetime=totalCaptureTime;
          }
        }

        if (tx_fail) {
          Serial.println("Master: Failed! ");
          stage=Stage::RESET;
          radio.handleProtocol(message);//try and send Again
        }

        if (rx_ready) { //got ack, read it then continue with protocol
          radio.rxMicros=micros();
          // Serial.println("Master: received! ");
          message=radio.receiveMessage();
          radio.instance->messageCount=message.count;
          //get capture time if slave replies with TCP phase
          //this is master->slave-> master 
          if(strcmp(message.messageType,"TCP")==0){
            //Reply TCP pkt with TCP pkt
            stage=Stage::TCP;
            //prepare to get capture time
            radio.instance->secondCaptureDone=false;
            radio.handleProtocol(message);
            return;
          }
          else if(strcmp(message.messageType,"DATA")==0){
            message.masterCaptureTime=radio.totalCapturetime;

            Serial.print("Capture Time: ");
            Serial.println(radio.instance->captureTime);
            Serial.print("Capture Time global: ");
            Serial.println(totalCaptureTime);
            Serial.print("Total capture Time: ");
            Serial.println(radio.totalCapturetime);
            Serial.print(" Time: ");
            Serial.print(message.slaveCaptureTime);
            Serial.print(" - ");
            Serial.print(message.masterCaptureTime);
            Serial.print(" Time: ");
            
            Serial.println(int(message.slaveCaptureTime)-int(message.masterCaptureTime));


            radio.handleProtocol(message);
            return;
          }
          

      }
    }

    if(radio.timeOutFlag){
      //ack not received
      radio.instance->timeOutFlag=false;
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
