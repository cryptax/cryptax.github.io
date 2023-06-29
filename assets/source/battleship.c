/*
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include "battleship.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hardware.h"
#include "ws2812.h"
#include "pax_gfx.h"
#include "pax_codecs.h"

/*   A mini-version of the well known game "Battleship"
 *   But with a twist!
 *
 *   Written by Dennis Mantz (ERNW). 2023
 */

#define VERSION "v1.0"

// Colors & Font Sizes
#define COLOR_FG            0xFFFFFFFF
#define COLOR_BG            0xFF1E1E1E
#define COLOR_HEADER        0xFF333333
#define COLOR_HEADING       0xFFFF9933
#define COLOR_FOOTER        0xFF131313
#define COLOR_SHIP          0x80C0C0C0
#define COLOR_HOVER         0x8000FFFF
#define COLOR_HOVER_INVALID 0x80FF0000
#define COLOR_MSGBOX        0xFF3E3E3E
#define COLOR_MSGBOX_BORDER 0xFF4E4E4E
#define HEADING_FONT_SIZE   22

// Game Constants
#define FIELD_COUNT_X 12
#define FIELD_COUNT_Y 6
#define FIELD_COUNT   72 //6*12
#define SHIP_COUNT    4 // one 5-field, one 4-field, one 3-field and one 2-field ships
#define SHIP_SIZES    (int[]){5, 4, 3, 2}
#define MAX_SHIP_FIELDS 14 // 5 + 4 + 3 + 2
#define SLEEP_DELAY_MS 1000

// Screen related constants (pixel)
#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240
#define CELL_SIZE        16
#define HEADER_HEIGHT    24
#define FOOTER_HEIGHT    20
#define INFOBOX_X        (FIELD_COUNT_X*CELL_SIZE)
#define INFOBOX_X_CENTER (INFOBOX_X + (SCREEN_WIDTH-INFOBOX_X)/2)
#define INFOBOX_Y        HEADER_HEIGHT
#define MSGBOX_X         40
#define MSGBOX_Y         40
#define MSGBOX_WIDTH     (SCREEN_WIDTH-2*MSGBOX_X)
#define MSGBOX_HEIGHT    (SCREEN_HEIGHT-2*MSGBOX_Y)
#define COMPUTER_GRID_Y  HEADER_HEIGHT
#define PLAYER_GRID_Y    (COMPUTER_GRID_Y + FIELD_COUNT_Y*CELL_SIZE + 4)

// Game States
#define GAMESTATE_START_SCREEN    0
#define GAMESTATE_PLACE_SHIPS     1
#define GAMESTATE_PLAYER_TURN     2
#define GAMESTATE_COMPUTER_TURN   3
#define GAMESTATE_WIN             4
#define GAMESTATE_LOSE            5
#define GAMESTATE_ASK_END_GAME    6
#define GAMESTATE_HIDDEN_FLAG     7

// Computer States
#define COMPUTER_STATE_RANDOM         0
#define COMPUTER_STATE_SEARCH_RIGHT   1
#define COMPUTER_STATE_SEARCH_LEFT    2
#define COMPUTER_STATE_SEARCH_TOP     3
#define COMPUTER_STATE_SEARCH_BOTTOM  4

// PNG Icons
extern const uint8_t qrcode_png_start[] asm("_binary_qrcode_png_start");
extern const uint8_t qrcode_png_end[] asm("_binary_qrcode_png_end");
extern const uint8_t empty_png_start[] asm("_binary_empty_png_start");
extern const uint8_t empty_png_end[] asm("_binary_empty_png_end");
extern const uint8_t fire_png_start[] asm("_binary_fire_png_start");
extern const uint8_t fire_png_end[] asm("_binary_fire_png_end");
extern const uint8_t miss_png_start[] asm("_binary_miss_png_start");
extern const uint8_t miss_png_end[] asm("_binary_miss_png_end");
extern const uint8_t miss_last_png_start[] asm("_binary_miss_last_png_start");
extern const uint8_t miss_last_png_end[] asm("_binary_miss_last_png_end");
extern const uint8_t ship_orange_png_start[] asm("_binary_ship_orange_png_start");
extern const uint8_t ship_orange_png_end[] asm("_binary_ship_orange_png_end");
extern const uint8_t ship_sunken_png_start[] asm("_binary_ship_sunken_png_start");
extern const uint8_t ship_sunken_png_end[] asm("_binary_ship_sunken_png_end");
extern const uint8_t ship_black_png_start[] asm("_binary_ship_black_png_start");
extern const uint8_t ship_black_png_end[] asm("_binary_ship_black_png_end");
extern const uint8_t ship_black_vertical_png_start[] asm("_binary_ship_black_vertical_png_start");
extern const uint8_t ship_black_vertical_png_end[] asm("_binary_ship_black_vertical_png_end");

pax_buf_t icon_qrcode;
pax_buf_t icon_empty;
pax_buf_t icon_fire;
pax_buf_t icon_miss;
pax_buf_t icon_miss_last;
pax_buf_t icon_ship_orange;
pax_buf_t icon_ship_sunken;
pax_buf_t icon_ship_black;
pax_buf_t icon_ship_black_vertical;

typedef struct ship {
    uint8_t field;
    uint8_t size;
    bool horizontal;
    bool placed;
    bool sunken;
} Ship;

