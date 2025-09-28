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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu6502.h"
#include "mc6821.h"
#include "6502.rom.h"


// 8 kB ROM
#define ROMSIZE 0x2000
#define ROMMASK 0x1FFF
static uint8_t ROM[ROMSIZE];

// 32k kB RAM
#define RAMSIZE 0x8000
#define RAMMASK 0x7FFF
static uint8_t RAM[RAMSIZE];



// ROM read: returns the byte requested
static uint8_t rom_readbyte(uint16_t address){
    return ROM[address & ROMMASK];
}



//ROM write: writting to the ROM does nothing
static void rom_writebyte(uint16_t address, uint8_t data){
    // Do nothing
}



// RAM read: returns the byte requested
static uint8_t ram_readbyte(uint16_t address){
    return RAM[address & RAMMASK];
}



// RAM write: write the byte requested with the new value
static void ram_writebyte(uint16_t address, uint8_t data){
    RAM[address & RAMMASK] = data;
}



void motherboard_init(void){
    // Load rom from embedded data
    if (__6502_rom_bin_len != ROMSIZE) {
        fprintf(stderr, "Warning: ROM size mismatch! Embedded ROM is %u bytes, expected %u bytes.\n", __6502_rom_bin_len, ROMSIZE);
    }
    memcpy(ROM, __6502_rom_bin, (__6502_rom_bin_len < ROMSIZE) ? __6502_rom_bin_len : ROMSIZE);
    // Other initializations (if needed) go here
}



void motherboard_reset(void){
    // This function simulates the reset of the whole system, just
    // like the real reset signal in a motherboard
    // Neither ROM or RAM have reset
    //
    // Resets PIA
    mc6821_reset();
    
    // Resets CPU
    cpu_reset();
    
    // Done
}



// NOTE: This kind of switch-case with built-in
// ranges is an extension of GCC and Clang/LLVM
uint8_t motherboard_readbyte(uint16_t address){ 
    // This switch-case implements the address decoding
    switch (address){
        case 0x0000 ... 0x7FFF :
            return ram_readbyte(address);
            break;

        case 0xD000 ... 0xDFFF :
            return mc6821_readbyte(address);
            break;
                                 
        case 0xE000 ... 0xFFFF : 
            return rom_readbyte(address);
            break;
        
        default: 
            // Reading a not used address 
            // returns 0xFF in real hardware
            return 0xFF;
            break;
    }
}



void motherboard_writebyte(uint16_t address, uint8_t data){
    // This switch-case implements the address decoding
    switch (address){
        case 0x0000 ... 0x7FFF : 
            ram_writebyte(address, data);
            break;
                                
        case 0xD000 ... 0xDFFF :
            return mc6821_writebyte(address, data);            
            break;

        case 0xE000 ... 0xFFFF :
            rom_writebyte(address, data);
            break;
       
        default:
            // Write into a not mapped address use 
            // to do nothing in real hardware
            break;
    }
}
            

