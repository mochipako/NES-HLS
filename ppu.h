#ifndef PPU_H_
#define PPU_H_

#include <stdint.h>
#include "ram.h"
#include "HLS/hls.h"
#include "HLS/ac_int.h"

class PPU{
    private:
        //NES *nes;
        //uint8_t VRAM[3*256*240];
        //uint8_t* VRAM;
        //uint8_t* CROM;
        //uint8_t VRAM_gray[256*240];
        //uint1 BG_Valid[256];
        ac_int<1, false> BG_Valid[256];
        //uint8_t *PPU_RAM;
        //uint8_t *SP_RAM;
        //bool en_gray;
        uint8_t line;

    public:
        bool render(uint8_t* PPU_RAM, uint8_t* SP_RAM, uint6* VRAM, struct SPREG* spreg);
        //void bg_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* VRAM, uint8_t BG_offset_x, uint8_t BG_offset_y);
        //void sp_render(uint8_t line, uint8_t ctrlreg1, uint8_t ctrlreg2, uint8_t* WRAM, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint8_t* VRAM);
        //void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint8_t* WRAM, uint8_t* VRAM, uint8_t ctrlreg2);
        void bg_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint6* VRAM);
        void sp_render(uint8_t line, struct SPREG* spreg, uint8_t* PPU_RAM, uint8_t* SP_RAM, uint6* VRAM);
        void store_vram(uint8_t line, uint8_t x, uint8_t color, bool sprite, uint6* VRAM, struct SPREG* spreg);
        void set_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
        void clr_bit(uint8_t* WRAM, uint16_t addr, uint8_t bit);
};



#endif

