
#include "channels.h"

void elyChanSubscribe(uint8_t * buffer, uint8_t length, uint32_t interval) {
  (void)(buffer);
  (void)(length);
  (void)(interval);
}

void elyChanUnsubscribe(uint8_t * buffer, uint8_t length) {
  (void)(buffer);
  (void)(length);
}

void elyChanLog(uint8_t * buffer, uint8_t length, uint32_t interval) {
  (void)(buffer);
  (void)(length);
  (void)(interval);
}

void elyChanUnlog(uint8_t * buffer, uint8_t length) {
  (void)(buffer);
  (void)(length);
}

size_t elyChanGetValue(uint8_t * buffer, uint8_t id) {
  (void)(buffer);
  (void)(id);
  return 4;
}

void elyChanReset() {
}

