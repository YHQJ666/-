#ifndef __KEY_EXTI_H
#define __KEY_EXTI_H

#include <stdbool.h>


typedef void (*key_press_callback_t)(void);


void key_exti_init(void);
void key_exti_register(key_press_callback_t callback);


#endif /* __KEY_EXTI_H */
