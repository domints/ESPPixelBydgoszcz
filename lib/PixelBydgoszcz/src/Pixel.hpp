#ifndef __PIXEL_BYDGOSZCZ__
#define __PIXEL_BYDGOSZCZ__

#include <Arduino.h>

class PixelClass {
    public:
        /// @brief Constructor for Pixel class
        /// @param serial Stream object for serial. Idk, make it hardware or software, just make sure it's initialized to 4800 baud, 8E1.
        /// @param txEnPin 
        /// @param rxEnPin 
        PixelClass(Stream& serial, int txEnPin, int rxEnPin);
        void begin();
        void end();

        void sendSpace(bool autoEnd = true);
        void sendDblSpace(bool autoEnd = true);

        void sendCommand(byte displayNo, char *command);
        size_t readResponse(byte buffer[], uint16_t length, uint32_t timeout = 3000);
        uint8_t checkResponse(byte buffer[], uint16_t msgLength, uint16_t& start);
        uint8_t readStringCommand(uint8_t displayNo, char *command, char buffer[], uint16_t length, uint16_t& responseMsgLength);

    private:
        Stream* _serial;
        int _txEnPin;
        int _rxEnPin;

        void beginTransmit();
        void endTransmit();
};

extern PixelClass Pixel;    

#endif