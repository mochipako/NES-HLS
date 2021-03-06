#include "HLS/hls.h"
#include "HLS/ac_int.h"
#include "HLS/stdio.h"
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ram.h"
#include "cpu.h"
#include "ppu.h"

using namespace std;

//const bool test = true;
const bool test = false;

void load_ROM(ifstream *rom, uint8_t* PROM, uint8_t* CROM);
void set_vram(uint8_t* COLOR, uint8_t* VRAM);
void make_bmp(uint8_t* VRAM, int index);

uint8_t _PROM[0x8000];
uint8_t _CROM[0x2000];
void load_test_ROM(ifstream *rom);

void test_load(uint8_t* PROM, uint8_t* CROM){
    for(uint32_t addr = 0x0000; addr <= 0x7FFF; addr++){
        PROM[addr] = (uint8_t)_PROM[addr];
    }

    for(uint32_t addr = 0x00; addr <= 0x1FFF; addr++){
        CROM[addr] = _CROM[addr];
    }
}

component uint16_t exec_nes(
            ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8> >& VRAM,
            hls_stable_argument uint16_t nmi_vec, 
            hls_stable_argument uint16_t res_vec, 
            hls_stable_argument uint16_t irq_vec,
            uint8_t key, 
            bool res
        ){
    static CPU cpu;
    static PPU ppu;

    hls_init_on_powerup hls_doublepump static uint8_t PROM[0x8000];
    //this line can reduce II to 33 but incerease MEM blocks
    //hls_init_on_powerup static uint8_t PROM[0x8000];
    hls_init_on_powerup static uint8_t CROM[0x2000];
    static uint8_t PPU_RAM[0x2000];
    static uint8_t WRAM[0x800];
    static uint8_t SP_RAM[0x100];
    static uint16_t Stack[0x100];

    if(test){
        static bool init;
        if(!init) test_load(PROM, CROM);
        init = true;
        nmi_vec = (uint16_t)PROM[0x7FFB] << 8 | PROM[0x7FFA];
        res_vec = (uint16_t)PROM[0x7FFD] << 8 | PROM[0x7FFC];
        irq_vec = (uint16_t)PROM[0x7FFF] << 8 | PROM[0x7FFE];
    }

    static struct SPREG spreg;
    static bool nmi;
    
    uint8_t irq_num;
    if(nmi) irq_num = NMI;
    else if(res) irq_num = RESET;
    if(res | nmi){
            cpu.exec_irq(irq_num, nmi_vec, res_vec, irq_vec);
    }

    cpu.load_key(key);
    for(int c = 0; c < 38; c++) {
        cpu.exec(WRAM, PPU_RAM, SP_RAM, PROM, &spreg, Stack, CROM);
    }
    nmi = ppu.render(PPU_RAM, SP_RAM, VRAM, &spreg, CROM);

    return cpu.get_PC();
}


int main(int argc, char* argv[]){
    if(argc == 1){
        cout<< "usage: nes [ROM] [-k KEY] [-g] [-s] [-p port] [-f frame] [-b] [-i interval] [-l]" << endl;
        return -1;
    }
    ifstream ROM(argv[1],ios::in|ios::binary);
    if(!ROM){
        cout << "cannot open ROM file." << endl;
        return 1;
    }
    bool en_bmp = false;
    int frame = 1000;
    int interval = 8;
    int opt;
    while((opt = getopt(argc, argv, "f:bi:")) != -1){
        switch(opt){
            case 'f':
                frame = atoi(optarg);
                break;
            case 'b':
                en_bmp = true;
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            default:
                cout<< "usage: nes [ROM] [-f frame] [-b] [-i interval]" << endl;
                return -1;
                break;
        }
    }

    uint8_t COLOR[256*240];
    uint8_t VRAM[3*256*240];
    load_test_ROM(&ROM);
    ROM.close();
    ihc::mm_master<uint8_t, ihc::aspace<1>, ihc::awidth<16>, ihc::dwidth<8> > mm_COLOR(COLOR, sizeof(uint8_t)*(256*240));

    if(test == false) printf("WARNING: test is false.\n");
    int index = 0;
    int f = 0;
    exec_nes(mm_COLOR, 0, 0, 0, 0x0, true);
    while(f++ < frame){
        for(int l = 0; l < 256; l++){
            exec_nes(mm_COLOR, 0, 0, 0, 0x0, false);
        }
        if(en_bmp && f % interval == 0){
            set_vram(COLOR, VRAM);
            make_bmp(VRAM, index++);
        }
    }

    cout << "Finish" << endl;
    return 0;
}

