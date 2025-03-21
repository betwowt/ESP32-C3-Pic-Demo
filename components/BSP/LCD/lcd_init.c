#include "lcd_init.h"

/* SPI句柄 */
spi_device_handle_t LCD_Handle;

/* 定义LCD最大传输字节数 */
#define LCD_MAX_BUF_SIZE (LCD_W * LCD_H * 2)

/* 定义一次连续写入的字节数 */
#define LCD_BUF_SIZE 15194

/* 定义帧缓存 牺牲空间换时间 */
uint8_t LCD_Buf[LCD_MAX_BUF_SIZE];

/**
 * @brief       液晶端口初始化
 * @param       无
 * @retval      无
 */
void LCD_GPIOInit(void)
{
    gpio_config_t gpio_init_struct = {0};
    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;           /* 失能引脚中断 */
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;           /* 输入输出模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;         /* 使能上拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;    /* 失能下拉 */
    gpio_init_struct.pin_bit_mask = 1ull << LCD_RES_GPIO_PIN; /* 设置的引脚的位掩码 */
    gpio_config(&gpio_init_struct);                           /* 配置GPIO */

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;          /* 失能引脚中断 */
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;          /* 输入输出模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;        /* 使能上拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;   /* 失能下拉 */
    gpio_init_struct.pin_bit_mask = 1ull << LCD_DC_GPIO_PIN; /* 设置的引脚的位掩码 */
    gpio_config(&gpio_init_struct);                          /* 配置GPIO */

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;           /* 失能引脚中断 */
    gpio_init_struct.mode = GPIO_MODE_INPUT_OUTPUT;           /* 输入输出模式 */
    gpio_init_struct.pull_up_en = GPIO_PULLUP_ENABLE;         /* 使能上拉 */
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;    /* 失能下拉 */
    gpio_init_struct.pin_bit_mask = 1ull << LCD_BLK_GPIO_PIN; /* 设置的引脚的位掩码 */
    gpio_config(&gpio_init_struct);                           /* 配置GPIO */
}

/**
 * @brief       向液晶写寄存器命令
 * @param       reg: 要写的命令
 * @retval      无
 */
void LCD_WR_REG(uint8_t reg)
{
    LCD_DC_Clr();
    BSP_SPI_WR_Bus(LCD_Handle, reg);
    LCD_DC_Set();
}

/**
 * @brief       向液晶写一个字节数据
 * @param       dat: 要写的数据
 * @retval      无
 */
void LCD_WR_Byte(uint8_t dat)
{
    LCD_DC_Set();
    BSP_SPI_WR_Bus(LCD_Handle, dat);
    LCD_DC_Set();
}


/**
 * @brief       向液晶写一个半字数据
 * @param       dat: 要写的数据
 * @retval      无
 */
void LCD_WR_HalfWord(uint16_t dat)
{
    LCD_DC_Set();
    BSP_SPI_WR_Bus(LCD_Handle, dat >> 8);
    BSP_SPI_WR_Bus(LCD_Handle, dat & 0xFF);
    LCD_DC_Set();
}

/**
 * @brief       向液晶连续写入len个字节数据
 * @param       dat: 要写的数据地址
 * @param       len：要写入字节长度
 * @retval      无
 */
void LCD_WR_DATA(const uint8_t *dat, int len)
{
    LCD_DC_Set();
    BSP_SPI_Write_Data(LCD_Handle, dat, len);
    LCD_DC_Set();
}

void LCD_WR_DATA_1(uint8_t *dat, int len)
{
    LCD_DC_Set();
    BSP_SPI_Write_Data(LCD_Handle, dat, len);
    LCD_DC_Set();
}

/**
 * @brief       液晶进入休眠
 * @param       无
 * @retval      无
 */
