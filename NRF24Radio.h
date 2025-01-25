#ifndef NRF24RADIO_H
#define NRF24RADIO_H

#include "config.h"
#include <printf.h>

class NRF24Radio {
public:
    RF24 radio;

    NRF24Radio(byte cePin, byte csnPin) : radio(cePin, csnPin) {}

    void initialize(const byte* address, const byte* addressAck) {
        radio.begin();
        radio.setChannel(75);                    // ~2400 MHz
        radio.setDataRate(RF24_1MBPS);          // 1 Mbps
        radio.setPALevel(RF24_PA_MIN);          // PA_MIN
        radio.setRetries(1, 0);                 // 250 µs delay, 0 retries
        radio.setPayloadSize(sizeof(Message)); // Match payload size
        radio.openWritingPipe(address);         // Writing pipe
        radio.openReadingPipe(1, addressAck);   // Reading pipe
        radio.startListening();
        radio.maskIRQ(0,1,0);

        pinMode(IRQ_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(IRQ_PIN), interruptRoutine, FALLING);
    }

    void printDetails() {
        printf_begin();
        radio.printDetails();
    }

    void interruptRoutine() {
      interruptFlag = true;
    }
};

#endif // NRF24RADIO_H
