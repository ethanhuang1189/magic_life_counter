#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define LCD_SCLK 6
#define LCD_SI   7
#define LCD_SCS  9
#define LCD_DISP 10

#define WIDTH  128
#define HEIGHT 128
#define WIDTH_BYTES (WIDTH / 8)

static bool vcom = false;
static uint8_t framebuffer[HEIGHT][WIDTH_BYTES];

// 3x5 digit font
static const uint8_t digits[10][5] = {
    {0b111,0b101,0b101,0b101,0b111}, // 0
    {0b010,0b110,0b010,0b010,0b111}, // 1
    {0b111,0b001,0b111,0b100,0b111}, // 2
    {0b111,0b001,0b111,0b001,0b111}, // 3
    {0b101,0b101,0b111,0b001,0b001}, // 4
    {0b111,0b100,0b111,0b001,0b111}, // 5
    {0b111,0b100,0b111,0b101,0b111}, // 6
    {0b111,0b001,0b001,0b001,0b001}, // 7
    {0b111,0b101,0b111,0b101,0b111}, // 8
    {0b111,0b101,0b111,0b001,0b111}  // 9
};

static uint8_t reverse_bits(uint8_t value) {
    uint8_t result = 0;

    for (int i = 0; i < 8; i++) {
        result <<= 1;
        result |= value & 1;
        value >>= 1;
    }

    return result;
}

static void set_pixel(int x, int y, bool white) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return;
    }

    int byte_index = x / 8;
    int bit_index = x % 8;

    if (white) {
        framebuffer[y][byte_index] |= (1 << bit_index);
    } else {
        framebuffer[y][byte_index] &= ~(1 << bit_index);
    }
}

static void clear_screen(bool white) {
    memset(framebuffer, white ? 0xFF : 0x00, sizeof(framebuffer));
}

static void draw_digit(int digit, int x, int y, int scale) {
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {

            bool pixel_on = digits[digit][row] & (1 << (2 - col));

            if (pixel_on) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        set_pixel(
                            x + col * scale + sx,
                            y + row * scale + sy,
                            false
                        );
                    }
                }
            }
        }
    }
}

static void draw_number(int number) {
    clear_screen(true);

    char text[12];
    snprintf(text, sizeof(text), "%d", number);

    int scale = 10;
    int digit_width = 3 * scale;
    int spacing = scale;
    int count = strlen(text);

    int total_width =
        count * digit_width +
        (count - 1) * spacing;

    int x = (WIDTH - total_width) / 2;
    int y = (HEIGHT - 5 * scale) / 2;

    for (int i = 0; i < count; i++) {
        int digit = text[i] - '0';

        draw_digit(digit, x, y, scale);

        x += digit_width + spacing;
    }
}

static void send_frame(void) {
    uint8_t byte;

    gpio_put(LCD_SCS, 1);
    sleep_us(10);

    byte = 0x80;

    if (vcom) {
        byte |= 0x40;
    }

    spi_write_blocking(spi0, &byte, 1);

    vcom = !vcom;

    for (int line = 0; line < HEIGHT; line++) {

        byte = reverse_bits((uint8_t)(line + 1));
        spi_write_blocking(spi0, &byte, 1);

        spi_write_blocking(
            spi0,
            framebuffer[line],
            WIDTH_BYTES
        );

        byte = 0x00;
        spi_write_blocking(spi0, &byte, 1);
    }

    byte = 0x00;
    spi_write_blocking(spi0, &byte, 1);

    sleep_us(10);
    gpio_put(LCD_SCS, 0);
}

int main(void) {
    stdio_init_all();

    gpio_init(LCD_DISP);
    gpio_set_dir(LCD_DISP, GPIO_OUT);
    gpio_put(LCD_DISP, 1);

    gpio_init(LCD_SCS);
    gpio_set_dir(LCD_SCS, GPIO_OUT);
    gpio_put(LCD_SCS, 0);

    spi_init(spi0, 100000);

    gpio_set_function(LCD_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_SI, GPIO_FUNC_SPI);

    spi_set_format(
        spi0,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    sleep_ms(1000);

    int number = 1;

    while (true) {
        printf("NUMBER: %d\n", number);

        draw_number(number);
        send_frame();

        number++;

        sleep_ms(1000);
    }
}