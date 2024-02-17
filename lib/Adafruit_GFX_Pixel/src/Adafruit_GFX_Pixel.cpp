#include <Adafruit_GFX.h>
#include <Pixel.hpp>
#include "Adafruit_GFX_Pixel.hpp"

Adafruit_Pixel::Adafruit_Pixel(PixelClass &pixel, uint16_t w) : _pixel(&pixel),
                                                                Adafruit_GFX(w, 16)
{
}

Adafruit_Pixel::~Adafruit_Pixel(void)
{
    for (uint8_t i = 0; i < PIXEL_BUFFER_CNT; i++)
    {
        if (_buffers[i])
        {
            free(_buffers[i]);
        }
    }

    free(_drawBuffer);
    free(_packetBuffer);
    free(_messageBuffer);
}

void Adafruit_Pixel::init()
{
    for (uint8_t i = 0; i < PIXEL_BUFFER_CNT; i++)
    {
        if (!_buffers[i])
        {
            _buffers[i] = (uint16_t *)malloc(_width * PIXEL_DISP_HEIGHT);
            memset(_buffers[i], 0, _width * PIXEL_DISP_HEIGHT);
        }
    }

    _drawBuffer = (uint8_t *)malloc(_width * (PIXEL_DISP_HEIGHT / 2));
    _packetBuffer = (uint8_t *)malloc(_width * (PIXEL_DISP_HEIGHT / 2) + 16);        // with space for header and CRC
    _messageBuffer = (uint8_t *)malloc((_width * (PIXEL_DISP_HEIGHT / 2) + 16) * 2); // with space for packetBuffer + CRC
}

void Adafruit_Pixel::selectBuffer(uint8_t bufferNo)
{
    if (bufferNo > PIXEL_BUFFER_CNT)
        return;

    _currentBuffer = bufferNo;
}

void Adafruit_Pixel::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
        return;

    if (color == 0x0000)
    {
        _buffers[_currentBuffer][x] &= ~(1 << y);
    }
    else
    {
        _buffers[_currentBuffer][x] |= (1 << y);
    }
}

uint8_t Adafruit_Pixel::commitBufferToPage(int8_t bufferNo, int8_t pageNo)
{
    uint8_t bfrId = (uint8_t)bufferNo;
    if (bufferNo < 0)
        bfrId = _currentBuffer;
    uint8_t page = (uint8_t)pageNo;
    if (pageNo < 0)
        pageNo = bufferNo;

    bool firstDot = false; // start from black unless chosen otherwise
    memset(_drawBuffer, 0, _width * (PIXEL_DISP_HEIGHT / 2));
    uint16_t currByte = 0;
    uint8_t currNibble = 0;
    uint16_t currCount = 0;
    bool lastBit = false;
    for (uint8_t i = 0; i < _width; i++)
    {
        if (i == 0)
        {
            firstDot = ((_buffers[bfrId][i]) & 0x8000);
            lastBit = firstDot;
        }

        for (uint8_t y = 0; y < 16; y++)
        {
            bool pixel = (_buffers[bfrId][i] & (1 << (15 - y)));
            if (pixel == lastBit)
            {
                currCount++;
            }
            else
            {
                lastBit = pixel;
                addBlockToDrawBuffer(currCount, currByte, currNibble);
                currCount = 1;
            }
        }
    }

    if (currCount > 0)
    {
        addBlockToDrawBuffer(currCount, currByte, currNibble);
    }

    if (currNibble == 1)
    {
        currByte++;
    }
    uint16_t packetLen = currByte + 15;

    _packetBuffer[0] = packetLen & 0xFF;
    _packetBuffer[1] = (packetLen >> 8) & 0xFF;
    _packetBuffer[2] = pageNo;
    _packetBuffer[3] = 0x01;
    _packetBuffer[4] = 0x03;
    _packetBuffer[5] = 0x00;
    _packetBuffer[6] = 0x01;
    _packetBuffer[7] = currByte + 6;
    _packetBuffer[8] = 0x00;
    _packetBuffer[9] = 0x00;  // offset from left, setting whole display so don't care
    _packetBuffer[10] = 0x00; // offset from bottom, setting whole display so don't care
    _packetBuffer[11] = 0x50; // config, (rowCount & 0x1f) | ((1 if startWith else 0) << 5) | 0b01000000
    if (firstDot)
        _packetBuffer[11] |= 0x20; // set first dot to white/yellow;

    _packetBuffer[12] = _width; // column number, setting whole display, so width;

    for (uint16_t i = 0; i < currByte; i++)
    {
        _packetBuffer[i + 13] = _drawBuffer[i];
    }

    uint16_t crc = _pixel->getCrc(_packetBuffer, currByte + 13);
    _packetBuffer[currByte + 13] = crc & 0xFF;
    _packetBuffer[currByte + 14] = (crc >> 8) & 0xFF;

    for (uint16_t i = 0; i < packetLen; i++)
    {
        byteToHex(_packetBuffer[i], _messageBuffer, i * 2);
    }

    return _pixel->displayDataBlock(0, _messageBuffer, packetLen * 2);
}

void Adafruit_Pixel::addBlockToDrawBuffer(uint16_t currCount, uint16_t &currByte, uint8_t &currNibble)
{
    if (currCount > 15)
    {
        uint8_t requiredZeroes = currCount / 15;
        currByte += requiredZeroes / 2;
        if (requiredZeroes % 2)
        {
            currNibble++;
            if (currNibble == 2)
            {
                currNibble = 0;
                currByte++;
            }
        }
        uint8_t leftover = currCount % 15;
        if (currNibble == 0)
        {
            _drawBuffer[currByte] |= (leftover << 4);
            currNibble++;
        }
        else
        {
            _drawBuffer[currByte] |= leftover;
            currNibble = 0;
            currByte++;
        }
    }
    else
    {
        if (currNibble == 0)
        {
            _drawBuffer[currByte] |= (currCount << 4);
            currNibble++;
        }
        else
        {
            _drawBuffer[currByte] |= currCount;
            currNibble = 0;
            currByte++;
        }
    }
}

void Adafruit_Pixel::byteToHex(uint8_t value, uint8_t *buffer, uint16_t position)
{
    uint8_t firstNibble = (value >> 4) & 0xF;
    uint8_t secondNibble = value & 0xF;
    buffer[position] = nibbleToHex(firstNibble);
    buffer[position + 1] = nibbleToHex(secondNibble);
}

uint8_t Adafruit_Pixel::nibbleToHex(uint8_t nibble)
{
    if (nibble < 0xA)
        return nibble + 0x30;

    return nibble + 0x37;
}
