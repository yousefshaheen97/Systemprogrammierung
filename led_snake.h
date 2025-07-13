/*
 * led_snake.h
 *
 * Created: 14.07.2024 06:17:24
 *  Author: yousef
 */ 


#ifndef LED_SNAKE_H_
#define LED_SNAKE_H_

#include <stdbool.h>
#include "joystick.h"







void play_Snake(void);


void stop_Game(void);

void lose_Game(void);


void initialize_State_Of_Game(void);

void setup_Snake(void);

void walk_Snake(void);


void adjust_Snake_Direction(Direction newDirection);

void adjust_Snake_Buffer(void);


void adjust_State_Of_Game(void);



uint8_t load_Snake_Buffer_Bit_Pair(uint16_t bitPairIndex);

bool collision_Detection(void);


bool did_Snake_Eat(uint8_t foodX, uint8_t foodY);

void display_Snake(void);

void produce_Food(void);

void redraw_Game_Field(void);




#define MAXIMUM_LENGTH 1024
#define RINGBUFFER_SIZE (2 * MAXIMUM_LENGTH)


#define COLUMNS_NUMBER 32
#define ROWS_NUMBER 32

#define FIELD_UP_ROW 5
#define FIELD_DOWN_ROW 31
#define FIELD_LEFT_COLUMN 0
#define FIELD_RIGHT_COLUMN 31




#define RIGHT 0
#define LEFT 1
#define UP 2
#define DOWN 3

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint8_t directions[256];
} RingBuffer;








#endif /* LED_SNAKE_H_ */