typedef struct game_state {
    int8_t cursor;
    Ship computer_ships[SHIP_COUNT];
    Ship player_ships[SHIP_COUNT];
    uint8_t computer_missed_shots_idx;
    uint8_t computer_missed_shots[FIELD_COUNT];
    uint8_t player_missed_shots_idx;
    uint8_t player_missed_shots[FIELD_COUNT];
    uint8_t computer_hits_idx;
    uint8_t computer_hits[MAX_SHIP_FIELDS];
    uint8_t player_hits_idx;
    uint8_t player_hits[MAX_SHIP_FIELDS];
    int round;
    int state;
    int saved_state;
    int computer_state;
    char msg[100];
} GameState;

//********************************************************************************
//* Helper Functions                                                             *
//********************************************************************************

// Load icons from memory into PAX buffers
void init_icons() {
    pax_decode_png_buf(&icon_qrcode, (void*) qrcode_png_start, qrcode_png_end - qrcode_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_empty, (void*) empty_png_start, empty_png_end - empty_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_fire, (void*) fire_png_start, fire_png_end - fire_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_miss, (void*) miss_png_start, miss_png_end - miss_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_miss_last, (void*) miss_last_png_start, miss_last_png_end - miss_last_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_ship_orange, (void*) ship_orange_png_start, ship_orange_png_end - ship_orange_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_ship_sunken, (void*) ship_sunken_png_start, ship_sunken_png_end - ship_sunken_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_ship_black, (void*) ship_black_png_start, ship_black_png_end - ship_black_png_start, PAX_BUF_16_4444ARGB, 0);
    pax_decode_png_buf(&icon_ship_black_vertical, (void*) ship_black_vertical_png_start, ship_black_vertical_png_end - ship_black_vertical_png_start, PAX_BUF_16_4444ARGB, 0);
}


const uint8_t led_off[27] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t led_orange[27] = {30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0, 30, 30, 0};
const uint8_t led_red[27] = {60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0, 60, 0, 0};
const uint8_t led_rainbow[27] = {255, 0, 0, 255, 128, 0, 255, 255, 0, 64, 255, 0, 0, 255, 128, 0, 192, 255, 0, 0, 255, 128, 0, 255, 255, 0, 192};

// set led colors
void set_leds(const uint8_t leds[]) {
    int i;
    uint8_t leds_random[27];
    if(leds != NULL){
        ws2812_send_data(leds, sizeof(led_off));
    } else {
        for(i=0; i<sizeof(leds_random); i++)
            leds_random[i] = esp_random() % 50;
        ws2812_send_data(leds_random, sizeof(leds_random));
    }
}

/* Game Grid (field numbers):
 * X Y--->
 * | 00 06 12 18 24 30 36 42 48 54 60 66
 * | 01 07 13 19 25 31 37 43 49 55 61 67
 * v 02 08 14 20 26 32 38 44 50 56 62 68
 *   03 09 15 21 27 33 39 45 51 57 63 69
 *   04 10 16 22 28 34 40 46 52 58 64 70
 *   05 11 17 23 29 35 41 47 53 59 65 71
 */

// Get the X Coordinate (0-11) from the field number (0-71)
int get_x(uint8_t field_num) {
    return field_num/FIELD_COUNT_Y;
}

// Get the Y Coordinate (0-5) from the field number (0-71)
int get_y(uint8_t field_num) {
    return field_num%FIELD_COUNT_Y;
}

// Get the n-th field of a ship
uint8_t get_ship_field(Ship* ship, uint8_t field_number) {
    if(ship->horizontal) {
        return ship->field + field_number*FIELD_COUNT_Y;
    }
    return ship->field + field_number;
}

// Returns true if the list (of length 'len') contains the value
bool list_contains(uint8_t list[], int len, uint8_t value) {
    int i;
    for(i=0; i<len; i++) {
        if(list[i] == value)
            return true;
    }
    return false;
}

// Returns true if the 'ship' is placed within the game grid and
// does not touch any placed ship
bool is_ship_placement_valid(Ship ships[], int ships_size, Ship* ship) {
    int i, field_idx, ship_idx, checked_field, current_field;
    // check if ship goes out of bounds:
    if(ship->horizontal) {
        if(ship->field + (ship->size - 1)*FIELD_COUNT_Y > FIELD_COUNT-1)
            return false;
    } else {
        if((ship->field + ship->size - 1)/FIELD_COUNT_Y > ship->field/FIELD_COUNT_Y)
            return false;
    }
    // For each ship field ...
    for(field_idx=0; field_idx<ship->size; field_idx++) {
        current_field = get_ship_field(ship, field_idx);
        // ... check for each existing ship ...
        for(ship_idx=0; ship_idx < ships_size; ship_idx++) {
            if(!ships[ship_idx].placed)
                continue;
            // ... if it overlaps with the ship field ...
            for(i=0; i < ships[ship_idx].size; i++) {
                checked_field = get_ship_field(&ships[ship_idx], i);
                if(checked_field == current_field) // .. itself
                    return false;
                if(checked_field == current_field-FIELD_COUNT_Y) // .. on the right
                    return false;
                if(checked_field == current_field+FIELD_COUNT_Y) // .. on the left
                    return false;
                if(current_field%FIELD_COUNT_Y != 0 && checked_field == current_field-1) // .. on the top
                    return false;
                if(current_field%FIELD_COUNT_Y != FIELD_COUNT_Y-1 && checked_field == current_field+1) // .. on the bottom
                    return false;
            }
        }
    }
    return true;
}

