
#include "cmds.h"

msg_t elyMainMBPost(uint8_t * buffer, systime_t timeout) {
  return elyUARTPost(buffer, timeout);
}

msg_t elyMainMBPostI(uint8_t * buffer) {
  return elyUARTPostI(buffer);
}

