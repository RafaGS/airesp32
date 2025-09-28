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

#include <stdint.h>

#include "terminal.h"

/* Instead of implementing a complete MC6820/6821 this source file 
 * implements the PIA once it is configured by the firmware (WozMon)
 * 
 * Keyboard
 * 
 * Keyboard is attached to port A, bits 0 to 6. bit 7 is tied to +5V
 * and keyboard strobe is connected to CA1. The firmware reads CRA register
 * and checks bit 7 (CA1). if bit 7 is 1 then reads PRA. Bit 7 of CRA is
 * cleared automatically once it is readed.
 * 
 * 
 * Terminal
 * 
 * Terminal is connected to port B, bit 0 to 6 (outputs). Bit 7 is input.
 * The firmware waits until bit 7 is zero, then outputs the ascii character
 * at port B
 */

// https://pdf.datasheetcatalog.com/datasheet/motorola/6821.pdf

// MC6820 PIA registers
static uint8_t PRA;
static uint8_t PRB;
static uint8_t DDRA;
static uint8_t DDRB;
static uint8_t CRA;
static uint8_t CRB;

// The PIA control lins are conected in this way:
//
// A4 --> CS0
// A1 --> RS1
// A0 --> RS0



void mc6821_reset(void){
    // From MC6820 datasheet:
    //
    // A RESET has the effect of zeroing all PIA registers.
    // This will set PAO-PA7, PBO-PB7, CA2 and CB2 as inputs,
    // and all interrupts disabled.
    PRA = 0x00;
    PRB = 0x00;
    DDRA = 0x00;
    DDRB = 0x00;
    CRA = 0x00;
    CRB = 0x00;
}



// Read a byte from the PIA
uint8_t mc6821_readbyte(uint16_t address){
    // Check for A4 = 1
    if (address & 0x0010){
        uint8_t data;
        switch (address & 0x0003){
            // RS1=0, RS0=0, 0xD010
            case 0: // PRA or DDRA
                if (CRA & 0x04){
                    data = read_keyboard();
                } else {
                    data = DDRA;
                }
                break;
                
            // RS1=0, RS0=1, 0xD011
            case 1: // CRA
                data = CRA & 0x7F;
                // Check if chars are available and return interrupt status
                if (check_keyboard_ready()){
                    data |= 0x80;
                } 
                break;
                
            // RS1=1, RS0=0, 0xD012
            case 2: // PRB or DDRB
                if (CRB & 0x04){                    
                    if (check_terminal_busy()){
                        data = PRB | 0x80; // Video busy flag set
                    } else {
                        data = PRB & 0x7F;  // Video busy flag cleared
                    }
                } else {
                    data = DDRB;
                }
                break;
                
            // RS1=1, RS0=1, 0xD013
            case 3:
                data = CRB;
                break;
        }
        return data;
    } else {
        // PIA not enabled
        return 0xFF;
    }
}



// Writes a byte to the PIA
void mc6821_writebyte(uint16_t address, uint8_t data){
    // Check for A4 = 1
    if (address & 0x0010){
        switch (address & 0x0003){

            // RS1=0, RS0=0, 0xD010
            case 0: // PRA or DDRA
                if (CRA & 0x04){
                    PRA = data;
                } else {
                    DDRA = data;
                }
                break;
                
            // RS1=0, RS0=1, 0xD011
            case 1: // CRA
                CRA = data;
                break;
                
            // RS1=1, RS0=0, 0xD012
            case 2: // PRB or DDRB
                if (CRB & 0x04){
                    PRB = data;
                    write_terminal(PRB);
                } else {
                    DDRB = data;
                }
                break;
                
            // RS1=1, RS0=1, 0xD013
            case 3:
                CRB = data;
                break;
        }
    }
    // Writes with A4 = 0 are ignored
    // because PIA is not selected
}