// Returns true if all fields of the ship are contained in the hits[] array
// (which means the ship has sunken)
bool all_fields_of_ship_destroyed(Ship* ship, uint8_t hits[], uint8_t hits_size) {
    uint8_t hits_idx, ship_field_idx;
    for(ship_field_idx=0; ship_field_idx < ship->size; ship_field_idx++) {
        for(hits_idx=0; hits_idx < hits_size; hits_idx++) {
            if(get_ship_field(ship, ship_field_idx) == hits[hits_idx])
                break;
        }
        if(hits_idx == hits_size) // Current ship field was not hit
            break;
    }
    return ship_field_idx == ship->size; // All fields were hit
}

// Checks whether the given field number contains a ship
// Returns a pointer to that ship
// Argument player: If true, check the player_ships, otherwise the
// computer_ships.
Ship* get_ship_from_field(GameState* gamestate, uint8_t field, bool player) {
    int i;
    uint8_t ship_field;
    Ship* ship;
    for(i=0; i<SHIP_COUNT; i++) {
        if(player)
            ship = &gamestate->player_ships[i];
        else
            ship = &gamestate->computer_ships[i];

        for(ship_field=0; ship_field < ship->size; ship_field++) {
            if(get_ship_field(ship, ship_field) == field)
                return ship;
        }
    }
    return NULL;
}

// Randomly place all computer ships on the grid
void place_computer_ships(GameState* gamestate) {
    int i;
    for(i=0; i<SHIP_COUNT; i++) {
        while(true) {
            gamestate->computer_ships[i].horizontal = esp_random()%2;
            gamestate->computer_ships[i].field = esp_random() % FIELD_COUNT;
            if(is_ship_placement_valid(gamestate->computer_ships, SHIP_COUNT, &gamestate->computer_ships[i]))
                break;
        }
        gamestate->computer_ships[i].sunken = false;
        gamestate->computer_ships[i].placed = true;

        //printf("Placing computer %d-ship at %d (%s)\n",
        //        gamestate->computer_ships[i].size,
        //        gamestate->computer_ships[i].field,
        //        gamestate->computer_ships[i].horizontal ? "horizontal" : "vertical");
    }
}

// Reset the game state and place all computer ships
void new_game(GameState* gamestate) {
    int i;
    // Initialize game state
    memset(gamestate, 0, sizeof(*gamestate));
    for(i=0; i<SHIP_COUNT; i++) {
        gamestate->player_ships[i].horizontal = true;
        gamestate->player_ships[i].size = SHIP_SIZES[i];
        gamestate->computer_ships[i].size = SHIP_SIZES[i];
    }
    place_computer_ships(gamestate);
}


//********************************************************************************
//* RENDERING                                                                    *
//********************************************************************************

void render_background(pax_buf_t* pax_buffer) {
    int i;
    const pax_font_t* font = pax_font_saira_regular;
    const char* heading = "Troopers 2023 Battleship";
    pax_vec1_t heading_size = pax_text_size(font, HEADING_FONT_SIZE, heading);

    pax_background(pax_buffer, COLOR_BG);
    pax_noclip(pax_buffer);

    // Header
    pax_simple_rect(pax_buffer, COLOR_HEADER, 0, 0, SCREEN_WIDTH, HEADER_HEIGHT);
    pax_draw_text(pax_buffer, COLOR_HEADING, font, HEADING_FONT_SIZE, (SCREEN_WIDTH - heading_size.x)/2, 2, heading);
    // Footer
    pax_simple_rect(pax_buffer, COLOR_FOOTER, 0, SCREEN_HEIGHT-FOOTER_HEIGHT, SCREEN_WIDTH, FOOTER_HEIGHT);

    // Grid
    for(i=0; i<FIELD_COUNT; i++) {
        pax_draw_image(pax_buffer, &icon_empty, get_x(i)*CELL_SIZE, COMPUTER_GRID_Y + get_y(i)*CELL_SIZE);
    }
    for(i=0; i<FIELD_COUNT; i++) {
        pax_draw_image(pax_buffer, &icon_empty, get_x(i)*CELL_SIZE, PLAYER_GRID_Y + get_y(i)*CELL_SIZE);
    }

    // Infobox
    pax_outline_rect(pax_buffer, COLOR_FG, INFOBOX_X, INFOBOX_Y, SCREEN_WIDTH-INFOBOX_X-1, SCREEN_HEIGHT-INFOBOX_Y-FOOTER_HEIGHT-1);
}

void draw_ship(pax_buf_t* pax_buffer, GameState* gamestate, Ship* ship, bool hover) {
    int i, current_field, color;
    for(i=0; i<ship->size; i++) {
        if(ship->horizontal) {
            current_field = ship->field + i*FIELD_COUNT_Y;
            if(current_field > FIELD_COUNT-1)
                current_field -= FIELD_COUNT;
        } else {
            current_field = ship->field + i;
            if(current_field/FIELD_COUNT_Y > ship->field/FIELD_COUNT_Y)
                current_field -= FIELD_COUNT_Y;
        }
        if(hover) {
            if(is_ship_placement_valid(gamestate->player_ships, SHIP_COUNT, ship)) {
                color = COLOR_HOVER;
            } else {
                color = COLOR_HOVER_INVALID;
            }
        } else {
            color = COLOR_SHIP;
        }
        pax_simple_rect(pax_buffer, color,
                get_x(current_field)*CELL_SIZE, PLAYER_GRID_Y +
                get_y(current_field)*CELL_SIZE, CELL_SIZE, CELL_SIZE);
    }
    // Draw ship image for placed ships:
    if(!hover) {
        if(ship->horizontal)
            pax_draw_image_sized(pax_buffer, &icon_ship_black,
                    get_x(ship->field)*CELL_SIZE, PLAYER_GRID_Y +
                    get_y(ship->field)*CELL_SIZE, ship->size*CELL_SIZE, CELL_SIZE);
        else
            pax_draw_image_sized(pax_buffer, &icon_ship_black_vertical,
                    get_x(ship->field)*CELL_SIZE, PLAYER_GRID_Y +
                    get_y(ship->field)*CELL_SIZE, CELL_SIZE, ship->size*CELL_SIZE);
    }
}

