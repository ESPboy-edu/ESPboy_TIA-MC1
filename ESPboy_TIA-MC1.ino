// S.A.M.E. (Soviet Arcade Machine Emulator) CC-BY shiru8bit (shiru@mail.ru)
// ESP8266/ESPboy port adapted for TIA-88 / TIA-91 architectures

#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include "nbSPI.h"
#include "data/all_roms.h" 
/*
#include "lib/ESPboyTerminalGUI.h"
#include "lib/ESPboyTerminalGUI.cpp"
#include "lib/ESPboyOTA2.h"
#include "lib/ESPboyOTA2.cpp"
*/

 
// 20 КБ ОЗУ (достаточно для TIA-88 и TIA-91)
uint8_t *shared_ram;
uint16_t *line_buffer1;
uint16_t *line_buffer2;

int emu_core;
int emu_loops;

ESPboyInit myESPboy;

#include "EmuMain.h"

#define LINES_PER_CHUNK 32

const int NUM_GAMES = 6;
const char* game_names[NUM_GAMES] = {
    "Konek (TIA-88)",
    "Koroleva (TIA-88)",
    "SOS (TIA-88)",
    "Gorodki (TIA-88)",
    "Bilyard (TIA-88)",
    "Kot Rybolov (TIA-91)"
};

int selected_game_index = 0;
bool in_menu = true;
bool menu_needs_full_redraw = true;

void drawMenuFull();
void drawMenuItem(int index, bool selected);
void handleMenu();
void runEmulatorFrame();

void setup() {
    myESPboy.begin("TIA-MC1 USSR Arcade");
/*
  //Check OTA2
  if (myESPboy.getKeys()&PAD_ACT || myESPboy.getKeys()&PAD_ESC) { 
     ESPboyTerminalGUI *terminalGUIobj = new ESPboyTerminalGUI(&myESPboy.tft, &myESPboy.mcp);
     ESPboyOTA2 *OTA2obj = new ESPboyOTA2(terminalGUIobj);
  }
*/
    shared_ram = (uint8_t *)malloc(20480); 
    line_buffer1 = (uint16_t *)malloc(2*128*LINES_PER_CHUNK);
    line_buffer2 = (uint16_t *)malloc(2*128*LINES_PER_CHUNK);
    ESP.wdtDisable(); 
}

void loop() {
    if (in_menu) {
        handleMenu();
    } else {
        runEmulatorFrame();
    }
}

void drawMenuFull() {
    myESPboy.tft.fillScreen(TFT_BLACK);
    myESPboy.tft.setTextColor(TFT_YELLOW);
    myESPboy.tft.setTextSize(1);
    myESPboy.tft.drawString(F("TIA ARCADE EMULATOR"), 5, 5);
    myESPboy.tft.drawLine(0, 15, 128, 15, TFT_BLUE);
    myESPboy.tft.setTextColor(TFT_WHITE);
    for (int i = 0; i < NUM_GAMES; i++) {
        drawMenuItem(i, i == selected_game_index);
    }
}

void drawMenuItem(int index, bool selected) {
    int y_pos = 20 + (index * 12);
    if (selected) {
        myESPboy.tft.fillRect(0, y_pos, 128, 12, TFT_BLUE);
        myESPboy.tft.setTextColor(TFT_YELLOW);
    } else {
        myESPboy.tft.fillRect(0, y_pos, 128, 12, TFT_BLACK); 
        myESPboy.tft.setTextColor(TFT_WHITE);
    }
    myESPboy.tft.drawString(game_names[index], 5, y_pos + 2);
}

void handleMenu() {
    if (menu_needs_full_redraw) {
        drawMenuFull();
        menu_needs_full_redraw = false;
    }

    static uint8_t prev_keys = 0;
    uint8_t keys = myESPboy.getKeys();
    
    if (keys != prev_keys) {
        int old_index = selected_game_index;
        bool changed = false;

        if (keys & PAD_UP) {
            selected_game_index--;
            if (selected_game_index < 0) selected_game_index = NUM_GAMES - 1;
            changed = true;
        }
        if (keys & PAD_DOWN) {
            selected_game_index++;
            if (selected_game_index >= NUM_GAMES) selected_game_index = 0;
            changed = true;
        }
        
        if (changed) {
            drawMenuItem(old_index, false); 
            drawMenuItem(selected_game_index, true); 
        }

        if (keys & PAD_ACT) {
            myESPboy.tft.fillScreen(TFT_BLACK);
            myESPboy.tft.setTextColor(TFT_YELLOW);
            
            // Обернули все строки подсказки в F()
            myESPboy.tft.drawString(F("After the game starts"), 0, 35);
            myESPboy.tft.drawString(F("drop a coin"), 0, 45);
            myESPboy.tft.drawString(F("by pressing"), 0, 55);
            
            myESPboy.tft.setTextColor(TFT_RED);
            myESPboy.tft.drawString(F("the LFT button"), 0, 65);
            
            myESPboy.tft.setTextColor(TFT_YELLOW);
            myESPboy.tft.drawString(F("to begin playing"), 0, 75);
            
            delay(3000); 
            
            myESPboy.tft.fillScreen(TFT_BLACK); 

            load_game(selected_game_index); 
            in_menu = false;
        }
        prev_keys = keys;
    }
    delay(50);
    ESP.wdtFeed();
}

void runEmulatorFrame() {
    uint8_t keys = myESPboy.getKeys();
    
    // Выход в меню теперь по одновременному нажатию A + B
    if ((keys & PAD_ACT) && (keys & PAD_ESC)) {
        while(nbSPI_isBusy()) { yield(); } 
        myESPboy.noPlayTone();
        in_menu = true;
        menu_needs_full_redraw = true; 
        return;
    }

    emulate(keys, 1); 

    static uint8_t frame_skip = 0;
    frame_skip++;
    
    if (frame_skip & 1) {
        ESP.wdtFeed();
        return; 
    }

    int total_lines = 128; 
    int chunks = total_lines / LINES_PER_CHUNK;
    
    while(nbSPI_isBusy()) { yield(); } 
    myESPboy.tft.setAddrWindow(0, 0, 128, 128); 
    
    for (int chunk = 0; chunk < chunks; ++chunk) {
        int start_y = chunk * LINES_PER_CHUNK;
        
        uint16_t* chunk_buffer;
        if (chunk_buffer == line_buffer1) chunk_buffer = line_buffer2;
        else chunk_buffer = line_buffer1;

        uint16_t* ptr_for_this_line = chunk_buffer;
        
        for (int i = 0; i < LINES_PER_CHUNK; ++i) {
            render_video_scanline(start_y + i, ptr_for_this_line);
            ptr_for_this_line = (uint16_t*)((uint32_t)ptr_for_this_line + 256);
        }
        
        while(nbSPI_isBusy()) { yield(); } 
        
        nbSPI_writeBytes((uint8_t *)chunk_buffer, 128 * LINES_PER_CHUNK * 2);
        
    }
    ESP.wdtFeed(); 
}
