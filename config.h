#ifndef CONFIG_H
#define CONFIG_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Set this to true for the master, false for the slave
#define IS_MASTER true
// #define IS_MASTER false

// Pin Definitions
#if defined IS_MASTER
  #define IS_MASTER true
  #define CSN_PIN 7
  #define CE_PIN 9
  #define IRQ_PIN 2
  #define ROLE "Master"
#else
  #define IS_MASTER false
  #define CSN_PIN 7
  #define CE_PIN 9
  #define RECEIVED_LED 4
  #define IRQ_PIN 2
  #define INPUT_CAPTURE_PIN 8
  #define ROLE "Slave"
#endif

// Shared Data Structure
struct Message {
    char messageType[5];
    int count;
    unsigned long masterCaptureTime;
    unsigned long slaveCaptureTime;
};

volatile bool interruptFlag = false;

#endif // CONFIG_H