void draw_player_ships(pax_buf_t* pax_buffer, GameState* gamestate) {
    int i;
    for(i=0; i<SHIP_COUNT; i++) {
        if(gamestate->player_ships[i].placed)
            draw_ship(pax_buffer, gamestate, &gamestate->player_ships[i], false);
    }
}

void draw_hits_and_misses(pax_buf_t* pax_buffer, GameState* gamestate) {
    int i;
    pax_buf_t* icon;
    for(i=0; i<gamestate->player_hits_idx; i++) {
        pax_draw_image(pax_buffer, &icon_fire,
                get_x(gamestate->player_hits[i])*CELL_SIZE,
                COMPUTER_GRID_Y + get_y(gamestate->player_hits[i])*CELL_SIZE);
    }
    for(i=0; i<gamestate->player_missed_shots_idx; i++) {
        if(i==gamestate->player_missed_shots_idx-1)
            icon = &icon_miss_last;
        else
            icon = &icon_miss;
        pax_draw_image(pax_buffer, icon,
                get_x(gamestate->player_missed_shots[i])*CELL_SIZE,
                COMPUTER_GRID_Y + get_y(gamestate->player_missed_shots[i])*CELL_SIZE);
    }
    for(i=0; i<gamestate->computer_hits_idx; i++) {
        pax_draw_image(pax_buffer, &icon_fire,
                get_x(gamestate->computer_hits[i])*CELL_SIZE,
                PLAYER_GRID_Y + get_y(gamestate->computer_hits[i])*CELL_SIZE);
    }
    for(i=0; i<gamestate->computer_missed_shots_idx; i++) {
        if(i==gamestate->computer_missed_shots_idx-1)
            icon = &icon_miss_last;
        else
            icon = &icon_miss;
        pax_draw_image(pax_buffer, icon,
                get_x(gamestate->computer_missed_shots[i])*CELL_SIZE,
                PLAYER_GRID_Y + get_y(gamestate->computer_missed_shots[i])*CELL_SIZE);
    }
}

void draw_status(pax_buf_t* pax_buffer, GameState* gamestate) {
    char textbuffer[200];
    pax_buf_t* icon_player;
    pax_buf_t* icon_computer;
    int i, x_size, x, y;
    const pax_font_t* font = pax_font_saira_regular;

    snprintf(textbuffer, 200, "-PLAYER-  HITS: %d", gamestate->player_hits_idx);
    pax_draw_text(pax_buffer, COLOR_HEADING, font, 12, INFOBOX_X + 5, INFOBOX_Y + 4, textbuffer);
    snprintf(textbuffer, 200, "Computer  HITS: %d", gamestate->computer_hits_idx);
    pax_draw_text(pax_buffer, COLOR_HEADING, font, 12, INFOBOX_X + 5, INFOBOX_Y + 10 + SHIP_COUNT*15, textbuffer);
    snprintf(textbuffer, 200, "Round: #%d", gamestate->round);
    pax_draw_text(pax_buffer, COLOR_HEADING, font, 12, INFOBOX_X + 5, INFOBOX_Y + 135, textbuffer);
    for(i=0; i<SHIP_COUNT; i++) {
        icon_player   = gamestate->player_ships[i].sunken ? &icon_ship_sunken : &icon_ship_orange;
        icon_computer = gamestate->computer_ships[i].sunken ? &icon_ship_sunken : &icon_ship_orange;
        x_size = gamestate->player_ships[i].size*CELL_SIZE;
        if(i < SHIP_COUNT-1) {
            x = INFOBOX_X + 5;
            y = INFOBOX_Y + 16 + i*CELL_SIZE;
        } else {
            x = INFOBOX_X + 10 + CELL_SIZE*3;
            y = INFOBOX_Y + 16 + (i-1)*CELL_SIZE;
        }
        if(gamestate->player_ships[i].placed)
            pax_draw_image_sized(pax_buffer, icon_player, x, y + 65, x_size, CELL_SIZE);
        pax_draw_image_sized(pax_buffer, icon_computer, x, y, x_size, CELL_SIZE);
    }
}

