/*
 * led_snake.c
 *
 * Created: 14.07.2024 06:19:24
 *  Author: yousef
 */ 
#include "led_snake.h"
#include "os_scheduler.h"
#include "joystick.h"
#include "led_draw.h"
#include "led_paneldriver.h"
#include "lcd.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

//! size of the LED matrix
#define FIELD_SIZE 32
/////////////////////
#define Zeile 25
#define Kolumne 31
//////////////////////



//! number of cells on the board
#define MAX_CELLS (Zeile * Kolumne)

//! bytes needed for the direction ring buffer (2 bit per entry)
#define RING_SIZE (MAX_CELLS / 4)

//! delay between game steps in ms
#define MOVE_DELAY 200

static const int8_t dir_dx[4] = {0, 1, 0, -1};
static const int8_t dir_dy[4] = {-1, 0, 1, 0};

//! helper macro for modulo on indices
#define MOD_INDEX(x) ((x) & (MAX_CELLS - 1))

//! ring buffer handling -------------------------------------------------------
static uint8_t ring[RING_SIZE];

//! write a direction (2 bit) into the ring buffer
static inline void ring_set(uint16_t index, uint8_t dir) {
	index = MOD_INDEX(index);
	const uint16_t bit = index * 2;
	const uint16_t byte = bit >> 3;           // divide by 8
	const uint8_t shift = 6 - (bit & 6);      // {0,2,4,6} -> {6,4,2,0}
	ring[byte] &= ~(3 << shift);
	ring[byte] |= (dir & 3) << shift;
}

//! read a direction (2 bit) from the ring buffer
static inline uint8_t ring_get(uint16_t index) {
	index = MOD_INDEX(index);
	const uint16_t bit = index * 2;
	const uint16_t byte = bit >> 3;
	const uint8_t shift = 6 - (bit & 6);
	return (ring[byte] >> shift) & 3;
}

//!---------------------------------------------------------------------------

typedef struct {
	uint16_t head;      //!< index of first body element in ring
	uint16_t tail;      //!< index after last body element in ring
	uint16_t length;    //!< number of body segments (without head)

	uint8_t head_x;
	uint8_t head_y;
	uint8_t tail_x;
	uint8_t tail_y;
	SnakeDir dir;       //!< current moving direction

	uint8_t food_x;
	uint8_t food_y;

	uint16_t score;
	uint16_t highscore;

	bool board[Kolumne][Zeile];

	bool game_over;
	bool paused;
} SnakeGame;

static SnakeGame game;

//! create a new food pixel at a random free position
static void spawn_food(void) {
	do {
		game.food_x = rand() % Kolumne;
		game.food_y = rand() % Zeile;
	} while (game.board[game.food_x][game.food_y]);
}

//! update the information on the LCD display
static void draw_score(void) {
	//////////////////////////////////////////////////////////////////////////
	draw_filledRectangle(1, 25, 31, 31, COLOR_BLUE);
	const uint8_t y = FIELD_SIZE - 6; // bottom area for text
	draw_letter('S', 2, y, COLOR_RED, false, false);
	draw_number(game.score, false, 10, y, COLOR_RED, false, false);

	draw_letter('H', 16, y, COLOR_RED, false, false);
	draw_number(game.highscore, true, 25, y, COLOR_RED, false, false);
}

//! render the whole board onto the LED panel
static void draw_board(void) {
	draw_clearDisplay();
	for (uint8_t x = 0; x < Kolumne; x++) {
		for (uint8_t y = 0; y < Zeile ; y++) {
			if (game.board[x][y]) {
				draw_setPixel(x, y, COLOR_GREEN);
			}
		}
	}
	draw_setPixel(game.food_x, game.food_y, COLOR_YELLOW);
}

//! reset the game state for a new round
static void reset_game(void) {
	memset(&game, 0, sizeof(game));
	memset(ring, 0, sizeof(ring));

	game.dir = SNAKE_RIGHT;

	// starting position roughly in the middle
	game.head_x = Kolumne / 2;
	game.head_y = Zeile / 2;

	// initial body length two pixels to the left
	game.length = 2;
	game.head = 0;
	game.tail = game.length;

	game.board[game.head_x][game.head_y] = true;
	game.board[game.head_x - 1][game.head_y] = true;
	game.board[game.head_x - 2][game.head_y] = true;

	game.tail_x = game.head_x - 2;
	game.tail_y = game.head_y;

	ring_set(0, SNAKE_RIGHT);
	ring_set(1, SNAKE_RIGHT);

	game.score = 0;
	spawn_food();
	
	draw_board();
	draw_score();
	game.game_over = false;
	game.paused = false;
}

