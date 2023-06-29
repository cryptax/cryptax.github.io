---
layout: post
title: "Troopers 23 Badge Challenges"
author: cryptax
date: 2023-06-29
tags:
- Troopers
- Badge
- 2023
- Buffer overflow
- Boot
- Integer
- Serial
- picocom
---

Troopers conference is known for its fabulous hardware badges. This year, we were given an ESP32-base badge. In an workshop, we could solder a Shitty-Addon with colored LEDs and a connector for a remote control.

![Troopers 23 badge](/images/troopers23-badge.jpg)

In addition, the badge was featuring 2 challenges. Similar to CTF-challenges, when solved, the challenges would give you a *token*, you'd submit the token on a local troopers website, and get raffle tickets after a given number of points.

## Battleship

The badge featured a battleship game. You play the game against the computer, select where to hide ships, and try to find computer ships before computer finds yours.

![Battleship game splash page](/images/troopers23-battleship-welcome.jpg)
![Fire at guessed locations and try to sink your opponent's ships](/images/troopers23-battleship-play.jpg)

The [source code of the game](/source/battleship.c) is given to participants.

I solved this challenge with the help of *Kev*, thanks!

### Locating the flag

We quickly spot where the flag should be (but of course, it is not provided):

```c
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
```

We need to get in this case `GAMESTATE_HIDDEN_FLAG`. We also learn that this is not reverse engineering challenge: we need not dump the firmware, but an exploit that can be triggered from the game itself.

So, how do we get in this state? The switch is on the game status. The possible game states are defined as an enumeration at the beginning of the file.

```c
// Game States
#define GAMESTATE_START_SCREEN    0
#define GAMESTATE_PLACE_SHIPS     1
#define GAMESTATE_PLAYER_TURN     2
#define GAMESTATE_COMPUTER_TURN   3
#define GAMESTATE_WIN             4
#define GAMESTATE_LOSE            5
#define GAMESTATE_ASK_END_GAME    6
#define GAMESTATE_HIDDEN_FLAG     7
...
    switch(gamestate->state) {
...
```

There is no place where we can explicitly state the gamestate to `GAMESTATE_HIDDEN_FLAG`.  We have a look at the type of `gamestate`:

```c
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
```

The state is an integer. Before that we have a couple of arrays such as `computer_hits` or `player_hits`, with a fixed size of `MAX_SHIP_FIELDS`. 

```c
#define MAX_SHIP_FIELDS 14 // 5 + 4 + 3 + 2
```

### Finding the overflow

Is there a possible overflow?
We spot those lines which happen when we hit a ship of the computer:

```c
sprintf(gamestate->msg, "You hit a ship!");
set_leds(led_orange);
gamestate->player_hits[gamestate->player_hits_idx++] = gamestate->cursor;
```

Can we have the index `gamestate->player_hits_idx++` be bigger than the allocated size for the `player_hits` table? Yes! Actually, there is no control on the limit of this index. So, for instance, **if we keep hitting the same location on the map, `player_hits_idx` will increment** and we can get it to overflow the next values.

As we have an integer `round` between the `player_hits` table and our target `state` integer, we'll have to hit the same location 14+4=18 times to start overflow the state.

### Exploiting to get to the desired state

Recall we need to get to state *7*. So, we need to **overflow state and write value 7** in it. How can we control the values we overflow the memory with?

```c
gamestate->player_hits[gamestate->player_hits_idx++] = gamestate->cursor;
```

With the position of the cursor!

So, we need to find what corresponds to `cursor=7`. The code is nicely commented for that.

```c
/* Game Grid (field numbers):
 * X Y--->
 * | 00 06 12 18 24 30 36 42 48 54 60 66
 * | 01 07 13 19 25 31 37 43 49 55 61 67
 * v 02 08 14 20 26 32 38 44 50 56 62 68
 *   03 09 15 21 27 33 39 45 51 57 63 69
 *   04 10 16 22 28 34 40 46 52 58 64 70
 *   05 11 17 23 29 35 41 47 53 59 65 71
 */
```

### Exploit

The methodology is the following:

1. We start a game and randomly place our ships (we don't care where they are located).
2. We fire at the 2nd column of the 2nd row. If there is no ship there, we won't be able to get to the desired stage, and we restart the game.
3. If there is a computer ship at that location, we repeatedly fire at that location. After more than 14 hits at the same location, you should see the round overflow:

[Round overflow](/images/troopers23-battleship-round.jpg)

4. We continue to hit the same location, and at some point (after 18th hit), we should finally overflow the state and get in the desired FLAG state and get our flag!

[We got the token!](/images/troopers23-battleship-flag.jpg)

### Conclusion

I loved this challenge! The fact it can be solved directly from the badge is an added bonus. You just need to read the code with attention.

## U-Boot

Another challenge suggested "have you tried to restart the badge"? 
The name of the challenge suggested there would be something to see during the boot phase. 

I connected to the serial port of the badge with `picocom -b 112500 /dev/ttyUSB0`, and re-started the badge.

![Spot the serial number: it is formatted exactly as an expected badge token](/images/troopers23-uboot.png)

The hexadecimal value didn't work - as a matter of fact badge tokens were only integers. So, Kev and I simply converted the values to decimal ;-)

`8932-5861-1530-4735-6252` is the correct flag!

Easy! An introduction challenge to connecting to the serial port.