void LCD_Enter_Sleep(void)
{
    LCD_WR_REG(0x28);
    vTaskDelay(120 / portTICK_PERIOD_MS);
    LCD_WR_REG(0x10);
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

/**
 * @brief       液晶退出休眠
 * @param       无
 * @retval      无
 */
void LCD_Exit_Sleep(void)
{
    LCD_WR_REG(0x11);
    vTaskDelay(120 / portTICK_PERIOD_MS);
    LCD_WR_REG(0x29);
}


/**
 * @brief       设置显示窗口
 * @param       xs:窗口列起始地址
 * @param       ys:坐标行起始地址
 * @param       xe:窗口列结束地址
 * @param       ye:坐标行结束地址
 * @retval      无
 */
void LCD_Address_Set(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    #if USE_HORIZONTIAL==0
    LCD_WR_REG(0x2a); /* 列地址设置 */
    LCD_WR_HalfWord(xs+0x0C);
    LCD_WR_HalfWord(xe+0x0C);
    LCD_WR_REG(0x2b); /* 行地址设置 */
    LCD_WR_HalfWord(ys);
    LCD_WR_HalfWord(ye);
    LCD_WR_REG(0x2c); /* 储存器写 */
    #elif USE_HORIZONTIAL==1
    LCD_WR_REG(0x2a); /* 列地址设置 */
    LCD_WR_HalfWord(xs+0x0E);
    LCD_WR_HalfWord(xe+0x0E);
    LCD_WR_REG(0x2b); /* 行地址设置 */
    LCD_WR_HalfWord(ys);
    LCD_WR_HalfWord(ye);
    LCD_WR_REG(0x2c); /* 储存器写 */
    #elif USE_HORIZONTIAL==2
    LCD_WR_REG(0x2a); /* 列地址设置 */
    LCD_WR_HalfWord(xs);
    LCD_WR_HalfWord(xe);
    LCD_WR_REG(0x2b); /* 行地址设置 */
    LCD_WR_HalfWord(ys+0x0E);
    LCD_WR_HalfWord(ye+0x0E);
    LCD_WR_REG(0x2c); /* 储存器写 */
    #else
    LCD_WR_REG(0x2a); /* 列地址设置 */
    LCD_WR_HalfWord(xs);
    LCD_WR_HalfWord(xe);
    LCD_WR_REG(0x2b); /* 行地址设置 */
    LCD_WR_HalfWord(ys+0x0C);
    LCD_WR_HalfWord(ye+0x0C);
    LCD_WR_REG(0x2c); /* 储存器写 */
    #endif
}

/**
 * @brief       指定颜色填充区域
 * @param       xs:填充区域列起始地址
 * @param       ys:填充区域行起始地址
 * @param       xe:填充区域列结束地址
 * @param       ye:填充区域行结束地址
 * @param       color:填充颜色值
 * @retval      无
 */
void LCD_Fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint16_t i, j;
    LCD_Address_Set(xs, ys, xe - 1, ye - 1);
    for (j = ys; j < ye; j++)
    {
        for (i = xs; i < xe; i++)
        {
            LCD_WR_HalfWord(color);
        }
    }
}

/**
 * @brief       快刷清屏区域
 * @param       color:填充颜色值
 * @retval      无
 */
void LCD_FastFill(uint16_t color)
{
    uint16_t i, j;
    uint8_t data[2] = {0};
    data[0] = color >> 8;
    data[1] = color;
    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    for (j = 0; j < LCD_BUF_SIZE / 2; j++)
    {
        LCD_Buf[j * 2] = data[0];
        LCD_Buf[j * 2 + 1] = data[1];
    }
    for (i = 0; i < (LCD_MAX_BUF_SIZE / LCD_BUF_SIZE); i++)
    {
        LCD_WR_DATA(LCD_Buf, LCD_BUF_SIZE);
    }
}

/**
 * @brief       初始化LCD
 * @param       无
 * @retval      无
 */