//! perform one game step if not paused or over
static void step_game(void) {
	uint8_t new_x = game.head_x + dir_dx[game.dir];
	uint8_t new_y = game.head_y + dir_dy[game.dir];

	if (new_x >= Kolumne || new_y >= Zeile || game.board[new_x][new_y]) {
		game.game_over = true;
		return;
	}

	const bool grow = (new_x == game.food_x && new_y == game.food_y);

	// insert new head element
	game.head = MOD_INDEX(game.head - 1);
	ring_set(game.head, game.dir);

	game.head_x = new_x;
	game.head_y = new_y;
	game.board[new_x][new_y] = true;

	if (grow) {
		game.score++;
		if (game.score > game.highscore) {
			game.highscore = game.score;
		}
		game.length++;
		spawn_food();
		} else {
		// remove tail
		game.board[game.tail_x][game.tail_y] = false;
		SnakeDir tail_dir = ring_get(MOD_INDEX(game.tail - 1));
		game.tail_x += dir_dx[tail_dir];
		game.tail_y += dir_dy[tail_dir];
		game.tail = MOD_INDEX(game.tail - 1);
	}

	draw_board();
	draw_score();
}

//! convert joystick direction to snake direction without allowing a reversal
static void update_direction(Direction js_dir) {
	switch (js_dir) {
		case JS_LEFT:
		if (game.dir != SNAKE_RIGHT) game.dir = SNAKE_LEFT;
		break;
		case JS_RIGHT:
		if (game.dir != SNAKE_LEFT) game.dir = SNAKE_RIGHT;
		break;
		case JS_UP:
		if (game.dir != SNAKE_DOWN) game.dir = SNAKE_UP;
		break;
		case JS_DOWN:
		if (game.dir != SNAKE_UP) game.dir = SNAKE_DOWN;
		break;
		default:
		break;
	}
}

//!---------------------------------------------------------------------------
//! public entry point of the snake game
void snake_main(void) {
	os_setSchedulingStrategy(OS_SS_RUN_TO_COMPLETION);
	panel_init();
	panel_initTimer();
	js_init();

	srand(os_systemTime_precise());

	reset_game();

	while (1) {
		if (game.game_over) {
			draw_clearDisplay();
			draw_board();
			draw_score();
			// draw "GAME OVER" centered
			draw_letter('G', 5, 5, COLOR_RED, false, true);
			draw_letter('A', 11, 5, COLOR_RED, false, true);
			draw_letter('M', 17, 5, COLOR_RED, false, true);
			draw_letter('E', 23, 5, COLOR_RED, false, true);
			draw_letter('O', 5, 15, COLOR_RED, false, true);
			draw_letter('V', 11, 15, COLOR_RED, false, true);
			draw_letter('E', 17, 15, COLOR_RED, false, true);
			draw_letter('R', 23, 15, COLOR_RED, false, true);
			panel_startTimer();
			while (!js_getButton()) {
			}
			panel_stopTimer();
			os_waitForNoJoystickButtonInput();
			reset_game();
			continue;
		}

		if (game.paused) {
			draw_clearDisplay();
			draw_letter('P', 2, 10, COLOR_RED, false, true);
			draw_letter('A', 8, 10, COLOR_RED, false, true);
			draw_letter('U', 14, 10, COLOR_RED, false, true);
			draw_letter('S', 20, 10, COLOR_RED, false, true);
			draw_letter('E', 26, 10, COLOR_RED, false, true);
			panel_startTimer();
			while (!js_getButton()) {
			}
			panel_stopTimer();
			os_waitForNoJoystickButtonInput();
			game.paused = false;
			draw_board();
			draw_score();
			continue;
			while (!js_getButton()) {
			}
			os_waitForNoJoystickButtonInput();
			game.paused = false;
			draw_score();
			continue;
		}

		if (js_getButton()) {
			os_waitForNoJoystickButtonInput();
			game.paused = true;
			continue;
		}

		update_direction(js_getDirection());

		step_game();

		panel_startTimer();
		delayMs(MOVE_DELAY);
		panel_stopTimer();
	}
}
