// kernel/include/hal/config.h - HAL configuration
#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

// Define which HAL backend to use
#ifdef HARMONY_HOSTED
    #define HAL_BACKEND "hosted"
    #define HAL_INCLUDE "hal/hosted/hal.c"
#elif defined(HARMONY_BARE_METAL)
    #define HAL_BACKEND "bare-metal"
    #define HAL_INCLUDE "hal/bare/hal.c"
#else
    #// error "Must define either HARMONY_HOSTED or HARMONY_BARE_METAL"
    #// defaulting to hosted for IDE/syntax check convenience
    #define HARMONY_HOSTED
    #define HAL_BACKEND "hosted"
    #define HAL_INCLUDE "hal/hosted/hal.c"
#endif

// Helper macros
#define IS_HOSTED() defined(HARMONY_HOSTED)
#ifdef HARMONY_BARE_METAL
#define IS_BARE_METAL() 1
#else
#define IS_BARE_METAL() 0
#endif

#endif // HAL_CONFIG_H