void LCD_Init(void)
{
    esp_err_t ret = 0;
    /* 初始化SPI总线 */
    BSP_SPI_GPIOInit();
    /* SPI驱动接口配置 */
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 50 * 1000 * 1000,  /* SPI时钟 */
        .mode = 3,                           /* SPI模式0 */
        .spics_io_num = BSP_SPI_CS_GPIO_PIN, /* SPI设备引脚 */
        .queue_size = 7,                     /* 事务队列尺寸 7个 */
    };
    /* 添加SPI总线设备 */
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &LCD_Handle); /* 配置SPI总线设备 */
    ESP_ERROR_CHECK(ret);ESP_ERROR_CHECK(ret);
    LCD_GPIOInit();
    LCD_RES_Set();
    vTaskDelay(50 / portTICK_PERIOD_MS);
    LCD_RES_Clr();
    vTaskDelay(50 / portTICK_PERIOD_MS);
    LCD_RES_Set();
    vTaskDelay(120 / portTICK_PERIOD_MS);
    LCD_BLK_Set();
    LCD_WR_REG(0x11);   
    vTaskDelay(120 / portTICK_PERIOD_MS);           
    LCD_WR_REG(0xff);
    LCD_WR_Byte(0xa5);
    LCD_WR_REG(0x9a);
    LCD_WR_Byte(0x08);
    LCD_WR_REG(0x9b);
    LCD_WR_Byte(0x08);
    LCD_WR_REG(0x9c);
    LCD_WR_Byte(0xb0);
    LCD_WR_REG(0x9d);
    LCD_WR_Byte(0x17);
    LCD_WR_REG(0x9e);
    LCD_WR_Byte(0xc2);
    LCD_WR_REG(0x8f);
    LCD_WR_Byte(0x22);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0x84);
    LCD_WR_Byte(0x90);
    LCD_WR_REG(0x83);
    LCD_WR_Byte(0x7B);
    LCD_WR_REG(0x85);
    LCD_WR_Byte(0x4F);
    ////GAMMA---------------------------------/////////////
    //V0[3:0]
    LCD_WR_REG(0x6e);
    LCD_WR_Byte(0x0f);
    LCD_WR_REG(0x7e);
    LCD_WR_Byte(0x0f);

    //V63[3:0]
    LCD_WR_REG(0x60);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x70);
    LCD_WR_Byte(0x00);
    //V1[5:0] 
    LCD_WR_REG(0x6d);
    LCD_WR_Byte(0x39);
    LCD_WR_REG(0x7d);
    LCD_WR_Byte(0x31);
    //V62[5:0]
    LCD_WR_REG(0x61);
    LCD_WR_Byte(0x0A);
    LCD_WR_REG(0x71);
    LCD_WR_Byte(0x0A);
    //V2[5:0]
    LCD_WR_REG(0x6c);
    LCD_WR_Byte(0x35);
    LCD_WR_REG(0x7c);
    LCD_WR_Byte(0x29);
    //V61[5:0]
    LCD_WR_REG(0x62);
    LCD_WR_Byte(0x0F);
    LCD_WR_REG(0x72);
    LCD_WR_Byte(0x0F);
    //V20[6:0]
    LCD_WR_REG(0x68);
    LCD_WR_Byte(0x4f);
    LCD_WR_REG(0x78);
    LCD_WR_Byte(0x45);
    //V43[6:0]
    LCD_WR_REG(0x66);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0x76);
    LCD_WR_Byte(0x33);
    //V4[4:0]
    LCD_WR_REG(0x6b);
    LCD_WR_Byte(0x14);
    LCD_WR_REG(0x7b);
    LCD_WR_Byte(0x14);
    //V59[4:0]
    LCD_WR_REG(0x63);
    LCD_WR_Byte(0x09);
    LCD_WR_REG(0x73);
    LCD_WR_Byte(0x09);
    //V6[4:0]
    LCD_WR_REG(0x6a);
    LCD_WR_Byte(0x13);
    LCD_WR_REG(0x7a);
    LCD_WR_Byte(0x16);
    //V57[4:0]
    LCD_WR_REG(0x64);
    LCD_WR_Byte(0x08);
    LCD_WR_REG(0x74);
    LCD_WR_Byte(0x08);
    LCD_WR_REG(0x69);
    LCD_WR_Byte(0x07);
    LCD_WR_REG(0x79);
    LCD_WR_Byte(0x0d);
    LCD_WR_REG(0x65);
    LCD_WR_Byte(0x05);
    LCD_WR_REG(0x75);
    LCD_WR_Byte(0x05);
    LCD_WR_REG(0x67);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0x77);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0x6f);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x7f);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x50);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x52);
    LCD_WR_Byte(0xd6);
    LCD_WR_REG(0x53);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0x54);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0x55);
    LCD_WR_Byte(0x1b);
    LCD_WR_REG(0x56);
    LCD_WR_Byte(0x1b);
    LCD_WR_REG(0xa0);
    LCD_WR_Byte(0x2a);
    LCD_WR_Byte(0x24);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0xa1);
    LCD_WR_Byte(0x84);
    LCD_WR_REG(0xa2);
    LCD_WR_Byte(0x85);
    LCD_WR_REG(0xa8);
    LCD_WR_Byte(0x34);
    LCD_WR_REG(0xa9);
    LCD_WR_Byte(0x80);
    LCD_WR_REG(0xaa);
    LCD_WR_Byte(0x73);
    LCD_WR_REG(0xAB);
    LCD_WR_Byte(0x03);
    LCD_WR_Byte(0x61);
    LCD_WR_REG(0xAC);
    LCD_WR_Byte(0x03);
    LCD_WR_Byte(0x65);
    LCD_WR_REG(0xAD);
    LCD_WR_Byte(0x03);
    LCD_WR_Byte(0x60);
    LCD_WR_REG(0xAE);
    LCD_WR_Byte(0x03);
    LCD_WR_Byte(0x64);
    LCD_WR_REG(0xB9);
    LCD_WR_Byte(0x82);
    LCD_WR_REG(0xBA);
    LCD_WR_Byte(0x83);
    LCD_WR_REG(0xBB);
    LCD_WR_Byte(0x80);
    LCD_WR_REG(0xBC);
    LCD_WR_Byte(0x81);
    LCD_WR_REG(0xBD);
    LCD_WR_Byte(0x02);
    LCD_WR_REG(0xBE);
    LCD_WR_Byte(0x01);
    LCD_WR_REG(0xBF);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0xC0);
    LCD_WR_Byte(0x03);
    LCD_WR_REG(0xc4);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0xc5);
    LCD_WR_Byte(0x80);
    LCD_WR_REG(0xc6);
    LCD_WR_Byte(0x73);
    LCD_WR_REG(0xc7);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0xC8);
    LCD_WR_Byte(0x33);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0xC9);
    LCD_WR_Byte(0x5b);
    LCD_WR_REG(0xCA);
    LCD_WR_Byte(0x5a);
    LCD_WR_REG(0xCB);
    LCD_WR_Byte(0x5d);
    LCD_WR_REG(0xCC);
    LCD_WR_Byte(0x5c);
    LCD_WR_REG(0xCD);
    LCD_WR_Byte(0x33);
    LCD_WR_Byte(0x33);
    LCD_WR_REG(0xCE);
    LCD_WR_Byte(0x5f);
    LCD_WR_REG(0xCF);
    LCD_WR_Byte(0x5e);
    LCD_WR_REG(0xD0);
    LCD_WR_Byte(0x61);
    LCD_WR_REG(0xD1);
    LCD_WR_Byte(0x60);
    LCD_WR_REG(0xB0);
    LCD_WR_Byte(0x3a);
    LCD_WR_Byte(0x3a);
    LCD_WR_Byte(0x00);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0xB6);
    LCD_WR_Byte(0x32);
    LCD_WR_REG(0xB7);
    LCD_WR_Byte(0x80);
    LCD_WR_REG(0xB8);
    LCD_WR_Byte(0x73);
    LCD_WR_REG(0xe0);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0xe1);
    LCD_WR_Byte(0x03);
    LCD_WR_Byte(0x0f);
    LCD_WR_REG(0xe2);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0xe3);
    LCD_WR_Byte(0x01);
    LCD_WR_REG(0xe4);
    LCD_WR_Byte(0x0e);
    LCD_WR_REG(0xe5);
    LCD_WR_Byte(0x01);
    LCD_WR_REG(0xe6);
    LCD_WR_Byte(0x19);
    LCD_WR_REG(0xe7);
    LCD_WR_Byte(0x10);
    LCD_WR_REG(0xe8);
    LCD_WR_Byte(0x10);
    LCD_WR_REG(0xe9);
    LCD_WR_Byte(0x21);
    LCD_WR_REG(0xea);
    LCD_WR_Byte(0x12);
    LCD_WR_REG(0xeb);
    LCD_WR_Byte(0xd0);
    LCD_WR_REG(0xec);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0xed);
    LCD_WR_Byte(0x07);
    LCD_WR_REG(0xee);
    LCD_WR_Byte(0x07);
    LCD_WR_REG(0xef);
    LCD_WR_Byte(0x09);
    LCD_WR_REG(0xF0);
    LCD_WR_Byte(0xD0);
    LCD_WR_REG(0xF1);
    LCD_WR_Byte(0x0E);
    LCD_WR_REG(0xF9);
    LCD_WR_Byte(0x56);
    LCD_WR_REG(0xf2);
    LCD_WR_Byte(0x26);
    LCD_WR_Byte(0x1b);
    LCD_WR_Byte(0x0b);
    LCD_WR_Byte(0x20);
    LCD_WR_REG(0xec);
    LCD_WR_Byte(0x04);
    LCD_WR_REG(0x35);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x44);
    LCD_WR_Byte(0x00);
    LCD_WR_Byte(0x10);
    LCD_WR_REG(0x46);
    LCD_WR_Byte(0x10);
    LCD_WR_REG(0xff);
    LCD_WR_Byte(0x00);
    LCD_WR_REG(0x3a);
    LCD_WR_Byte(0x05);
    LCD_WR_REG(0x36);
    if (USE_HORIZONTIAL == 0)
    {
        LCD_WR_Byte(0x00);
    }
    else if (USE_HORIZONTIAL == 1)
    {
        LCD_WR_Byte(0xC0);
    }
    else if (USE_HORIZONTIAL == 2)
    {
        LCD_WR_Byte(0x70);
    }
    else
    {
        LCD_WR_Byte(0xA0);
    }
    LCD_WR_REG(0x11);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    LCD_WR_REG(0x29);
    vTaskDelay(150 / portTICK_PERIOD_MS);
}

