
#include "ram.h"
#include <fstream>
#include <iostream>

using namespace std;

//RAM::RAM(NES *n, uint8_t* prom, uint8_t* crom){
RAM::RAM(){
    BGoffset_sel_X = true;
    PPUAddr_sel_H = true;
    pad_reset_state = 0;
    pad_read_state = 0;
    pad_input = 0;
    spram_buf = 0;
}


void RAM::dump_WRAM(uint16_t start_addr, uint16_t size){
    int i, k;
    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
        if(k % 16 == 0){
            printf("\n");
            printf("%04x : ", i);
        }
        printf("%02x ",WRAM[i]);
    }
    printf("\n");
}

void RAM::dump_PROM(uint16_t start_addr, uint16_t size){
    int i, k;
    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
        if(k % 16 == 0){
            printf("\n");
            printf("%04x : ", i);
        }
        printf("%02x ",PROM[i-0x8000]);
    }
    printf("\n");
}

void RAM::dump_PPURAM(uint16_t start_addr, uint16_t size){
    int i, k;
    for(i = start_addr, k = 0;i < start_addr + size; i++, k++){
        if(k % 16 == 0){
            printf("\n");
            printf("%04x : ", i);
        }
        printf("%02x ",PPU_RAM[i]);
    }
    printf("\n");
}

uint8_t RAM::read(uint16_t addr){
    uint8_t data;
    if(addr < 0x2000)   addr = addr & 0x7FF;
    else if(addr < 0x4000) addr = addr & 0x2007;

    if(addr >= 0x8000) data = read_prom(addr);
    else{
        switch(addr){
            case 0x2002: 
                data = _set(VBlank,7)|_set(SPhit,6)|_set(num_ScanSP,5);
                VBlank = false;
                BGoffset_sel_X = true;
                PPUAddr_sel_H = true;
                break;
            case 0x2007:
                data = read_2007();
                break;
            case 0x4016:
                data = read_pad_1();
                break;
            case 0x4017:
                data = read_pad_2();
                break;
            default:
                data = WRAM[addr];
                break;
        }
    }
    return data;
}

void RAM::write(uint16_t addr, uint8_t data){
    if(addr < 0x2000)   addr = addr & 0x7FF;
    else if(addr < 0x4000) addr = addr & 0x2007;
    switch(addr){
        case 0x2000: 
            write_2000(data);
            break;
        case 0x2001: 
            write_2001(data);
            break;
        case 0x2003: 
            write_2003(data);
            break;
        case 0x2004: 
            write_2004(data);
            break;
        case 0x2005: 
            write_2005(data);
            break;
        case 0x2006: 
            write_2006(data);
            break;
        case 0x2007: 
            write_2007(data);
            break;
        case 0x4014: 
            DMA_start(data);
            break;
        case 0x4016: 
            reset_pad(data);
            break;
        default:
            WRAM[addr] = data;
            break;
    }
    //if(addr >= 0x8000)
    //    printf("OK!\n");
}

void RAM::set_VBlank(bool vblank, bool nmi){
    VBlank = vblank;
    if(nmi && (!VBlank || VBlank_NMI))
        nes->cpu->set_nmi(VBlank);
}

void RAM::write_2000(uint8_t data){
    VBlank_NMI = (bool)((data >> 7) & 1);    
    SPSize =     (bool)((data >> 5) & 1);    
    BGPtnAddr =  (bool)((data >> 4) & 1);    
    SPPtnAddr =  (bool)((data >> 3) & 1);    
    PPUInc =     (bool)((data >> 2) & 1);    
    NameAddrH =  (bool)((data >> 1) & 1);    
    NameAddrL =  (bool)((data >> 0) & 1);    
}

void RAM::write_2001(uint8_t data){
    //printf("write $2001. data = %02x\n",data);
    BGColor2 =  (bool)((data >> 7) & 1);    
    BGColor1 =  (bool)((data >> 6) & 1);    
    BGColor0 =  (bool)((data >> 5) & 1);    
    EnSP =      (bool)((data >> 4) & 1);    
    EnBG =      (bool)((data >> 3) & 1);    
    SPMSK =     (bool)((data >> 2) & 1);    
    BGMSK =     (bool)((data >> 1) & 1);    
    DispType =  (bool)((data >> 0) & 1);    
}

void RAM::write_2003(uint8_t data){
    SPAddr = data;
}

void RAM::write_2004(uint8_t data){
    //SP_RAM[SPAddr++] = data;
    write_SPRAM(SPAddr++, data);
}

