#pragma once

#define RASTER_W    256
#define RASTER_H    240
#define SAMPLE_RATE   20000 
#define SOUND_VOLUME  100

extern uint8_t *shared_ram; 

extern int emu_core;
extern int emu_loops;

extern ESPboyInit myESPboy;

enum { EMU_TIA88, EMU_TIA91 };

enum {
  GAME_KONEK = 0, GAME_KOROLEVA, GAME_SOS, GAME_GORODKI, 
  GAME_BILYARD, GAME_KOT_RYBOLOV, GAMES_ALL
};

#include "CPU.h"
#include "IOList.h"
#include "SoundTIA.h"
#include "EmuTIA88.h"
#include "EmuTIA91.h"

inline void set_emu_tia88(bool analog) { emu_core = EMU_TIA88; emu_loops = 2; EmuTIA88::init(analog); }
inline void set_emu_tia91(void) { emu_core = EMU_TIA91; emu_loops = 2; EmuTIA91::init(); }

inline void load_konek(void) {
  set_emu_tia88(false);
  EmuTIA88::pmem_banks[0] = konek_g1_bin; EmuTIA88::pmem_banks[1] = konek_g2_bin;
  EmuTIA88::pmem_banks[2] = konek_g3_bin; EmuTIA88::pmem_banks[3] = konek_g4_bin;
  EmuTIA88::pmem_banks[4] = konek_g5_bin; EmuTIA88::pmem_banks[6] = konek_g7_bin;
  EmuTIA88::vrom_banks[0] = konek_a2_bin; EmuTIA88::vrom_banks[1] = konek_a3_bin;
  EmuTIA88::vrom_banks[2] = konek_a5_bin; EmuTIA88::vrom_banks[3] = konek_a6_bin;
  emu_loops = 500;
}

inline void load_bilyard(void) {
  set_emu_tia88(false);
  EmuTIA88::pmem_banks[0] = bilyard_g1_bin; EmuTIA88::pmem_banks[1] = bilyard_g2_bin;
  EmuTIA88::pmem_banks[6] = bilyard_g7_bin;
  EmuTIA88::vrom_banks[0] = bilyard_a2_bin; EmuTIA88::vrom_banks[1] = bilyard_a3_bin;
  EmuTIA88::vrom_banks[2] = bilyard_a5_bin; EmuTIA88::vrom_banks[3] = bilyard_a6_bin;
  emu_loops = 250;
}

inline void load_gorodki(void) {
  set_emu_tia88(true);
  EmuTIA88::pmem_banks[0] = gorodki_g1_bin; EmuTIA88::pmem_banks[1] = gorodki_g2_bin;
  EmuTIA88::pmem_banks[2] = gorodki_g3_bin; EmuTIA88::pmem_banks[3] = gorodki_g4_bin;
  EmuTIA88::vrom_banks[0] = gorodki_a2_bin; EmuTIA88::vrom_banks[1] = gorodki_a3_bin;
  EmuTIA88::vrom_banks[2] = gorodki_a5_bin; EmuTIA88::vrom_banks[3] = gorodki_a6_bin;
  emu_loops = 400;
}

inline void load_koroleva(void) {
  set_emu_tia88(false);
  EmuTIA88::pmem_banks[0] = koroleva_g1_bin; EmuTIA88::pmem_banks[1] = koroleva_g2_bin;
  EmuTIA88::pmem_banks[2] = koroleva_g3_bin; EmuTIA88::pmem_banks[3] = koroleva_g4_bin;
  EmuTIA88::pmem_banks[4] = koroleva_g5_bin; EmuTIA88::pmem_banks[6] = koroleva_g7_bin;
  EmuTIA88::vrom_banks[0] = koroleva_a2_bin; EmuTIA88::vrom_banks[1] = koroleva_a3_bin;
  EmuTIA88::vrom_banks[2] = koroleva_a5_bin; EmuTIA88::vrom_banks[3] = koroleva_a6_bin;
  emu_loops = 250;
}

inline void load_sos(void) {
  set_emu_tia88(false);
  EmuTIA88::pmem_banks[0] = sos_g1_bin; EmuTIA88::pmem_banks[1] = sos_g2_bin;
  EmuTIA88::pmem_banks[2] = sos_g3_bin; EmuTIA88::pmem_banks[3] = sos_g4_bin;
  EmuTIA88::pmem_banks[4] = sos_g5_bin; EmuTIA88::pmem_banks[6] = sos_g7_bin;
  EmuTIA88::vrom_banks[0] = sos_a2_bin; EmuTIA88::vrom_banks[1] = sos_a3_bin;
  EmuTIA88::vrom_banks[2] = sos_a5_bin; EmuTIA88::vrom_banks[3] = sos_a6_bin;
  emu_loops = 100;
}

inline void load_kotrybol(void) {
  set_emu_tia91();
  EmuTIA91::srom_banks[0] = kotrybol_850_bin; EmuTIA91::srom_banks[1] = kotrybol_851_bin;
  EmuTIA91::srom_banks[2] = kotrybol_852_bin; EmuTIA91::srom_banks[3] = kotrybol_853_bin;
  EmuTIA91::pmem_banks[0] = kotrybol_854_bin; EmuTIA91::pmem_banks[1] = kotrybol_855_bin;
  EmuTIA91::pmem_banks[2] = kotrybol_856_bin;
  EmuTIA91::crom_banks[0] = kotrybol_846_bin; EmuTIA91::crom_banks[1] = kotrybol_847_bin;
  EmuTIA91::crom_banks[2] = kotrybol_848_bin; EmuTIA91::crom_banks[3] = kotrybol_849_bin;
  emu_loops = 100;
}

inline void load_game(int id) {
  memset(shared_ram, 0, 20480);

  switch (id) {
    case GAME_KONEK: load_konek(); break;
    case GAME_KOROLEVA: load_koroleva(); break;
    case GAME_SOS: load_sos(); break;
    case GAME_GORODKI: load_gorodki(); break;
    case GAME_BILYARD: load_bilyard(); break;
    case GAME_KOT_RYBOLOV: load_kotrybol(); break;
  }

  switch (emu_core) {
    case EMU_TIA88: EmuTIA88::reset(); break;
    case EMU_TIA91: EmuTIA91::reset(); break;
  }
}

inline int emulate(uint8_t keys, int frames) {
  int t = 0;
  if (emu_loops < frames) emu_loops = frames;
  IOList::clear();

  while (emu_loops > 0) {
    switch (emu_core) {
      case EMU_TIA88: t += EmuTIA88::update(keys); break;
      case EMU_TIA91: t += EmuTIA91::update(keys); break;
    }
    --emu_loops;
    if (emu_loops > 0) { ESP.wdtFeed(); yield(); }
  }
  return t;
}

inline void render_video_scanline(int y, uint16_t* buffer) {
    switch (emu_core) {
        case EMU_TIA88: EmuTIA88::render_scanline(y, buffer); break;
        case EMU_TIA91: EmuTIA91::render_scanline(y, buffer); break;
    }
}
