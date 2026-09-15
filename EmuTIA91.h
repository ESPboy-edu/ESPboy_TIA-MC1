#pragma once

#define SPR_ATTR_ENABLE    0x01
#define SPR_ATTR_FLIP_H   0x08
#define SPR_ATTR_FLIP_V   0x02

#define TIA91_CPU_FREQ      (15750000 / 9)
#define TIA91_TICKS_VSYNC   57
#define TIA91_TICKS_FRAME   (TIA91_CPU_FREQ / 50)

extern uint8_t *shared_ram; 

namespace EmuTIA91 {
const double g_v[] = { 1.2071, 0.9971, 0.9259, 0.7159, 0.4912, 0.2812, 0.2100, 0.0000 };
const double r_v[] = { 1.5937, 1.3125, 1.1562, 0.8750, 0.7187, 0.4375, 0.2812, 0.0000 };
const double b_v[] = { 1.3523, 0.8750, 0.4773, 0.0000 };

uint16_t palette565[256];
uint16_t vpal_565[16];
uint16_t blend_lut[16][16];

const uint8_t* pmem_banks[4]; 
const uint8_t* srom_banks[4]; 
const uint8_t* crom_banks[4]; 
unsigned char* pmem_ram = 0; 

unsigned char spr_x[16], spr_y[16], spr_n[16], spr_a[16];
unsigned char pio_d0, pio_d1, pio_d2, vsync, vram_sel;
unsigned char scr_xoff, scr_yoff;

bool interpolate = true; 
bool toggle_lock = false;

inline unsigned char port_rd(unsigned char adr) {
  switch (adr) {
    case 0xD0: return pio_d0;
    case 0xD1: return pio_d1;
    case 0xD2: return pio_d2 | vsync;
  }
  return 0;
}

inline void rebuild_lut(int c) {
    uint16_t p1 = vpal_565[c];
    for (int i = 0; i < 16; i++) {
        uint16_t p2 = vpal_565[i];
        uint16_t avg = (p1 & p2) + (((p1 ^ p2) & 0xF7DE) >> 1);
        blend_lut[c][i] = avg;
        blend_lut[i][c] = avg;
    }
}

inline void port_wr(unsigned char adr, unsigned char val) {
  switch (adr & 0xf0) {
    case 0x00: spr_y[adr - 0x00] = val ^ 0xff; return;
    case 0x10: spr_x[adr - 0x10] = val ^ 0xff; return;
    case 0x20: spr_n[adr - 0x20] = val ^ 0xff; return;
    case 0x30: spr_a[adr - 0x30] = val ^ 0xff; return;
    case 0xe0: 
      adr -= 0xe0; vpal_565[adr] = palette565[val];
      rebuild_lut(adr); return;
  }
  switch (adr) {
    case 0xf0: return;
    case 0xf4: return;
    case 0xf8: vram_sel = val; return;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
      SoundTIA::write(CPU8080::time, adr, val); return;
  }
}

inline unsigned char read_crom(unsigned short int adr) {
    int bank = adr >> 13; 
    if (bank < 4 && crom_banks[bank]) return pgm_read_byte(&crom_banks[bank][adr & 0x1FFF]);
    return 0x00;
}

inline unsigned char read_srom(unsigned short int adr) {
    int bank = adr >> 13; 
    if (bank < 4 && srom_banks[bank]) return pgm_read_byte(&srom_banks[bank][adr & 0x1FFF]);
    return 0x00;
}

inline unsigned char mem_rd(unsigned short int adr) {
  if (adr >= 0xC000) return pmem_ram[adr - 0xC000];
  int bank = adr >> 13;
  if (bank < 4 && pmem_banks[bank]) return pgm_read_byte(&pmem_banks[bank][adr & 0x1FFF]);
  return 0x00;
}

inline void mem_wr(unsigned short int adr, unsigned char val) {
  if (adr >= 0xC000) pmem_ram[adr - 0xC000] = val;
}

inline int update(uint8_t keys) {
  pio_d0 = 0; pio_d1 = 0; pio_d2 = 0;
  
  if ((keys & PAD_LFT) && (keys & PAD_RGT)) {
      if (!toggle_lock) { interpolate = !interpolate; toggle_lock = true; }
  } else { toggle_lock = false; }

  if (keys & PAD_LEFT)  pio_d0 |= 0x20; // Движение влево
  if (keys & PAD_RIGHT) pio_d0 |= 0x02; // Движение вправо

  if (keys & PAD_RGT)  pio_d1 |= 0x80; // Сервисная кнопка (тест/пропуск)

  if (keys & PAD_LFT)  pio_d2 |= 0x10; // Монетка
  
  // Разделяем действия на две физические кнопки
  if (keys & PAD_ACT)  pio_d2 |= 0x40; // Удар ногой (Кнопка A)
  if (keys & PAD_ESC)  pio_d2 |= 0x20; // Удар рукой (Кнопка B)

  // Инверсия активных битов для портов D0, D1 и D2
  pio_d0 ^= 0x22; pio_d1 ^= 0x22 | 0x80; pio_d2 ^= 0x20 | 0x40;
  
  vsync |= 0x80;
  CPU8080::time = 0;
  CPU8080::run(TIA91_TICKS_VSYNC);
  vsync &= ~0x80;

  int t = TIA91_TICKS_FRAME - TIA91_TICKS_VSYNC;
  CPU8080::run(t);
  SoundTIA::write(CPU8080::time, 0, 0);

  return t;
}

inline void get_scanline_pixels(int bg_y, uint8_t* line_buf) {
    int ty = bg_y >> 3;
    int py = bg_y & 7;
    int vram_ptr = 0xf000 + ty * 32;

    for (int tx = 0; tx < 32; ++tx) {
        int tile = mem_rd(vram_ptr + tx);
        int ptr = (((vram_sel & 0xfe) << 7) + (tile << 3)) & 0x1ff8;
        ptr += py;
        int screen_x = (tx << 3) + scr_xoff;
        
        unsigned char bp1 = read_crom(ptr + 0x0000);
        unsigned char bp2 = read_crom(ptr + 0x2000);
        unsigned char bp3 = read_crom(ptr + 0x4000);
        unsigned char bp4 = read_crom(ptr + 0x6000);
        
        int mask = 0x80;
        for (int px = 0; px < 8; ++px) {
            unsigned char col = 
                (bp1 & mask ? 1 : 0) | 
                (bp2 & mask ? 2 : 0) | 
                (bp3 & mask ? 4 : 0) | 
                (bp4 & mask ? 8 : 0);
            line_buf[(screen_x + px) & 255] = col;
            mask >>= 1;
        }
    }
    
    for (int id = 0; id < 16; ++id) {
        if (!(spr_a[id] & SPR_ATTR_ENABLE)) continue;
        int sy = spr_y[id];
        int dy = (bg_y - sy) & 255;
        if (spr_a[id] & SPR_ATTR_FLIP_V) dy = 15 - dy;
        
        if (dy >= 0 && dy < 16) {
            int ptr = (spr_n[id] << 4) + dy;
            int sx = spr_x[id];
            bool flip_h = spr_a[id] & SPR_ATTR_FLIP_H;
            
            int ptrx_l = ptr;
            unsigned char bp1_l = read_srom(ptrx_l + 0x0000);
            unsigned char bp2_l = read_srom(ptrx_l + 0x2000);
            unsigned char bp3_l = read_srom(ptrx_l + 0x4000);
            unsigned char bp4_l = read_srom(ptrx_l + 0x6000);
            
            // ИСПРАВЛЕНИЕ: смещение правой половины равно 4096 (8 << 9)
            int ptrx_r = ptr + 4096; 
            unsigned char bp1_r = read_srom(ptrx_r + 0x0000);
            unsigned char bp2_r = read_srom(ptrx_r + 0x2000);
            unsigned char bp3_r = read_srom(ptrx_r + 0x4000);
            unsigned char bp4_r = read_srom(ptrx_r + 0x6000);

            for (int px = 0; px < 16; ++px) {
                unsigned char col;
                int mask = 128 >> (px & 7);
                
                if (px < 8) {
                    col = (bp1_l & mask ? 1 : 0) | (bp2_l & mask ? 2 : 0) | 
                          (bp3_l & mask ? 4 : 0) | (bp4_l & mask ? 8 : 0);
                } else {
                    col = (bp1_r & mask ? 1 : 0) | (bp2_r & mask ? 2 : 0) | 
                          (bp3_r & mask ? 4 : 0) | (bp4_r & mask ? 8 : 0);
                }
                  
                // Цвет 15 (0x0f) в автоматах ТИА означает прозрачность
                if (col < 0x0f) {
                    int draw_x = (sx + (flip_h ? 15 - px : px)) & 255;
                    line_buf[draw_x] = col;
                }
            }
        }
    }
}

inline void render_scanline(int tft_y, uint16_t* buffer) {
    int orig_y1 = tft_y * 2;
    uint8_t line1[256];
    get_scanline_pixels((orig_y1 + scr_yoff) & 255, line1);

    if (interpolate) {
        uint8_t line2[256];
        get_scanline_pixels((orig_y1 + 1 + scr_yoff) & 255, line2);
        
        for (int x = 0; x < 128; ++x) {
            int ox = x * 2;
            uint16_t p1 = blend_lut[line1[ox] & 0x0F][line1[ox + 1] & 0x0F];
            uint16_t p2 = blend_lut[line2[ox] & 0x0F][line2[ox + 1] & 0x0F];
            uint16_t c = (p1 & p2) + (((p1 ^ p2) & 0xF7DE) >> 1);
            buffer[x] = (c >> 8) | (c << 8); // Endianness swap
        }
    } else {
        for (int x = 0; x < 128; ++x) {
            uint16_t c = vpal_565[line1[x * 2] & 0x0F];
            buffer[x] = (c >> 8) | (c << 8); // Endianness swap
        }
    }
}

inline void init(void) {
  for(int i=0; i<4; i++) { pmem_banks[i] = nullptr; srom_banks[i] = nullptr; crom_banks[i] = nullptr; }
  
  pmem_ram = shared_ram; 

  SoundTIA::init(2);

  vsync = 0; vram_sel = 0; scr_xoff = 0; scr_yoff = 0;
  pio_d0 = 0; pio_d1 = 0; pio_d2 = 0;
  for (int i = 0; i < 16; ++i) spr_a[i] = 0;

  for (int i = 0; i < 256; ++i) {
    int ir = (i >> 3) & 7; int ig = i & 7; int ib = (i >> 6) & 3;
    double dr = 255.0 * r_v[ir] / r_v[0];
    double dg = 255.0 * g_v[ig] / g_v[0];
    double db = 255.0 * b_v[ib] / b_v[0];

    uint16_t r = (255 - (int)dr) & 255;
    uint16_t g = (255 - (int)dg) & 255;
    uint16_t b = (255 - (int)db) & 255;
    palette565[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  for (int i = 0; i < 16; ++i) {
    vpal_565[i] = palette565[i];
    rebuild_lut(i);
  }

  CPU8080::init(mem_rd, mem_wr, port_rd, port_wr);
}

inline void reset(void) { 
  CPU8080::reset(); 
  SoundTIA::reset(); 
}
}
