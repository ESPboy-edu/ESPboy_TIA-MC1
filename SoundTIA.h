#pragma once

namespace SoundTIA
{

unsigned char pit_load_mode[4];
unsigned char pit_load_flip[4];
unsigned short int pit_counter[4];
unsigned char pit_count_mode[4];
int sound_audible_channel;

inline void init(int ch) {
  sound_audible_channel = ch;
}

inline void reset(void) {
  for (int i = 0; i < 4; ++i) {
    pit_load_mode[i] = 0; pit_load_flip[i] = 0;
    pit_counter[i] = 0; pit_count_mode[i] = 0;
  }
  myESPboy.noPlayTone();
}

inline void update_tone() {
    uint32_t freq = 1750000 / (pit_counter[sound_audible_channel] + 1) / 2;
    if (freq > 20 && freq < 10000 && (pit_count_mode[sound_audible_channel] == 3 || pit_count_mode[sound_audible_channel] == 2)) {
        myESPboy.playTone(freq);
    } else {
        myESPboy.noPlayTone();
    }
}

inline void write(int time, unsigned char reg, unsigned char val) {
  switch (reg) {
    case 0xc0: case 0xc1: case 0xc2: {
        int ch = reg & 3;
        if (!pit_load_flip[ch]) {
            pit_counter[ch] = (pit_counter[ch] & 0xff00) | val;
        } else {
            pit_counter[ch] = (pit_counter[ch] & 0x00ff) | (val << 8);
        }
        if (pit_load_mode[ch] == 3) pit_load_flip[ch] ^= 1;

        if (ch == sound_audible_channel) update_tone();
        break;
    }
    case 0xc3: {
        int ch = val >> 6;
        if (ch < 4) {
            pit_count_mode[ch] = (val >> 1) & 7;
            pit_load_mode[ch] = (val >> 4) & 3;
            pit_load_flip[ch] = (pit_load_mode[ch] == 2) ? 1 : 0;
            
            if (ch == sound_audible_channel) update_tone();
        }
        break;
    }
  }
}
}