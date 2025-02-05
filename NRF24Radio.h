#ifndef NRF24RADIO_H
#define NRF24RADIO_H

#include "config.h"
#include <RF24.h>
#include <printf.h>

class NRF24Radio {
public:
    RF24 radio;
    volatile bool interruptFlag = false;       // Interrupt flag for nRF24 IRQ
    static NRF24Radio* instance;              // Static pointer to the instance

    // Variables for input capture
    volatile bool firstCaptureDone = false;   // Tracks capture state
    volatile unsigned int startCaptureTime = 0;
    volatile unsigned int endCaptureTime = 0;
    volatile unsigned int overflowCount = 0;
    volatile unsigned int captureTime = 0;
    volatile unsigned int captureTimeOVF = 0;
    unsigned long totalCapturetime;

    //timeout flag
    volatile bool timeOutFlag;

    int messageCount=0;
    unsigned long txMicros=0;
    unsigned long rxMicros = 0;

    // Variables for nRF24 IRQ handling
    volatile bool tx_ok = false, tx_fail = false, rx_ready = false;

    NRF24Radio(byte cePin, byte csnPin)
        : radio(cePin, csnPin) {}

    void initialize(const byte* address, const byte* addressAck) {
        radio.begin();
        radio.setChannel(75);                  // Set channel
        radio.setDataRate(RF24_1MBPS);         // Set data rate
        radio.setPALevel(RF24_PA_MIN);         // Set PA level
        //radio.setRetries(1, 0);                // Set auto retry: 250 µs delay, 0 retries
        radio.setAutoAck(false);
        radio.setPayloadSize(sizeof(Message)); // Set payload size
        radio.openWritingPipe(addressAck);        // Open writing pipe
        radio.openReadingPipe(0, address);  // Open reading pipe
        radio.startListening();                // Start listening
        radio.maskIRQ(0, 1, 0);                // Mask TX_DS and MAX_RT interrupts, enable RX_DR

        pinMode(IRQ_PIN, INPUT_PULLUP);
        instance = this; // Set static instance pointer
        attachInterrupt(digitalPinToInterrupt(IRQ_PIN), interruptRoutine, FALLING);

        setupInputCapture(); // Configure timer for input capture
    }

    void printDetails() {
        printf_begin();
        radio.printPrettyDetails();
    }

    void startListening(){
      radio.startListening();
    }

    void stopListening(){
      radio.stopListening();
    }

   Message receiveMessage(){
      Message dataReceived;
      if(radio.available()){
        radio.read(&dataReceived, sizeof(dataReceived));

        // Serial.print(F("{Message Type: "));
        // Serial.print(dataReceived.messageType);
        // Serial.print(F(", Count: "));
        // Serial.print(dataReceived.count);
        // Serial.print(F(", master Capture Time: "));
        // Serial.print(dataReceived.masterCaptureTime);
        // Serial.print(F(", Slave Capture Time: "));
        // Serial.print(dataReceived.slaveCaptureTime);
        // Serial.println(F("}"));
        
        return dataReceived;
      }
      else{
        Serial.print("..");
      }

      //not available; return NULL
      sprintf(dataReceived.messageType,sizeof(dataReceived.messageType),"NULL");
      return dataReceived;
    }

    // message: count, captureTime
    // messagetype is handled by handleprotocol()
    void sendMessage(Message message){
      radio.stopListening();
      Message dataToSend=message;
      dataToSend.count=messageCount;
      radio.startFastWrite(&dataToSend, sizeof(dataToSend), 0);
    }

    static void interruptRoutine() {
      instance->interruptFlag = true; // Set the instance-specific interrupt flag
    }

    void setupInputCapture() {
        noInterrupts(); // Disable interrupts during setup

        // Configure Timer1 for input capture, normal mode, no prescaler
        TCCR1A = 0;
        TCCR1B = (1 << CS10); // No prescaler, input capture on falling edge

        // Enable input capture interrupt and overflow interrupt
        TIMSK1 |= (1 << ICIE1); // Input capture interrupt
        TIMSK1 |= (1 << TOIE1); // Overflow interrupt

        interrupts(); // Enable interrupts
    }

    
    //  Protocol: 
    //  TCP=> gets master capture time
    //  DATA=> gets slave capture time
    //  RESET=> resets cycle incase of time out or a radio resets
    //  MSG=> used to send a general info message
    void handleProtocol(Message message) {
      switch (stage) {
        case Stage::TCP:
          snprintf(message.messageType, sizeof(message.messageType), "TCP");
          instance->sendMessage(message);
          // Serial.println("TCP");
          stage = Stage::DATA;
          // instance->timeOut();
          break;
        case Stage::DATA:
          snprintf(message.messageType, sizeof(message.messageType), "DATA");
          instance->sendMessage(message);
          // Serial.println("DATA");
          stage = Stage::TCP;
          // instance->timeOut();
          break;
        case Stage::RESET:
          //sprintf(message.messageType,sizeof(message.messageType),"RESET");
          strcpy(message.messageType,"RESET");
          instance->sendMessage(message);
          // Serial.println("RESET");
          stage = Stage::TCP;
          // instance->timeOut();
          break;
        case Stage::MSG:
          snprintf(message.messageType, sizeof(message.messageType), "MSG");
          instance->sendMessage(message);
          // Serial.println("MSG");
          stage = Stage::TCP;
          break;
        default:
          Serial.println("Invalid state");
      }
    }
  
    // Handle Timeout
    void timeOut() {
      int timeoutCounter = 0;
      radio.startListening();
      while (!radio.available() && timeoutCounter < 10) {
          delay(150);
          timeoutCounter++;
          if (timeoutCounter == 10) {
              instance->timeOutFlag = true;
              Serial.println(F("Time out"));
              break;
      }
    }

}

    static void timerInputCaptureISR() {
        if (instance) {
            static unsigned int startOverflowCount = 0;

            if (!instance->firstCaptureDone) {
                instance->startCaptureTime = ICR1;         // Capture start time
                startOverflowCount = instance->overflowCount; // Record overflow count
                instance->firstCaptureDone = true;
            } else {
                instance->endCaptureTime = ICR1; // Capture end time

                // Calculate total elapsed time
                instance->captureTime = instance->endCaptureTime - instance->startCaptureTime;
                instance->captureTimeOVF = instance->overflowCount - startOverflowCount ;
                

                instance->firstCaptureDone = false; // Reset for the next measurement
                // Serial.print("Capture Time: ");
                // Serial.println(instance->captureTime);
            }
        }
    }
};

// Define the static instance pointer
NRF24Radio* NRF24Radio::instance = nullptr;

// ISR for Timer1 Input Capture
ISR(TIMER1_CAPT_vect) {
    if (NRF24Radio::instance) {
        NRF24Radio::timerInputCaptureISR();
    }
}

// ISR for Timer1 Overflow (optional, if needed)
ISR(TIMER1_OVF_vect) {
    if (NRF24Radio::instance) {
        NRF24Radio::instance->overflowCount++;
    }
}

#endif // NRF24RADIO_H
