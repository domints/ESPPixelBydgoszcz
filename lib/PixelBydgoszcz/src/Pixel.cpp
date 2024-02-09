#include <Pixel.hpp>
#include <Arduino.h>
#include <esp_rom_crc.h>

PixelClass::PixelClass(Stream& serial, int txEnPin, int rxEnPin) :
    _serial(&serial),
    _txEnPin(txEnPin),
    _rxEnPin(rxEnPin)
{
    digitalWrite(_rxEnPin, HIGH);
    digitalWrite(_txEnPin, LOW);
    pinMode(_rxEnPin, OUTPUT);
    pinMode(_txEnPin, OUTPUT);
}

void PixelClass::begin() {
    digitalWrite(_rxEnPin, LOW);
    digitalWrite(_txEnPin, LOW);
}

void PixelClass::end() {
    digitalWrite(_rxEnPin, HIGH);
    digitalWrite(_txEnPin, LOW);
}

void PixelClass::sendSpace(bool autoEnd) {
    beginTransmit();
    uint8_t bfr[2] = {0x20, 0x04};
    _serial->write(bfr, 2);
    if (autoEnd)
        endTransmit();
}

void PixelClass::sendDblSpace(bool autoEnd) {
    beginTransmit();
    uint8_t bfr[3] = {0x20, 0x20, 0x04};
    _serial->write(bfr, 2);
    if (autoEnd)
        endTransmit();
}

void PixelClass::sendCommand(byte displayNo, char *command) {
    if (displayNo > 7)
        return;

    sendSpace(false);
    _serial->write('_');
    _serial->write(0x01);
    _serial->write('2');
    _serial->write(displayNo + 0x30);
    _serial->print(command);
    _serial->write(0x0a);
    _serial->write(0x0d);
    _serial->write(0x04);
    endTransmit();
}

void PixelClass::sendCommandWithBuffer(uint8_t displayNo, char *command, uint8_t data[], uint16_t dataLength)
{
    if (displayNo > 7)
        return;

    sendSpace(false);
    _serial->write('_');
    _serial->write(0x01);
    _serial->write('2');
    _serial->write(displayNo + 0x30);
    _serial->print(command);
    _serial->print(' ');
    _serial->write(data, (size_t)dataLength);
    _serial->write(0x0a);
    _serial->write(0x0d);
    _serial->write(0x04);
    endTransmit();
}

size_t PixelClass::readResponse(byte buffer[], uint16_t length, uint32_t timeout) {
    unsigned long orig_timeout = _serial->getTimeout();
    _serial->setTimeout(timeout);
    size_t readCnt = _serial->readBytesUntil(0x04, buffer, length);
    _serial->setTimeout(orig_timeout);
    return readCnt;
}

/// @brief Checks response code and finds response start
/// @param buffer buffer to check
/// @param msgLength Length of buffer
/// @param start message start or 0xFFFF if start byte not found
/// @return error code, possibly 0xFF is start byte not found or wrong code met
uint8_t PixelClass::checkResponse(byte buffer[], uint16_t msgLength, uint16_t &start)
{
    for (start = 0; start < msgLength; start++)
    {
        if (buffer[start] == 0x02)
            break;
    }

    if (start == msgLength)
    {
        start = 0xFFFF;
        return 0xFF;
    }

    char nibble1 = buffer[start + 8];
    char nibble2 = buffer[start + 9];

    uint8_t errorCode = 0x00;

    if (nibble1 >= 0x30 && nibble1 <= 0x39) {
        errorCode |= (nibble1 - 0x30) << 4;
    }
    else if (nibble1 >= 'A' && nibble1 <= 'F') {
        errorCode |= (nibble1 - 0x37) << 4;
    }
    else if (nibble1 >= 'a' && nibble2 <= 'f') {
        errorCode |= (nibble1 - 0x57) << 4;
    }
    else {
        return 0xFF;
    }

    if (nibble2 >= 0x30 && nibble2 <= 0x39) {
        errorCode |= (nibble2 - 0x30);
    }
    else if (nibble2 >= 'A' && nibble2 <= 'F') {
        errorCode |= (nibble2 - 0x37);
    }
    else if (nibble2 >= 'a' && nibble2 <= 'f') {
        errorCode |= (nibble2 - 0x57);
    }
    return errorCode;
}

/// @brief Reads string response for command
/// @param displayNo display number
/// @param command command string to send to display
/// @param buffer buffer for response
/// @param length length of buffer
/// @param responseMsgLength Length of resulting string, or 0xFFFF if failed to parse response
/// @return display response code or 0xFF if parsing error occured
uint8_t PixelClass::readStringCommand(uint8_t displayNo, char *command, char buffer[], uint16_t length, uint16_t& responseMsgLength) {
    sendCommand(displayNo, command);
    byte respBuffer[length + 32];
    size_t responseLength = readResponse(respBuffer, length + 32);
    uint16_t responseStart = 0;
    uint8_t errorCode = checkResponse(respBuffer, responseLength, responseStart);
    if (responseStart == 0xFFFF) {
        responseMsgLength = 0xFFFF;
        return 0xFF;
    }

    responseMsgLength = responseLength - responseStart - 11;

    for (uint16_t ix = 0; ix < responseMsgLength - 1; ix++) {
        buffer[ix] = respBuffer[ix + responseStart + 11];
    }

    buffer[responseMsgLength - 1] = 0x00;

    return errorCode;
}

uint8_t PixelClass::readGid(uint8_t displayNo, char buffer[], uint16_t length)
{
    uint16_t respLen = 0;
    return readStringCommand(displayNo, "GID", buffer, length, respLen);
}

uint8_t PixelClass::readDid(uint8_t displayNo, char buffer[], uint16_t length)
{
    uint16_t respLen = 0;
    return readStringCommand(displayNo, "DID", buffer, length, respLen);
}

uint8_t PixelClass::readFactoryIdentification(uint8_t displayNo, char buffer[], uint16_t length)
{
    uint16_t respLen = 0;
    return readStringCommand(displayNo, "#FI", buffer, length, respLen);
}

uint8_t PixelClass::getAvailableCommands(uint8_t displayNo, char buffer[], uint16_t length)
{
    uint16_t respLen = 0;
    return readStringCommand(displayNo, "#LC", buffer, length, respLen);
}

/// @brief Sends provided data block to display
/// @param displayNo display number
/// @param buffer data block
/// @param length length of data
/// @return display status code (non zero is bad)
uint8_t PixelClass::displayDataBlock(uint8_t displayNo, uint8_t buffer[], uint16_t length)
{
    sendCommandWithBuffer(0, "DDB", buffer, length);
    uint8_t respBuffer[32];
    size_t responseLength = readResponse(respBuffer, length + 32);
    uint16_t responseStart = 0;
    return checkResponse(respBuffer, responseLength, responseStart);
}

void PixelClass::beginTransmit()
{
    digitalWrite(_rxEnPin, HIGH);
    digitalWrite(_txEnPin, HIGH);
}

void PixelClass::endTransmit()
{
    _serial->flush();
    digitalWrite(_txEnPin, LOW);
    digitalWrite(_rxEnPin, LOW);
}

uint16_t PixelClass::getCrc(uint8_t buffer[], uint32_t len)
{
    return ~esp_rom_crc16_be((uint16_t)~0x0000, buffer, len);
}