void render_screen(pax_buf_t* pax_buffer, GameState* gamestate) {
    const pax_font_t* font = pax_font_saira_regular;
    int i;

    render_background(pax_buffer);
    draw_player_ships(pax_buffer, gamestate);
    draw_hits_and_misses(pax_buffer, gamestate);

    // State specific drawing
    switch(gamestate->state) {
        case GAMESTATE_START_SCREEN:
            pax_simple_rect(pax_buffer, COLOR_BG, 0, COMPUTER_GRID_Y, SCREEN_WIDTH, SCREEN_HEIGHT-HEADER_HEIGHT-FOOTER_HEIGHT);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, COMPUTER_GRID_Y+10, "Welcome to Mini Battleship!");
            pax_draw_image_sized(pax_buffer, &icon_ship_orange, SCREEN_WIDTH/2-50, COMPUTER_GRID_Y+40, 100, 30);
            pax_draw_text(pax_buffer, COLOR_FG, font, 14, SCREEN_WIDTH-95, SCREEN_HEIGHT-FOOTER_HEIGHT-120, "Source Code:");
            pax_draw_image(pax_buffer, &icon_qrcode, SCREEN_WIDTH-95, SCREEN_HEIGHT-FOOTER_HEIGHT-105);
            pax_center_text(pax_buffer, COLOR_FG, font, 14, SCREEN_WIDTH/2, SCREEN_HEIGHT-FOOTER_HEIGHT-14, "https://battleship.challenge.troopers.de/");
            pax_center_text(pax_buffer, COLOR_HEADING, font, 16, SCREEN_WIDTH/3, SCREEN_HEIGHT-100, "Created By");
            pax_center_text(pax_buffer, COLOR_HEADING, font, 16, SCREEN_WIDTH/3, SCREEN_HEIGHT-80, "Dennis Mantz");
            pax_center_text(pax_buffer, COLOR_FG, font, 14, SCREEN_WIDTH/3, SCREEN_HEIGHT-60, VERSION);
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🅰 start game   🅱/🆂 leave");
            break;
        case GAMESTATE_PLACE_SHIPS:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 3, SCREEN_HEIGHT - 18, "🅰place 🅱rotate [R]andom 🆂end");

            // draw hovering ship
            for(i=0; i<SHIP_COUNT; i++) {
                if(!gamestate->player_ships[i].placed) {
                    draw_ship(pax_buffer, gamestate, &gamestate->player_ships[i], true);
                    break;
                }
            }
            // draw cursor
            pax_outline_rect(pax_buffer, COLOR_FG,
                    get_x(gamestate->cursor)*CELL_SIZE, 
                    PLAYER_GRID_Y + get_y(gamestate->cursor)*CELL_SIZE, CELL_SIZE, CELL_SIZE);
            //status
            draw_status(pax_buffer, gamestate);
            // info text
            pax_center_text(pax_buffer, COLOR_FG, font, 12, INFOBOX_X_CENTER, 180, "place your ships!");
            pax_center_text(pax_buffer, COLOR_FG, font, 12, INFOBOX_X_CENTER, 195, "(ships cannot");
            pax_center_text(pax_buffer, COLOR_FG, font, 12, INFOBOX_X_CENTER, 207, "touch!)");
            break;
        case GAMESTATE_PLAYER_TURN:
            // status
            draw_status(pax_buffer, gamestate);
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🅰 fire 🆂 end game");
            // draw cursor
            pax_outline_rect(pax_buffer, COLOR_FG,
                    get_x(gamestate->cursor)*CELL_SIZE,
                    COMPUTER_GRID_Y + get_y(gamestate->cursor)*CELL_SIZE, CELL_SIZE, CELL_SIZE);
            // draw message text
            pax_center_text(pax_buffer, COLOR_FG, font, 12, INFOBOX_X_CENTER, 195, gamestate->msg);
            break;
        case GAMESTATE_COMPUTER_TURN:
            // status
            draw_status(pax_buffer, gamestate);
            // draw message text
            pax_center_text(pax_buffer, COLOR_FG, font, 12, INFOBOX_X_CENTER, 195, gamestate->msg);
            break;
        case GAMESTATE_WIN:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🆂 new game");
            // Win Screen
            pax_simple_rect(pax_buffer, COLOR_MSGBOX_BORDER, MSGBOX_X, MSGBOX_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
            pax_simple_rect(pax_buffer, COLOR_MSGBOX, MSGBOX_X+5, MSGBOX_Y+5, MSGBOX_WIDTH-10, MSGBOX_HEIGHT-10);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-16, "You WIN!");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+30, "But there is more!");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+50, "Look at the source!");
            break;
        case GAMESTATE_LOSE:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🆂 new game");
            // Lose Screen
            pax_simple_rect(pax_buffer, COLOR_MSGBOX_BORDER, MSGBOX_X, MSGBOX_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
            pax_simple_rect(pax_buffer, COLOR_MSGBOX, MSGBOX_X+5, MSGBOX_Y+5, MSGBOX_WIDTH-10, MSGBOX_HEIGHT-10);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-16, "You lost!");
            pax_draw_image(pax_buffer, &icon_fire, SCREEN_WIDTH/2-8, SCREEN_HEIGHT/2+16);
            break;
        case GAMESTATE_ASK_END_GAME:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🅰 end game 🅱 resume game");
            // Ask Screen
            pax_simple_rect(pax_buffer, COLOR_MSGBOX_BORDER, MSGBOX_X, MSGBOX_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
            pax_simple_rect(pax_buffer, COLOR_MSGBOX, MSGBOX_X+5, MSGBOX_Y+5, MSGBOX_WIDTH-10, MSGBOX_HEIGHT-10);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-16, "Do you really want");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "to end the game?");
            break;
        case GAMESTATE_HIDDEN_FLAG:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🅰 win 🅱 lose 🆂 new game");
            // msg box
            pax_simple_rect(pax_buffer, COLOR_MSGBOX_BORDER, MSGBOX_X, MSGBOX_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
            pax_simple_rect(pax_buffer, COLOR_MSGBOX, MSGBOX_X+5, MSGBOX_Y+5, MSGBOX_WIDTH-10, MSGBOX_HEIGHT-10);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-60, "Good job!");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-40, "Here is your flag:");
            printf("You don't need to dump the firmware, to read the flag! The bug can be exploited within the game!\n");
            pax_center_text(pax_buffer, COLOR_HEADING, font, 22, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "XXXX-XXXX-XXXX-");
            pax_center_text(pax_buffer, COLOR_HEADING, font, 22, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+20, "XXXX-XXXX");
            break;
        default:
            // Footer
            pax_draw_text(pax_buffer, COLOR_FG, font, 18, 5, SCREEN_HEIGHT - 18, "🆂 new game");
            // msg box
            pax_simple_rect(pax_buffer, COLOR_MSGBOX_BORDER, MSGBOX_X, MSGBOX_Y, MSGBOX_WIDTH, MSGBOX_HEIGHT);
            pax_simple_rect(pax_buffer, COLOR_MSGBOX, MSGBOX_X+5, MSGBOX_Y+5, MSGBOX_WIDTH-10, MSGBOX_HEIGHT-10);
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-25, "You broke it. NICE!");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, "Can you find the");
            pax_center_text(pax_buffer, COLOR_FG, font, 18, SCREEN_WIDTH/2, SCREEN_HEIGHT/2+25, "hidden flag?");
            break;
    }

    display_flush();
}


