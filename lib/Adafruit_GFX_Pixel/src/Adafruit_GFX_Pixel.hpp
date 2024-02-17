#ifndef __ADAFRUIT_PIXEL_BYDGOSZCZ__
#define __ADAFRUIT_PIXEL_BYDGOSZCZ__

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Pixel.hpp>

#define PIXEL_BUFFER_CNT 16
#define PIXEL_DISP_HEIGHT 16

class Adafruit_Pixel : public Adafruit_GFX {
    public:
        Adafruit_Pixel(PixelClass &pixel, uint16_t w);
        ~Adafruit_Pixel(void);

        void init();
        void selectBuffer(uint8_t bufferNo);

        void drawPixel(int16_t x, int16_t y, uint16_t color);

        uint8_t commitBufferToPage(int8_t bufferNo = -1, int8_t pageNo = -1);

    private:
        PixelClass *_pixel = NULL;

        uint8_t _currentBuffer = 0;

        uint16_t* _buffers[PIXEL_BUFFER_CNT];
        uint8_t * _drawBuffer = NULL;
        uint8_t *_packetBuffer = NULL; // Buffer for raw bytes
        uint8_t *_messageBuffer = NULL; // Buffers for ASCII chars to send to display

        void addBlockToDrawBuffer(uint16_t currCount, uint16_t &currByte, uint8_t &currNibble);
        void byteToHex(uint8_t value, uint8_t *buffer, uint16_t position);
        uint8_t nibbleToHex(uint8_t nibble);
};

#endif