//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//

#include <Arduino.h>
#include <ctype.h>
#include "terminal.h"

// Simple timing for ESP32
static unsigned long last_char_timestamp = 0;
static unsigned long last_key_timestamp = 0;

int ACTION = 0;



// Simple terminal functions for ESP32
// No need for complex terminal configuration

void configure_terminal(void){
    // Nothing to configure in ESP32 - Serial is already set up
    // Just ensure we have a clean state
    last_char_timestamp = millis();
    last_key_timestamp = millis();
}



// Checks if a key has been typed
uint8_t check_keyboard_ready(void){
    return Serial.available() > 0;
}



uint8_t read_keyboard(void){
    if (Serial.available() > 0) {
        int ch = Serial.read();
        if (ch != -1) {
            if (ch == '\n' || ch == '\r') {
                ch = 0x0D;   // Apple 1 uses CR
            }
            // Convert to uppercase for printable characters
            if (ch >= 0x20 && ch <= 0x7E) {
                ch = toupper(ch);
            }
            return ((uint8_t)ch) | 0x80; // Bit 7 set to one
        }
    }
    return 0x80; // Return something safe if no data
}



// Check for terminal ready
uint8_t check_terminal_busy(void){    
    unsigned long now = millis();
    unsigned long elapsed = now - last_char_timestamp;
    // The Apple 1 video circuit can't output characters faster than
    // 60 chars per second. This would be 1000ms / 60 = ~16.7ms delay
    return (elapsed < 17);    
}



// Write to the terminal
void write_terminal(uint8_t byte){
    static int char_counter = 0;
    // Only 7 bits configured for output
    byte &= 0x7F;
    byte = (uint8_t)toupper((int)byte);
    last_char_timestamp = millis();
    
    if ((byte == 0x0D) | (byte >= 32)){
        if (byte == 0x0D){
            Serial.println();  // Send newline
            char_counter = 0;
        } else {
            Serial.write(byte);  // Send character
            char_counter++;
        }
        // Apple 1 screen is only 40 characters wide
        if (char_counter == 40){
            Serial.println();  // Send newline
            char_counter = 0;
        }
        Serial.flush();  // Ensure data is sent
    }
}