//********************************************************************************
//* STATE MACHINE (Game Logic)                                                   *
//********************************************************************************

void handle_START_SCREEN(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_START_SCREEN\n");
    if(input_valid) {
        if(input == BUTTON_ACCEPT) {
            gamestate->state = GAMESTATE_PLACE_SHIPS;
        }
    }
}

// In this state the player can place all available ships (select field with the
// joystick, then place the ship with button A. Button B rotates the ship 90 degrees).
void handle_PLACE_SHIPS(GameState* gamestate, bool input_valid, uint8_t input) {
    int current_ship_idx, i;

    for(current_ship_idx=0; current_ship_idx<SHIP_COUNT; current_ship_idx++) {
        if(!gamestate->player_ships[current_ship_idx].placed)
            break;
    }
    if(current_ship_idx >= SHIP_COUNT) {
        // All ships placed. Lets start
        gamestate->round = 1;
        gamestate->state = GAMESTATE_PLAYER_TURN;
        return;
    }

    if(input_valid) {
        switch (input) {
            case JOYSTICK_LEFT:
                gamestate->cursor -= FIELD_COUNT_Y;
                if(gamestate->cursor < 0)
                    gamestate->cursor += FIELD_COUNT;
                break;
            case JOYSTICK_RIGHT:
                gamestate->cursor += FIELD_COUNT_Y;
                if(gamestate->cursor > FIELD_COUNT-1)
                    gamestate->cursor -= FIELD_COUNT;
                break;
            case JOYSTICK_DOWN:
                if(gamestate->cursor % FIELD_COUNT_Y == 5)
                    gamestate->cursor -= FIELD_COUNT_Y-1;
                else
                    gamestate->cursor += 1;
                break;
            case JOYSTICK_UP:
                if(gamestate->cursor % FIELD_COUNT_Y == 0)
                    gamestate->cursor += FIELD_COUNT_Y-1;
                else
                    gamestate->cursor -= 1;
                break;
            case BUTTON_BACK:
                gamestate->player_ships[current_ship_idx].horizontal = !gamestate->player_ships[current_ship_idx].horizontal;
                break;
            case BUTTON_ACCEPT:
                if(is_ship_placement_valid(gamestate->player_ships, SHIP_COUNT, &gamestate->player_ships[current_ship_idx])) {
                    gamestate->player_ships[current_ship_idx].placed = true;
                    printf("placed ship (%d) in field %d\n",
                            gamestate->player_ships[current_ship_idx].size,
                            gamestate->player_ships[current_ship_idx].field);
                    if(current_ship_idx < SHIP_COUNT-1)
                        current_ship_idx++;
                } else {
                    printf("ship placement invalid\n");
                }
                break;
            case BUTTON_START:
                gamestate->saved_state = gamestate->state;
                gamestate->state = GAMESTATE_ASK_END_GAME;
                break;
            case KEY_R:
                printf("R KEY\n");
                for(i=0; i<SHIP_COUNT; i++) {
                    if(!gamestate->player_ships[i].placed) {
                        while(true) {
                            gamestate->player_ships[i].horizontal = esp_random()%2;
                            gamestate->player_ships[i].field = esp_random() % FIELD_COUNT;
                            if(is_ship_placement_valid(gamestate->player_ships, SHIP_COUNT, &gamestate->player_ships[i]))
                                break;
                        }
                        printf("placing at %d\n", gamestate->player_ships[i].field);
                        gamestate->player_ships[i].placed = true;
                    }
                }
                return;
            default:
                break;
        }
        gamestate->player_ships[current_ship_idx].field = gamestate->cursor;
    }
}

