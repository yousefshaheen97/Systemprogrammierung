/*
 * joystick.h
 *
 * Created: 08.07.2025 18:35:28
 *  Author: yousef
 */ 



#ifndef JOYSTICK_H_
#define JOYSTICK_H_
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	JS_LEFT,
	JS_RIGHT,
	JS_UP,
	JS_DOWN,
	JS_NEUTRAL
} Direction;

void js_init(void);
uint16_t js_getHorizontal(void);
uint16_t js_getVertical(void);
Direction js_getDirection(void);
bool js_getButton(void);
void os_waitForJoystickButtonInput(void);

void os_waitForNoJoystickButtonInput(void);

#endif 