#ifndef __H__
#define __H__


#include <stdbool.h>
#include <stdint.h>


typedef void (*lcd_send_finish_callback_t)(void);


void lcd_init(void);
void lcd_write(uint8_t *data, uint16_t length);
void lcd_write_async(uint8_t *data, uint16_t length);
void lcd_send_finish_register(lcd_send_finish_callback_t callback);


// ================ 基础颜色 ================
#define BLACK     0x0000  // 黑色 (0, 0, 0)
#define WHITE     0xFFFF  // 白色 (255, 255, 255)
#define RED       0xF800  // 红色 (255, 0, 0)
#define GREEN     0x07E0  // 绿色 (0, 255, 0)
#define BLUE      0x001F  // 蓝色 (0, 0, 255)
#define YELLOW    0xFFE0  // 黄色 (255, 255, 0)
#define MAGENTA   0xF81F  // 品红 (255, 0, 255)
#define CYAN      0x07FF  // 青色 (0, 255, 255)
#define GRAY      0x8410  // 灰色 (128, 128, 128)
#define BROWN     0xA145  // 棕色 (165,42,42)
#define BRRED     0xFC07  // 亮红色 (255,51,51)

#endif /* __H__ */
