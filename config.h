// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "eeprom_config.h"

// Pin Definitions
#if defined IS_MASTER
  #define IS_MASTER true
  #define CSN_PIN 9
  #define CE_PIN 10
  #define IRQ_PIN 2
  #define ROLE "Master"
#else
  #define IS_MASTER false
  #define CSN_PIN 9
  #define CE_PIN 10
  #define RECEIVED_LED 4
  #define IRQ_PIN 2
  #define INPUT_CAPTURE_PIN 8
  #define ROLE "Slave"
#endif

// Shared Data Structure
struct Message {
    char messageType[5];
    int count;
    unsigned long int masterCaptureTime;
    unsigned long int  slaveCaptureTime;
};

enum class Stage {
      TCP,
      DATA,
      RESET,
      MSG
};

Stage stage=Stage::TCP;
#endif // CONFIG_H
