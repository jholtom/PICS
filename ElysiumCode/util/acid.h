
#ifndef _ACID_H_
#define _ACID_H_

#include "stdint.h"
#include "string.h"

#define _ACID_START_TRANSACTION(name) \
  name.shadow = &name.storage[(name.true == &name.storage[0] ? 1 : 0)]; \
  name.shadow_read = name.shadow; \
  memcpy(name.shadow, name.true, sizeof(*name.shadow))

#define _ACID_COMMIT(name) \
  name.true = name.shadow

#define _ACID_REVERT(name) \
  name.shadow = name.true; \
  name.shadow_read = name.shadow

#define _ACID_READ(name) \
  (*name.shadow_read)

#define _ACID_READ_FIELD(name, field) \
  (*name.shadow_read).field

#define _ACID_WRITE(name, ...) \
  (*name.shadow) = __VA_ARGS__

#define _ACID_WRITE_FIELD(name, field, ...) \
  (*name.shadow).field = __VA_ARGS__

#if defined(ACID_DEBUG)
  
  #include "assert.h"
  enum {
    ACID_TRANS_STARTED = 0x01
  } ACID_FLAGS;
  
  #define ACID_VAR_DECL(T, name, ...) \
    __attribute__((persistent)) struct { \
      T storage[2]; \
      const T * true; \
      const T * shadow_read; \
      T * shadow; \
      uint8_t flags; \
    } name = { {__VA_ARGS__, __VA_ARGS__}, &name.storage[0], &name.storage[1], &name.storage[1], 0}
  
  #define ACID_START_TRANSACTION(name) \
    name.flags |= ACID_TRANS_STARTED; \
    _ACID_START_TRANSACTION(name)

  #define ACID_COMMIT(name) \
    assert(name.flags & ACID_TRANS_STARTED); \
    _ACID_COMMIT(name); \
    name.flags &= ~ACID_TRANS_STARTED

  #define ACID_REVERT(name) \
    assert(name.flags & ACID_TRANS_STARTED); \
    _ACID_REVERT(name); \
    name.flags &= ~ACID_TRANS_STARTED

  #define ACID_READ(name) \
    _ACID_READ(name)

  #define ACID_READ_FIELD(name, field) \
    _ACID_READ_FIELD(name, field)

  #define ACID_WRITE(name, ...) \
    assert(name.flags & ACID_TRANS_STARTED); \
    _ACID_WRITE(name, __VA_ARGS__)

  #define ACID_WRITE_FIELD(name, field, ...) \
    assert(name.flags & ACID_TRANS_STARTED); \
    _ACID_WRITE_FIELD(name, field, __VA_ARGS__)
  
#else

  #define ACID_VAR_DECL(T, name, ...) \
    __attribute__((persistent)) struct { \
      T storage[2]; \
      const T * true; \
      const T * shadow_read; \
      T * shadow; \
    } name = { {__VA_ARGS__, __VA_ARGS__}, &name.storage[0], &name.storage[1], &name.storage[1]}

  #define ACID_START_TRANSACTION(name) _ACID_START_TRANSACTION(name)
  #define ACID_COMMIT(name) _ACID_COMMIT(name)
  #define ACID_REVERT(name) _ACID_REVERT(name)
  #define ACID_READ(name) _ACID_READ(name)
  #define ACID_READ_FIELD(name, field) _ACID_READ_FIELD(name, field)
  #define ACID_WRITE(name, ...) _ACID_WRITE(name, __VA_ARGS__)
  #define ACID_WRITE_FIELD(name, field, ...) _ACID_WRITE_FIELD(name, field, __VA_ARGS__)
  
#endif

#endif
