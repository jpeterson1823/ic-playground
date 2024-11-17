#pragma once

#include "hardware/ShiftReg.hpp"
#include "logic/LCDMessage.hpp"

#include <cstdint>
#include <string>

#define LCD_CMD_DELAY_US 10

namespace LCDMask {
    const uint8_t DDRAMAddrSet = 0x80;
    const uint8_t CGRAMAddrSet = 0x40;
    const uint8_t FunctionSet  = 0x30;
    const uint8_t CDShiftSet   = 0x10;
    const uint8_t DisplaySet   = 0x08;
    const uint8_t EntryModeSet = 0x04;
};

class LCD {
private:
    uint8_t e;
    uint8_t rw;
    uint8_t rs;
    ShiftReg* dataReg;

private:
    void gpioSetup();
    void awaken();
    void pulseEnable();
    void ddramAddr(uint8_t addr);
    void cgramAddr(uint8_t addr);

public:
    LCD(ShiftReg* dataReg, uint8_t e, uint8_t rw, uint8_t rs);

    // lcd setup methods
    void defaultSetup();
    void functionSet(bool byteBusMode, bool twoLineMode, bool smallFontMode);
    void entryModeSet(bool incDDRAM, bool shiftDisplay);
    void displaySet(bool displayOn, bool cursorOn, bool cursorBlink);

    // data methods
    void command(uint8_t byte);
    void writeChar(char c);
    void writeString(const char* str);
    void writeString(std::string str);
    void writeMessage(LCDMessage msg);

    // control methods
    void clear();
    void cursorHome();
    void setCursorPos(bool line, uint8_t index);
};
