
#ifndef _ELYSIUM_SLIP_H_
#define _ELYSIUM_SLIP_H_

typedef enum {
  SLIP_ESC = 0xDB, /* octal 333 */
  SLIP_END = 0xC0, /* octal 300 */
  SLIP_ESC_END = 0xDC, /* octal 334 */
  SLIP_ESC_ESC = 0xDD, /* octal 335 */
} slip_constants_t;

#endif