void load_test_ROM(ifstream *rom){
    uint32_t magic;
    uint8_t prom_size;
    uint8_t crom_size;
    int psize;
    int csize;
    rom->read((char*)&magic, sizeof(uint32_t));
    if(magic != 0x1A53454E){
        cout << "This is not iNES format file." << endl;
    }
    rom->read((char*)&prom_size, sizeof(uint8_t));
    rom->read((char*)&crom_size, sizeof(uint8_t));

    psize = prom_size * 0x4000;
    csize = crom_size * 0x2000;

    rom->seekg(16,ios_base::beg);
    uint8_t *prom_ptr = (prom_size == 1) ? _PROM + 0x4000 : _PROM;
    for(int i=0;i<psize;i++)
        rom->read((char*)(prom_ptr+i), sizeof(uint8_t));
    
    for(int i=0;i<csize;i++)
        rom->read((char*)(_CROM+i), sizeof(uint8_t));
}

#define _rgb(r, g, b) (red = r, green = g, blue = b)
void set_vram(uint8_t* COLOR, uint8_t* VRAM){
    for(int i = 0; i < 240*256; i++){
        //BGR
        uint8_t blue, green, red; 
        switch(COLOR[i]){
            case 0x00: _rgb(0x75,0x75,0x75); break;
            case 0x01: _rgb(0x27,0x1B,0x8F); break;
            case 0x02: _rgb(0x00,0x00,0xAB); break;
            case 0x03: _rgb(0x47,0x00,0x9F); break;
            case 0x04: _rgb(0x8F,0x00,0x77); break;
            case 0x05: _rgb(0xAB,0x00,0x13); break;
            case 0x06: _rgb(0xA7,0x00,0x00); break;
            case 0x07: _rgb(0x7F,0x0B,0x00); break;
            case 0x08: _rgb(0x43,0x2F,0x00); break;
            case 0x09: _rgb(0x00,0x47,0x00); break;
            case 0x0a: _rgb(0x00,0x51,0x00); break;
            case 0x0b: _rgb(0x00,0x3F,0x17); break;
            case 0x0c: _rgb(0x1B,0x3F,0x5F); break;
            case 0x0d: _rgb(0x00,0x00,0x00); break;
            case 0x0e: _rgb(0x00,0x00,0x00); break;
            case 0x0f: _rgb(0x00,0x00,0x00); break;

            case 0x10: _rgb(0xBC,0xBC,0xBC); break;
            case 0x11: _rgb(0x00,0x73,0xEF); break;
            case 0x12: _rgb(0x23,0x3B,0xEF); break;
            case 0x13: _rgb(0x83,0x00,0xF3); break;
            case 0x14: _rgb(0xBF,0x00,0xBF); break;
            case 0x15: _rgb(0xE7,0x00,0x5B); break;
            case 0x16: _rgb(0xDB,0x2B,0x00); break;
            case 0x17: _rgb(0xCB,0x4F,0x0F); break;
            case 0x18: _rgb(0x8B,0x73,0x00); break;
            case 0x19: _rgb(0x00,0x97,0x00); break;
            case 0x1a: _rgb(0x00,0xAB,0x00); break;
            case 0x1b: _rgb(0x00,0x93,0x3B); break;
            case 0x1c: _rgb(0x00,0x83,0x8B); break;
            case 0x1d: _rgb(0x00,0x00,0x00); break;
            case 0x1e: _rgb(0x00,0x00,0x00); break;
            case 0x1f: _rgb(0x00,0x00,0x00); break;

            case 0x20: _rgb(0xFF,0xFF,0xFF); break;
            case 0x21: _rgb(0x3F,0xBF,0xFF); break;
            case 0x22: _rgb(0x5F,0x73,0xFF); break;
            case 0x23: _rgb(0xA7,0x8B,0xFD); break;
            case 0x24: _rgb(0xF7,0x7B,0xFF); break;
            case 0x25: _rgb(0xFF,0x77,0xB7); break;
            case 0x26: _rgb(0xFF,0x77,0x63); break;
            case 0x27: _rgb(0xFF,0x9B,0x3B); break;
            case 0x28: _rgb(0xF3,0xBF,0x3F); break;
            case 0x29: _rgb(0x83,0xD3,0x13); break;
            case 0x2a: _rgb(0x4F,0xDF,0x4B); break;
            case 0x2b: _rgb(0x58,0xF8,0x98); break;
            case 0x2c: _rgb(0x00,0xEB,0xDB); break;
            case 0x2d: _rgb(0x75,0x75,0x75); break;
            case 0x2e: _rgb(0x00,0x00,0x00); break;
            case 0x2f: _rgb(0x00,0x00,0x00); break;

            case 0x30: _rgb(0xFF,0xFF,0xFF); break;
            case 0x31: _rgb(0xAB,0xE7,0xFF); break;
            case 0x32: _rgb(0xC7,0xD7,0xFF); break;
            case 0x33: _rgb(0xD7,0xCB,0xFF); break;
            case 0x34: _rgb(0xFF,0xC7,0xFF); break;
            case 0x35: _rgb(0xFF,0xC7,0xDB); break;
            case 0x36: _rgb(0xFF,0xBF,0xB3); break;
            case 0x37: _rgb(0xFF,0xDB,0xAB); break;
            case 0x38: _rgb(0xFF,0xE7,0xA3); break;
            case 0x39: _rgb(0xE3,0xFF,0xA3); break;
            case 0x3a: _rgb(0xAB,0xF3,0xBF); break;
            case 0x3b: _rgb(0xB3,0xFF,0xCF); break;
            case 0x3c: _rgb(0x9F,0xFF,0xF3); break;
            case 0x3d: _rgb(0xBC,0xBC,0xBC); break;
            case 0x3e: _rgb(0x00,0x00,0x00); break;
            case 0x3f: _rgb(0x00,0x00,0x00); break;

            default: _rgb(0,0,0); break;
        }
        VRAM[3*i] = blue;
        VRAM[3*i + 1] = green;
        VRAM[3*i + 2] = red;
    }
}

