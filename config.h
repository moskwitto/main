// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Set this to true for the master, commented for the slave
#define IS_MASTER true

// Pin Definitions
#if defined IS_MASTER
  #define IS_MASTER true
  #define CSN_PIN 6
  #define CE_PIN 7
  #define IRQ_PIN 2
  #define ROLE "Master"
#else
  #define IS_MASTER false
  #define CSN_PIN 6 //7
  #define CE_PIN 7 //9
  #define RECEIVED_LED 4
  #define IRQ_PIN 2
  #define INPUT_CAPTURE_PIN 8
  #define ROLE "Slave"
#endif

// Shared Data Structure
struct Message {
    char messageType[5]="";
    int count;
    unsigned long masterCaptureTime;
    unsigned long slaveCaptureTime;
};

enum class Stage {
      TCP,
      DATA,
      RESET,
      MSG
};

Stage stage=Stage::TCP;
#endif // CONFIG_H