void RAM::write_2005(uint8_t data){
    if(BGoffset_sel_X){
        BGoffset_X = data;
        BGoffset_sel_X = false;
    }  
    else{
        BGoffset_Y = data;
        BGoffset_sel_X = true;
    }
}

void RAM::write_2006(uint8_t data){
    if(PPUAddr_sel_H){
        PPUAddr_H = data;
        PPUAddr_sel_H = false;
    }
    else{
        PPUAddr_L = data;
        PPUAddr_sel_H = true;
    } 
}

void RAM::write_2007(uint8_t data){
    uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    uint32_t addr = PPU_Addr & 0x3fff;
    switch(addr){
        case 0x3f10:
            addr = 0x3f00;
            break;
        case 0x3f14:
            addr = 0x3f04;
            break;
        case 0x3f18:
            addr = 0x3f08;
            break;
        case 0x3f1C:
            addr = 0x3f0C;
            break;
    }
    //PPU_RAM[addr] = data;
    write_PPURAM(addr, data);
    PPU_Addr = (PPUInc) ? PPU_Addr + 32 : PPU_Addr + 1;
    PPUAddr_H = (uint8_t)(PPU_Addr >> 8);
    PPUAddr_L = (uint8_t)PPU_Addr;
}

uint8_t RAM::read_2007(){
    uint8_t data;
    uint16_t PPU_Addr = ((uint16_t)PPUAddr_H << 8) | PPUAddr_L;
    uint32_t addr = PPU_Addr & 0x3fff;
    switch(addr){
        case 0x3f10:
            addr = 0x3f00;
            break;
        case 0x3f14:
            addr = 0x3f04;
            break;
        case 0x3f18:
            addr = 0x3f08;
            break;
        case 0x3f1C:
            addr = 0x3f0C;
            break;
    }
    if(addr < 0x3F00){
        data = spram_buf;
        //spram_buf = PPU_RAM[addr];
        spram_buf = read_PPURAM(addr);
    }
    else 
        //data = PPU_RAM[addr];
        data = read_PPURAM(addr);
    //printf("read: %04x %02x\n", addr, data);
    PPU_Addr = (PPUInc) ? PPU_Addr + 32 : PPU_Addr + 1;
    PPUAddr_H = (uint8_t)(PPU_Addr >> 8);
    PPUAddr_L = (uint8_t)PPU_Addr;
    return data;
}

void RAM::DMA_start(uint8_t addr_H){
    uint16_t addr = ((uint16_t)addr_H << 8);
    for(int i = 0; i < 256; i++)
        //SP_RAM[i] = WRAM[addr + i];
        write_SPRAM(i, WRAM[addr+i]);
}

void RAM::reset_pad(uint8_t data){
    if(pad_reset_state == 1 && ((data & 1) == 0))
        pad_reset_state = 2;
    
    if(pad_reset_state == 0 && ((data & 1) == 1))
        pad_reset_state = 1;

    if(pad_reset_state == 2){
        pad_reset_state = 0;
        pad_read_state = 0;
        pad_input = key_input;
    }
}

uint8_t RAM::read_pad_1(){
    uint8_t data;
    switch(pad_read_state){
       //A
       case 0: 
           data = (pad_input & 1);
           pad_read_state++;
           break;
       //B
       case 1: 
           data = ((pad_input >> 1) & 1);
           pad_read_state++;
           break;
       //SELECT
       case 2: 
           data = ((pad_input >> 2) & 1);
           pad_read_state++;
           break;
       //START
       case 3: 
           data = ((pad_input >> 3) & 1);
           pad_read_state++;
           break;
       //UP
       case 4: 
           data = ((pad_input >> 4) & 1);
           pad_read_state++;
           break;
       //DOWN
       case 5: 
           data = ((pad_input >> 5) & 1);
           pad_read_state++;
           break;
       //LEFT
       case 6: 
           data = ((pad_input >> 6) & 1);
           pad_read_state++;
           break;
       //RIGHT
       case 7: 
           data = ((pad_input >> 7) & 1);
           pad_read_state = 0;
           break;
    }
    return data;
}

uint8_t RAM::read_pad_2(){
    return 0;
}

void RAM::frame_end(){
    NameAddrH = false;    
    NameAddrL = false;    
}

uint8_t RAM::read_prom(uint16_t addr){
    return PROM[addr - 0x8000];
}

void RAM::write_PPURAM(uint16_t addr, uint8_t data){ 
    if(addr <= 0x2000)
        CROM[addr] = data;
    else
        PPU_RAM[addr-0x2000] = data; 
}

uint8_t RAM::read_PPURAM(uint16_t addr){ 
    if(addr <= 0x2000)
        return CROM[addr];
    else
        return PPU_RAM[addr-0x2000]; 
}