void make_bmp(uint8_t* VRAM, int index){
    char bmp_file[256];
    sprintf(bmp_file, "./frame/frame_%d.bmp",index);
    ofstream bmp(bmp_file, ios::out|ios::binary);
    if(!bmp){
        cout << "cannot create BMP file." << endl;
        return;
    }

    const uint8_t bfType[] = "BM";
    const uint32_t bfSize = 184346; 
    const uint8_t bfReserved = 0;
    const uint32_t bfOffBits = 0x1A;

    bmp.write((char*)bfType, 2*sizeof(uint8_t));
    bmp.write((char*)&bfSize, sizeof(uint32_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfReserved, sizeof(uint8_t));
    bmp.write((char*)&bfOffBits, sizeof(uint32_t));

    const uint32_t bcSize = 12;
    const uint16_t bcWidth = 256;
    const uint16_t bcHeight = 240;
    const uint16_t bcPlanes = 1;
    const uint16_t bcBitCount = 24;

    bmp.write((char*)&bcSize, sizeof(uint32_t));
    bmp.write((char*)&bcWidth, sizeof(uint16_t));
    bmp.write((char*)&bcHeight, sizeof(uint16_t));
    bmp.write((char*)&bcPlanes, sizeof(uint16_t));
    bmp.write((char*)&bcBitCount, sizeof(uint16_t));
    bmp.write((char*)VRAM, 3*256*240*sizeof(uint8_t));

    bmp.close(); 
}

