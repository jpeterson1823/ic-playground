#include "hardware/LCD.hpp"
#include "pico/stdlib.h"
#include <cstdlib>
#include <cstring>

LCD::LCD(ShiftReg* dataReg, uint8_t e, uint8_t rw, uint8_t rs)
: dataReg(dataReg), e(e), rw(rw), rs(rs)
{
    gpioSetup();
    awaken();
}

// general setup of GPIO pins
void LCD::gpioSetup() {
    gpio_init(e);
    gpio_init(rw);
    gpio_init(rs);

    gpio_set_dir(e, GPIO_OUT);
    gpio_set_dir(rw, GPIO_OUT);
    gpio_set_dir(rs, GPIO_OUT);

    gpio_pull_down(e);
    gpio_pull_down(rw);
    gpio_pull_down(rs);

    gpio_put(e, 0);
    gpio_put(rw, 0);
    gpio_put(rs, 0);
}

// go through wake-up procedure shown in datasheet example.
void LCD::awaken() {
    command(0x30);  // wake up #1
    sleep_ms(10);        // busy flag not available, must wait 5ms
    command(0x30);  // wake up #2
    sleep_ms(10);      // busy flag not available, must wait 160us
    command(0x30);  // wake up #3
    sleep_us(250);      // busy flag not available, must wait 160us
}

/**
 * Sets LCD to:
 *  - 8-bit bus
 *  - 2 lines
 *  - 5x8 dot font
 *  - display, cursor, and cursor blink ON
 *  - left-to-right writing
 *  - display shift DISABLED
 */
void LCD::defaultSetup() {
    functionSet(true, true, true);
    displaySet(true, true, true);
    entryModeSet(true, false);
}

// pulse enable pin for ~1000ns
void LCD::pulseEnable() {
    gpio_put(e, 1);
    sleep_us(5);
    gpio_put(e, 0);
    sleep_us(5);
}

// send command to lcd
void LCD::command(uint8_t byte) {
    // put data out on data bus
    dataReg->putByte(byte);

    // send command to lcd
    gpio_put(rs, 0);    // send instruction
    gpio_put(rw, 0);    // write
    pulseEnable();      // pulse enable pin
}

// sets DDRAM address
void LCD::ddramAddr(uint8_t addr) {
    command(LCDMask::DDRAMAddrSet | ((~LCDMask::DDRAMAddrSet)&addr));
    sleep_us(100);
}

// sets CGRAM address
void LCD::cgramAddr(uint8_t addr) {
    command(LCDMask::CGRAMAddrSet | ((~LCDMask::CGRAMAddrSet)&addr));
    sleep_us(100);
}

// function set options for LCD
void LCD::functionSet(bool byteBusMode, bool twoLineMode, bool smallFontMode) {
    // create command byte variable
    uint8_t command = LCDMask::FunctionSet;

    // 8bit bus mode selected
    if (byteBusMode)
        command |= 0x10;
    
    // two-line mode selected
    if (twoLineMode)
        command |= 0x08;
    
    // small font mode selected
    if (smallFontMode)
        command |= 0x04;

    // send command to lcd
    this->command(command);
}

// entry mode set options for LCD
void LCD::entryModeSet(bool incDDRAM, bool shiftDisplay) {
    // create command byte variable
    uint8_t command = LCDMask::EntryModeSet;

    // increment DDRAM selected
    if (incDDRAM)
        command |= 0x02;

    // shift display selected
    if (shiftDisplay)
        command |= 0x01;
    
    // send command to lcd
    this->command(command);
}

// display set options for LCD
void LCD::displaySet(bool displayOn, bool cursorOn, bool cursorBlink) {
    // create command byte variable
    uint8_t command = LCDMask::DisplaySet;

    // entire display on selected
    if (displayOn)
        command |= 0x04;

    // cursor is enabled
    if (cursorOn)
        command |= 0x02;
    
    // cursor blink enabled
    if (cursorBlink)
        command |= 0x01;
    
    // send command to lcd
    this->command(command);
}

// send character to lcd
void LCD::writeChar(char c) {
    // put data out on data bus
    dataReg->putByte(c);
    
    // write character to lcd
    gpio_put(rs, 1);    // send data
    gpio_put(rw, 0);    // write
    pulseEnable();      // pulse enable pin
    sleep_us(50);
    gpio_put(rs, 0);
}

// Write string to LCD on current line
void LCD::writeString(const char* str) {
    while (*str != '\0')
        writeChar(*str++);
}

// Write string to LCD on current line
void LCD::writeString(std::string str) {
    for (char c : str)
        writeChar(c);
}

// writes LCDMessage to display
void LCD::writeMessage(LCDMessage msg) {
    // write first line to display (0x00 - 0x0F)
    writeString(msg.getLine1());

    // move DDRAM to address 0x40 (line 2 of display)
    ddramAddr(0x40);

    // write second line to display (0x40 - 0x4F)
    writeString(msg.getLine2());
}

// clears lcd screen by clearing DDRAM
void LCD::clear() {
    command(0x01);
    sleep_us(1700);     // takes 1.52ms, delay a little longer
}

// returns cursor to home position by setting address couter to zero
void LCD::cursorHome() {
    command(0x02);
    sleep_us(1700);     // takes 1.52ms, delay a little longer
}

// Move cursor to specified location on LCD
void LCD::setCursorPos(bool line, uint8_t index) {
    // mask index to fit within 0x00-0x0F
    index &= 0x0F;

    // for line 2, mask index to fit within 0x40-0x4F
    if (line)
        index |= 0x40;
    
    // set DDRAM address to index
    ddramAddr(index);
}
