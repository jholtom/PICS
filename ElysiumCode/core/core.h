
#ifndef _ELYSIUM_CORE_H_
#define _ELYSIUM_CORE_H_

#ifdef __FUZZ__
#define PERSIST
#else
#define PERSIST __attribute__((section(".persistent"))) 
#endif

#include <stdint.h>
#include <stdbool.h>

#include "hal.h"

#include "cfg.h"
#include "mych.h"

#endif
