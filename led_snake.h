/*
 * led_snake.h
 *
 * Created: 14.07.2024 06:17:24
 *  Author: yousef
 */ 
#ifndef LED_SNAKE_H_
#define LED_SNAKE_H_
#include <stdint.h>
#include <stdbool.h>

//! Directions used by the snake game
typedef enum {
	SNAKE_UP = 0,
	SNAKE_RIGHT = 1,
	SNAKE_DOWN = 2,
	SNAKE_LEFT = 3
} SnakeDir;

//! Entry point of the snake application
void snake_main(void);


#endif /* LED_SNAKE_H_ */
