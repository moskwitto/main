#ifndef NRF24RADIO_H
#define NRF24RADIO_H

#include "config.h"
#include <RF24.h>
#include <printf.h>

volatile bool armed=false;
volatile bool finished=true; //read only outside ISR!!!!
volatile unsigned int overFlowCount; //read only outside ISR!!!!
volatile unsigned int startOverFlowCount; //read only outside ISR!!!!
volatile unsigned int captureTime1; //read only outside ISR!!!!
volatile unsigned int captureTime2; //read only outside ISR!!!!
volatile unsigned long totalCaptureTime;


class NRF24Radio {
public:
    RF24 radio;
    volatile bool interruptFlag = false;       // Interrupt flag for nRF24 IRQ
    static NRF24Radio* instance;              // Static pointer to the instance

    // Variables for input capture
    volatile bool firstCaptureDone = false;   // Tracks capture state
    volatile bool secondCaptureDone =false;
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
        while(!radio.begin()) //wait till radio is initialised
        radio.setChannel(78);                  // Set channel
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
        
        delay(100);
        Serial.print(F("{Message Type: "));
        Serial.print(dataReceived.messageType);
        Serial.print(F(", Count: "));
        Serial.print(dataReceived.count);
        Serial.print(F(", master Capture Time: "));
        Serial.print(dataReceived.masterCaptureTime);
        Serial.print(F(", Slave Capture Time: "));
        Serial.print(dataReceived.slaveCaptureTime);
        Serial.println(F("}"));
        
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
      dataToSend.slaveCaptureTime=message.slaveCaptureTime;
      dataToSend.masterCaptureTime=message.masterCaptureTime;
      dataToSend.count=messageCount;
      radio.startFastWrite(&dataToSend, sizeof(dataToSend), 0);

      // Serial.print(F("{Message Type: "));
      // Serial.print(dataToSend.messageType);
      // Serial.print(F(", Count: "));
      // Serial.print(dataToSend.count);
      // Serial.print(F(", master Capture Time: "));
      // Serial.print(dataToSend.masterCaptureTime);
      // Serial.print(F(", Slave Capture Time: "));
      // Serial.print(dataToSend.slaveCaptureTime);
      // Serial.println(F("}"));
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
          //In TCP packets, capture times should be 0
          message.masterCaptureTime=0;
          message.slaveCaptureTime=0;
          instance->sendMessage(message);
          stage = Stage::DATA; //set next stage to send data pkt
          break;
        case Stage::DATA:
          snprintf(message.messageType, sizeof(message.messageType), "DATA");
          instance->sendMessage(message);
          //After sending DATA pkt, reset capture flag to prepare for fresh capture time
          Serial.println("Arming!");
          Serial.println(overFlowCount);
          Serial.println(captureTime1);
          Serial.println(captureTime2);
          armed=true;
          stage = Stage::TCP;
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

}};

   // ISR for Timer1 Input Capture
   ISR(TIMER1_CAPT_vect) {
        if (!armed && !finished)
            {
              captureTime2 = ICR1;
              startOverFlowCount = overFlowCount - startOverFlowCount;
              finished=true;
              totalCaptureTime =((unsigned long)startOverFlowCount)<<16;
              totalCaptureTime += captureTime2 - captureTime1;
            }
       
        if (armed)
            {
              captureTime1 = ICR1;
              startOverFlowCount = overFlowCount;
              armed=false;
              finished=false;
            }
       
 
        }

    
// Define the static instance pointer
NRF24Radio* NRF24Radio::instance = nullptr;

// ISR for Timer1 Overflow (optional, if needed)
ISR(TIMER1_OVF_vect) {
    overFlowCount++;
    
}

#endif // NRF24RADIO_H