// In this state the player chooses a field to attack (button A).
// If the field was part of a ship its a hit otherwise a miss.
void handle_PLAYER_TURN(GameState* gamestate, bool input_valid, uint8_t input) {
    int ship_idx;
    Ship* target_ship;
    if(input_valid) {
        switch (input) {
            case JOYSTICK_LEFT:
                gamestate->cursor -= FIELD_COUNT_Y;
                if(gamestate->cursor < 0)
                    gamestate->cursor += FIELD_COUNT;
                break;
            case JOYSTICK_RIGHT:
                gamestate->cursor += FIELD_COUNT_Y;
                if(gamestate->cursor > FIELD_COUNT-1)
                    gamestate->cursor -= FIELD_COUNT;
                break;
            case JOYSTICK_DOWN:
                if(gamestate->cursor % FIELD_COUNT_Y == FIELD_COUNT_Y-1)
                    gamestate->cursor -= FIELD_COUNT_Y-1;
                else
                    gamestate->cursor += 1;
                break;
            case JOYSTICK_UP:
                if(gamestate->cursor % FIELD_COUNT_Y == 0)
                    gamestate->cursor += FIELD_COUNT_Y-1;
                else
                    gamestate->cursor -= 1;
                break;
            case BUTTON_ACCEPT:
                printf("fire: %d\n", gamestate->cursor);
                gamestate->state = GAMESTATE_COMPUTER_TURN;
                target_ship = get_ship_from_field(gamestate, gamestate->cursor, false);
                if(target_ship == NULL) {
                    printf("miss!\n");
                    sprintf(gamestate->msg, "You missed!");
                    gamestate->player_missed_shots[gamestate->player_missed_shots_idx++] = gamestate->cursor;
                } else {
                    printf("hit!\n");
                    sprintf(gamestate->msg, "You hit a ship!");
                    set_leds(led_orange);
                    gamestate->player_hits[gamestate->player_hits_idx++] = gamestate->cursor;
                    if(all_fields_of_ship_destroyed(target_ship, gamestate->player_hits, gamestate->player_hits_idx)) {
                        target_ship->sunken = true;
                        printf("SUNKEN (%d-ship)!\n", target_ship->size);
                        sprintf(gamestate->msg, "Comp. %d-ship sunk!", target_ship->size);
                        for(ship_idx=0; ship_idx<SHIP_COUNT; ship_idx++) {
                            if(!gamestate->computer_ships[ship_idx].sunken)
                                break;
                        }
                        if(ship_idx == SHIP_COUNT) {
                            printf("All computer ships have sunken! Player wins!\n");
                            gamestate->state = GAMESTATE_WIN;
                        }
                    }
                }
                break;
            case BUTTON_START:
                gamestate->saved_state = gamestate->state;
                gamestate->state = GAMESTATE_ASK_END_GAME;
                break;
            default:
                break;
        }
    }
}

// In this state the computer attacks a field of the player. (No inputs
// are accepted during this state). The computer uses the following algorithm:
// - Attack random field until it hits a ship
// - After a hit: Attack right of the hit coordinate if possible
// - If this fails (miss): Attack left of the hit coordinate if possible
// - If this fails attack top field, ... lastly try bottom fields
void handle_COMPUTER_TURN(GameState* gamestate, bool input_valid, uint8_t input) {
    bool made_my_choice = false;
    int8_t target_field = -1;
    Ship* target_ship;
    uint8_t ship_idx;
    printf("handle_COMPUTER_TURN (computer state=%d)\n", gamestate->computer_state);

    // start with the last hit
    if(gamestate->computer_state != COMPUTER_STATE_RANDOM)
        target_field = gamestate->computer_hits[gamestate->computer_hits_idx-1];

    while(!made_my_choice) {
        printf("no choice yet (computer state=%d  target_field=%d)\n", gamestate->computer_state, target_field);
        switch(gamestate->computer_state) {
            case COMPUTER_STATE_RANDOM:
                target_field = esp_random() % FIELD_COUNT;
                printf("random=%d\n", target_field);
                if(list_contains(gamestate->computer_hits, gamestate->computer_hits_idx, target_field))
                    continue;
                if(list_contains(gamestate->computer_missed_shots, gamestate->computer_missed_shots_idx, target_field))
                    continue;
                printf("accept random\n");
                made_my_choice = true;
                break;
            case COMPUTER_STATE_SEARCH_RIGHT:
                target_field += FIELD_COUNT_Y;
                if(    target_field > FIELD_COUNT-1
                    || list_contains(gamestate->computer_missed_shots, gamestate->computer_missed_shots_idx, target_field)
                    || list_contains(gamestate->computer_hits, gamestate->computer_hits_idx, target_field)) {

                    target_field = gamestate->computer_hits[gamestate->computer_hits_idx-1];
                    gamestate->computer_state = COMPUTER_STATE_SEARCH_LEFT;
                    continue;
                }
                made_my_choice = true;
                break;
            case COMPUTER_STATE_SEARCH_LEFT:
                target_field -= FIELD_COUNT_Y;
                if(    target_field < 0
                    || list_contains(gamestate->computer_missed_shots, gamestate->computer_missed_shots_idx, target_field)) {

                    target_field = gamestate->computer_hits[gamestate->computer_hits_idx-1];
                    gamestate->computer_state = COMPUTER_STATE_SEARCH_TOP;
                    continue;
                }
                if(list_contains(gamestate->computer_hits, gamestate->computer_hits_idx, target_field))
                    continue;
                made_my_choice = true;
                break;
            case COMPUTER_STATE_SEARCH_TOP:
                target_field -= 1;
                if(    target_field%FIELD_COUNT_Y == FIELD_COUNT_Y-1
                    || list_contains(gamestate->computer_missed_shots, gamestate->computer_missed_shots_idx, target_field)) {

                    target_field = gamestate->computer_hits[gamestate->computer_hits_idx-1];
                    gamestate->computer_state = COMPUTER_STATE_SEARCH_BOTTOM;
                    continue;
                }
                if(list_contains(gamestate->computer_hits, gamestate->computer_hits_idx, target_field))
                    continue;
                made_my_choice = true;
                break;
            case COMPUTER_STATE_SEARCH_BOTTOM:
                target_field += 1;
                if(    target_field%FIELD_COUNT_Y == 0
                    || list_contains(gamestate->computer_missed_shots, gamestate->computer_missed_shots_idx, target_field)) {

                    target_field = gamestate->computer_hits[gamestate->computer_hits_idx-1];
                    gamestate->computer_state = COMPUTER_STATE_SEARCH_RIGHT; // should never happen!
                    continue;
                }
                if(list_contains(gamestate->computer_hits, gamestate->computer_hits_idx, target_field))
                    continue;
                made_my_choice = true;
                break;
            default:
                printf("ERROR: Invalid computer turn state: %d\n", gamestate->computer_state);
                break;
        }
    }
    printf("computer fires on %d\n", target_field);
    target_ship = get_ship_from_field(gamestate, target_field, true);
    gamestate->state = GAMESTATE_PLAYER_TURN;
    if(target_ship == NULL) {
        printf("miss!\n");
        sprintf(gamestate->msg, "Computer missed.");
        gamestate->computer_missed_shots[gamestate->computer_missed_shots_idx++] = target_field;
        if(gamestate->computer_state != COMPUTER_STATE_RANDOM) {
            if(++gamestate->computer_state > COMPUTER_STATE_SEARCH_BOTTOM)
                gamestate->computer_state = COMPUTER_STATE_RANDOM;
        }
    } else {
        printf("hit!\n");
        sprintf(gamestate->msg, "Computer hit!");
        set_leds(led_orange);
        gamestate->computer_hits[gamestate->computer_hits_idx++] = target_field;
        if(gamestate->computer_state == COMPUTER_STATE_RANDOM)
            gamestate->computer_state = COMPUTER_STATE_SEARCH_RIGHT;
        if(all_fields_of_ship_destroyed(target_ship, gamestate->computer_hits, gamestate->computer_hits_idx)) {
            target_ship->sunken = true;
            gamestate->computer_state = COMPUTER_STATE_RANDOM;
            printf("SUNKEN (%d-ship)!\n", target_ship->size);
            sprintf(gamestate->msg, "Your %d-ship sunk!", target_ship->size);
            for(ship_idx=0; ship_idx<SHIP_COUNT; ship_idx++) {
                if(!gamestate->player_ships[ship_idx].sunken)
                    break;
            }
            if(ship_idx == SHIP_COUNT) {
                printf("All player ships have sunken! Player loses!\n");
                gamestate->state = GAMESTATE_LOSE;
            }
        }
    }

    gamestate->round++;
}

