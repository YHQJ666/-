#ifndef __LED_H
#define __LED_H


#include <stdbool.h>


void led_init(void);
void led_set(bool on);
void led_on(void);
void led_off(void);
void led_toggle(void);
void led2_init(void);
void led2_set(bool on);
void led2_on(void);
void led2_off(void);
void led2_toggle(void);


#endif /* __LED_H */
