#ifndef LED_INTERFACE_H
#define LED_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include <posix_queue.h>

/**
 * @brief 
 * 
 */
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Set)(void *object, uint8_t state);
} LED_Interface;

/**
 * @brief 
 * 
 * @param object 
 * @param argv 
 * @param led 
 * @return true 
 * @return false 
 */
bool LED_Run(void *object, POSIX_Queue *posix_queue, LED_Interface *led);

#endif /* LED_INTERFACE_H */