void handle_WIN(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_WIN\n");
    set_leds(NULL); // set leds randomly each iteration
    if(input_valid) {
        if(input == BUTTON_START) {
            printf("Button Start; -> to start screen!\n");
            new_game(gamestate);
        }
    }
}

void handle_LOSE(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_LOSE\n");
    if(input_valid) {
        if(input == BUTTON_START) {
            printf("Button Start; -> to start screen!\n");
            new_game(gamestate);
        }
    }
}

void handle_ASK_END_GAME(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_ASK_END_GAME\n");
    if(input_valid) {
        if(input == BUTTON_ACCEPT) {
            printf("Button A; -> end game!\n");
            new_game(gamestate);
        }
        if(input == BUTTON_BACK) {
            printf("Button B; -> resume game!\n");
            gamestate->state = gamestate->saved_state;
        }
    }
}

void handle_HIDDEN_FLAG(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_HIDDEN_FLAG\n");
    set_leds(led_rainbow);
    if(input_valid) {
        switch (input) {
            case BUTTON_ACCEPT:
                printf("Button A; You win!\n");
                gamestate->state = GAMESTATE_WIN;
                break;
            case BUTTON_BACK:
                printf("Button B; You lose!\n");
                gamestate->state = GAMESTATE_LOSE;
                break;
            case BUTTON_START:
                printf("Button Start; New Game!\n");
                new_game(gamestate);
                break;
            default:
                break;
        }
    }
}

void handle_INVALID_STATE(GameState* gamestate, bool input_valid, uint8_t input) {
    printf("handle_INVALID_STATE\n");
    if(input_valid) {
        switch (input) {
            case BUTTON_START:
                printf("Button Start; New Game!\n");
                new_game(gamestate);
                break;
            default:
                break;
        }
    }
}

// The main game function which starts the game state machine
void battleship_state_machine(xQueueHandle button_queue) {
    bool quit = false;
    pax_buf_t* pax_buffer = get_pax_buffer();
    keyboard_input_message_t msg;
    GameState gamestate;

    init_icons();
    new_game(&gamestate);


    printf("Start State Machine...\n");
    while (!quit) {
        render_screen(pax_buffer, &gamestate);
        clear_keyboard_queue();
        msg.state = false;
        xQueueReceive(button_queue, &msg, pdMS_TO_TICKS(SLEEP_DELAY_MS));
        printf("xQueueReceive()-->state=%d,input=%d,(gamestate=%d)\n", msg.state, msg.input, gamestate.state);
        set_leds(led_off);
        switch(gamestate.state) {
            case GAMESTATE_START_SCREEN:
                if(msg.state && (msg.input == BUTTON_BACK || msg.input == BUTTON_START)) {
                    // Leave App
                    printf("Bye!\n");
                    quit = true;
                    continue;
                }
                handle_START_SCREEN(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_PLACE_SHIPS:
                handle_PLACE_SHIPS(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_PLAYER_TURN:
                handle_PLAYER_TURN(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_COMPUTER_TURN:
                handle_COMPUTER_TURN(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_WIN:
                handle_WIN(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_LOSE:
                handle_LOSE(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_ASK_END_GAME:
                handle_ASK_END_GAME(&gamestate, msg.state, msg.input);
                break;
            case GAMESTATE_HIDDEN_FLAG:
                handle_HIDDEN_FLAG(&gamestate, msg.state, msg.input);
                break;
            default:
                handle_INVALID_STATE(&gamestate, msg.state, msg.input);
                break;
        }
    }
    printf("State Machine ended! Bye\n");
}
