#ifndef _LCD_INIT_H_
#define _LCD_INIT_H_

#include "spi.h"

/* ??LCD???? */
#define USE_HORIZONTIAL 1 /* 0?????? 1 ??180??? 2 ??90??? 3 ??270??? */

/* ??????? */
#if USE_HORIZONTIAL == 0 || USE_HORIZONTIAL == 1
#define LCD_W 142
#define LCD_H 428
#else
#define LCD_W 428
#define LCD_H 142
#endif



#define LCD_RES_GPIO_PIN GPIO_NUM_12
#define LCD_DC_GPIO_PIN GPIO_NUM_13
#define LCD_BLK_GPIO_PIN GPIO_NUM_21


#define LCD_RES_Set() gpio_set_level(LCD_RES_GPIO_PIN, 1)
#define LCD_RES_Clr() gpio_set_level(LCD_RES_GPIO_PIN, 0)

#define LCD_DC_Set() gpio_set_level(LCD_DC_GPIO_PIN, 1)
#define LCD_DC_Clr() gpio_set_level(LCD_DC_GPIO_PIN, 0)

#define LCD_BLK_Set() gpio_set_level(LCD_BLK_GPIO_PIN, 1)
#define LCD_BLK_Clr() gpio_set_level(LCD_BLK_GPIO_PIN, 0)


/* ??????? */
void LCD_GPIOInit(void);                                                           // ?????LCD???
void LCD_WR_REG(uint8_t reg);                                                      // LCD????????????
void LCD_WR_Byte(uint8_t dat);                                                     // LCD?????????????
void LCD_WR_HalfWord(uint16_t dat);                                                // LCD??????????????
void LCD_SetCursor(uint16_t x, uint16_t y);                                        // ??????????
void LCD_Address_Set(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);          // ???????????
void LCD_Fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color); // ????????
void LCD_FastFill(uint16_t color);                                                 // ?????????
void LCD_Enter_Sleep(void);                                                        // ???????????
void LCD_Exit_Sleep(void);                                                         // ??????????
void LCD_Init(void);                                                               // ????LCD?????
void LCD_WR_DATA_1(uint8_t *dat, int len);

/* ????????? */
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40      // ???
#define BRRED 0XFC07      // ????
#define GRAY 0X8430       // ???
#define DARKBLUE 0X01CF   // ?????
#define LIGHTBLUE 0X7D7C  // ????
#define GRAYBLUE 0X5458   // ?????
#define LIGHTGREEN 0X841F // ????
#define LGRAY 0XC618      // ????(PANNEL),???????
#define LGRAYBLUE 0XA651  // ??????(???????)
#define LBBLUE 0X2B12     // ??????(??????????)

#endif
