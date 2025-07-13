#include "lcd.h"
#include "led_draw.h"
#include "led_paneldriver.h"
#include "led_patterns.h"
#include "os_core.h"
#include "os_process.h"
#include "os_scheduler.h"
#include "util.h"
#include "led_snake.h"
#include "joystick.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <util/delay.h>


#include <math.h>

#include <util/atomic.h>
#define SNAKE_TICK_MS 200


#if VERSUCH != 6
#warning "Please fix the VERSUCH-define"
#endif

REGISTER_AUTOSTART(snake)
void snake(void){
	

	play_Snake();
	
	
	
}