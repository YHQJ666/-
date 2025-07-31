#ifndef __KEY_H
#define __KEY_H

#include <stdbool.h>

void key_init(void);
bool key_read(void);
void key_wait_release(void);


#endif /* __KEY_H */
