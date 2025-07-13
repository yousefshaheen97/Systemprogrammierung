/*
 * led_snake.c
 *
 * Created: 14.07.2024 06:19:24
 *  Author: yousef
 */ 

#include "led_snake.h"
#include "led_draw.h"
#include "led_paneldriver.h"
#include "lcd.h"

#include <stdlib.h>
#include <util/delay.h>
#include <time.h>





uint8_t maxScore = 0;
uint8_t score = 0;
uint8_t food_Position_X, food_Posision_Y;
uint8_t isFoodEaten = 0;

uint8_t headX;
uint8_t headY;
uint8_t snake_Direction;
uint16_t lenght_Of_Snake;

RingBuffer snake_Body;




void play_Snake(){
	panel_init();
	panel_initTimer();
	panel_startTimer();
	js_init();
	initialize_State_Of_Game();
	while (1){
		adjust_State_Of_Game(); // aktualisiere status des spiel je nach spiel umstände
		if (js_getButton()){  // Prüfe ob buttom gedrükt ist
			stop_Game();
		}
		_delay_ms(150);  // spielgeschwindigkeit steuern
	}
}


void lose_Game(){
	draw_clearDisplay();
	
	if (score >= maxScore){
		
		draw_letter('G', 1, 2, COLOR_GREEN, false, false);
		draw_letter('o', 5, 2, COLOR_GREEN, false, false);
		draw_letter('o', 9, 2, COLOR_GREEN, false, false);
		draw_letter('d', 13, 2, COLOR_GREEN, false, false);
		
		draw_letter('g', 17, 2, COLOR_GREEN, false, false);
		draw_letter('a', 21, 2, COLOR_GREEN, false, false);
		draw_letter('m', 25, 2, COLOR_GREEN, false, false);
		draw_letter('e', 29, 2, COLOR_GREEN, false, false);
		
		
		draw_letter('u', 5, 9, COLOR_GREEN, false, false);
		draw_letter('s', 9, 9, COLOR_GREEN, false, false);
		draw_letter('c', 13, 9, COLOR_GREEN, false, false);
		draw_letter('r', 17, 9, COLOR_GREEN, false, false);
		
		draw_number(score, false, 22, 9, COLOR_BLUE, false, false);
		
		
		draw_letter('h', 5, 15, COLOR_GREEN, false, false);
		draw_letter('s', 9, 15, COLOR_GREEN, false, false);
		draw_letter('c', 13, 15, COLOR_GREEN, false, false);
		draw_letter('r', 17, 15, COLOR_GREEN, false, false);
		
		draw_number(maxScore, false, 22, 15, COLOR_BLUE, false, false);
		
		
		draw_letter('C', 1, 21, COLOR_WHITE, false, false);
		draw_letter('L', 5, 21, COLOR_WHITE, false, false);
		draw_letter('i', 9, 21, COLOR_WHITE, false, false);
		draw_letter('c', 13, 21, COLOR_WHITE, false, false);
		draw_letter('k', 17, 21, COLOR_WHITE, false, false);
		
		draw_letter('T', 23, 21, COLOR_WHITE, false, false);
		draw_letter('o', 27, 21, COLOR_WHITE, false, false);
		
		draw_letter('r', 3, 27, COLOR_WHITE, false, false);
		draw_letter('e', 7, 27, COLOR_WHITE, false, false);
		draw_letter('s', 11, 27, COLOR_WHITE, false, false);
		draw_letter('t', 15, 27, COLOR_WHITE, false, false);
		draw_letter('a', 19, 27, COLOR_WHITE, false, false);
		draw_letter('r', 23, 27, COLOR_WHITE, false, false);
		draw_letter('t', 27, 27, COLOR_WHITE, false, false);
		
		} else {
		
		draw_letter('b', 2, 2, COLOR_RED, false, false);
		draw_letter('a', 6, 2, COLOR_RED, false, false);
		draw_letter('d', 10, 2, COLOR_RED, false, false);
		
		draw_letter('g', 16, 2, COLOR_RED, false, false);
		draw_letter('a', 20, 2, COLOR_RED, false, false);
		draw_letter('m', 24, 2, COLOR_RED, false, false);
		draw_letter('e', 28, 2, COLOR_RED, false, false);
		
		
		draw_letter('u', 5, 9, COLOR_GREEN, false, false);
		draw_letter('s', 9, 9, COLOR_GREEN, false, false);
		draw_letter('c', 13, 9, COLOR_GREEN, false, false);
		draw_letter('r', 17, 9, COLOR_GREEN, false, false);
		
		draw_number(score, false, 22, 9, COLOR_BLUE, false, false);
		
		
		draw_letter('h', 5, 15, COLOR_GREEN, false, false);
		draw_letter('s', 9, 15, COLOR_GREEN, false, false);
		draw_letter('c', 13, 15, COLOR_GREEN, false, false);
		draw_letter('r', 17, 15, COLOR_GREEN, false, false);
		
		draw_number(maxScore, false, 22, 15, COLOR_BLUE, false, false);
		
		
		draw_letter('C', 1, 21, COLOR_WHITE, false, false);
		draw_letter('L', 5, 21, COLOR_WHITE, false, false);
		draw_letter('i', 9, 21, COLOR_WHITE, false, false);
		draw_letter('c', 13, 21, COLOR_WHITE, false, false);
		draw_letter('k', 17, 21, COLOR_WHITE, false, false);
		
		draw_letter('T', 23, 21, COLOR_WHITE, false, false);
		draw_letter('o', 27, 21, COLOR_WHITE, false, false);
		
		draw_letter('r', 3, 27, COLOR_WHITE, false, false);
		draw_letter('e', 7, 27, COLOR_WHITE, false, false);
		draw_letter('s', 11, 27, COLOR_WHITE, false, false);
		draw_letter('t', 15, 27, COLOR_WHITE, false, false);
		draw_letter('a', 19, 27, COLOR_WHITE, false, false);
		draw_letter('r', 23, 27, COLOR_WHITE, false, false);
		draw_letter('t', 27, 27, COLOR_WHITE, false, false);
		
	}
	
	while (!js_getButton()){
		
	}
	
	os_waitForNoJoystickButtonInput();

	initialize_State_Of_Game();
}




void stop_Game(){
	os_waitForNoJoystickButtonInput();
	draw_clearDisplay();
	
	
	if (score >= maxScore){
		
		draw_letter('P', 2, 1, COLOR_RED, false, true);
		draw_letter('A', 8, 1, COLOR_RED, false, true);
		draw_letter('U', 14, 1, COLOR_RED, false, true);
		draw_letter('S', 20, 1, COLOR_RED, false, true);
		draw_letter('E', 26, 1, COLOR_RED, false, true);
		
		
		draw_letter('u', 5, 9, COLOR_GREEN, false, false);
		draw_letter('s', 9, 9, COLOR_GREEN, false, false);
		draw_letter('c', 13, 9, COLOR_GREEN, false, false);
		draw_letter('r', 17, 9, COLOR_GREEN, false, false);
		
		draw_number(score, false, 22, 9, COLOR_BLUE, false, false);
		
		
		draw_letter('h', 5, 15, COLOR_GREEN, false, false);
		draw_letter('s', 9, 15, COLOR_GREEN, false, false);
		draw_letter('c', 13, 15, COLOR_GREEN, false, false);
		draw_letter('r', 17, 15, COLOR_GREEN, false, false);
		
		draw_number(maxScore, false, 22, 15, COLOR_BLUE, false, false);
		
		
		draw_letter('C', 1, 21, COLOR_WHITE, false, false);
		draw_letter('L', 5, 21, COLOR_WHITE, false, false);
		draw_letter('i', 9, 21, COLOR_WHITE, false, false);
		draw_letter('c', 13, 21, COLOR_WHITE, false, false);
		draw_letter('k', 17, 21, COLOR_WHITE, false, false);
		
		draw_letter('T', 23, 21, COLOR_WHITE, false, false);
		draw_letter('o', 27, 21, COLOR_WHITE, false, false);
		
		draw_letter('C', 1, 27, COLOR_WHITE, false, false);
		draw_letter('o', 5, 27, COLOR_WHITE, false, false);
		draw_letter('n', 9, 27, COLOR_WHITE, false, false);
		draw_letter('t', 13, 27, COLOR_WHITE, false, false);
		draw_letter('i', 17, 27, COLOR_WHITE, false, false);
		draw_letter('n', 21, 27, COLOR_WHITE, false, false);
		draw_letter('u', 25, 27, COLOR_WHITE, false, false);
		draw_letter('e', 29, 27, COLOR_WHITE, false, false);
		
		} else {
		
		draw_letter('P', 2, 1, COLOR_RED, false, true);
		draw_letter('A', 8, 1, COLOR_RED, false, true);
		draw_letter('U', 14, 1, COLOR_RED, false, true);
		draw_letter('S', 20, 1, COLOR_RED, false, true);
		draw_letter('E', 26, 1, COLOR_RED, false, true);
		
		draw_letter('u', 5, 9, COLOR_YELLOW, false, false);
		draw_letter('s', 9, 9, COLOR_YELLOW, false, false);
		draw_letter('c', 13, 9, COLOR_YELLOW, false, false);
		draw_letter('r', 17, 9, COLOR_YELLOW, false, false);
		
		draw_number(score, false, 22, 9, COLOR_BLUE, false, false);
		
		
		draw_letter('h', 5, 15, COLOR_GREEN, false, false);
		draw_letter('s', 9, 15, COLOR_GREEN, false, false);
		draw_letter('c', 13, 15, COLOR_GREEN, false, false);
		draw_letter('r', 17, 15, COLOR_GREEN, false, false);
		
		draw_number(maxScore, false, 22, 15, COLOR_BLUE, false, false);
		
		
		draw_letter('C', 1, 21, COLOR_WHITE, false, false);
		draw_letter('L', 5, 21, COLOR_WHITE, false, false);
		draw_letter('i', 9, 21, COLOR_WHITE, false, false);
		draw_letter('c', 13, 21, COLOR_WHITE, false, false);
		draw_letter('k', 17, 21, COLOR_WHITE, false, false);
		
		draw_letter('T', 23, 21, COLOR_WHITE, false, false);
		draw_letter('o', 27, 21, COLOR_WHITE, false, false);
		
		draw_letter('C', 1, 27, COLOR_WHITE, false, false);
		draw_letter('o', 5, 27, COLOR_WHITE, false, false);
		draw_letter('n', 9, 27, COLOR_WHITE, false, false);
		draw_letter('t', 13, 27, COLOR_WHITE, false, false);
		draw_letter('i', 17, 27, COLOR_WHITE, false, false);
		draw_letter('n', 21, 27, COLOR_WHITE, false, false);
		draw_letter('u', 25, 27, COLOR_WHITE, false, false);
		draw_letter('e', 29, 27, COLOR_WHITE, false, false);
	}
	
	while (1){
		if (js_getButton()){
			if (js_getDirection() == JS_NEUTRAL){
				os_waitForNoJoystickButtonInput();
				draw_clearDisplay();
				redraw_Game_Field();
				break;
			}
			else{
				
				initialize_State_Of_Game();
				break;
			}
		}
	}
}


void redraw_Game_Field(){
	// Zeichne Spiel Feld
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_UP_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_DOWN_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_LEFT_COLUMN, i, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_RIGHT_COLUMN, i, COLOR_WHITE);
	}
	
	draw_letter('s', 2, 0, COLOR_WHITE, false, false);
	
	draw_number(score, true, 11, 0, COLOR_BLUE, false, false);
	if (score > maxScore){
		maxScore = score;
	}
	draw_letter('H', 16, 0, COLOR_WHITE, false, false);
	draw_letter('S', 19, 0, COLOR_WHITE, false, false);
	
	draw_number(maxScore, true, 28, 0, COLOR_GREEN, false, false);
	
	
	draw_setPixel(food_Position_X, food_Posision_Y, COLOR_YELLOW);
	
	
	draw_setPixel(headX, headY, COLOR_RED);
	
	// Hilfskoordinaten
	uint8_t x = headX;
	uint8_t y = headY;
	// Starte am letzten Ringbuffer-Index
	uint16_t idx = snake_Body.head;

	// Verfolge den Ringbuffer bis zum Schwanz
	while (idx != snake_Body.tail) {
		// Zwei Bit-Positionen zurückspringen
		if (idx >= 2) {
			idx = idx - 2;
			} else {
			idx = RINGBUFFER_SIZE - (2 - idx);
		}

		// Richtung aus dem Puffer lesen
		uint8_t dir = load_Snake_Buffer_Bit_Pair(idx);

		// Koordinaten anpassen
		if (dir == RIGHT) {
			x = x - 1;
			} else if (dir == LEFT) {
			x = x + 1;
			} else if (dir == UP) {
			y = y + 1;
			} else if (dir == DOWN) {
			y = y - 1;
		}

		// Segment zeichnen
		draw_setPixel(x, y, COLOR_GREEN);
	}
}




void setup_Snake(){
	uint8_t head_Min_X   = FIELD_LEFT_COLUMN + 1;
	uint8_t head_Max_X   = FIELD_RIGHT_COLUMN - 1;
	uint8_t widthX = head_Max_X - head_Min_X + 1;
	uint8_t head_Min_Y    = FIELD_UP_ROW + 1;
	uint8_t head_Max_Y    = FIELD_DOWN_ROW - 1;
	uint8_t heightY = head_Max_Y - head_Min_Y + 1;
	headX = (rand() % widthX) + head_Min_X;
	headY = (rand() % heightY) + head_Min_Y;
	snake_Direction = rand() % 4;
	lenght_Of_Snake = 1;
	snake_Body.head = 0;
	snake_Body.tail = 0;
}


void initialize_State_Of_Game(){
	draw_clearDisplay();
	
	// Zeichne Spiel Feld
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_UP_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_DOWN_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_LEFT_COLUMN, i, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_RIGHT_COLUMN, i, COLOR_WHITE);
	}
	
	score = 0;
	// zeichne score und high score
	draw_letter('s', 2, 0, COLOR_WHITE, false, false);
	draw_number(score, true, 11, 0, COLOR_BLUE, false, false);
	if (score > maxScore){
		maxScore = score;
	}
	draw_letter('H', 16, 0, COLOR_WHITE, false, false);
	draw_letter('S', 19, 0, COLOR_WHITE, false, false);
	draw_number(maxScore, true, 28, 0, COLOR_GREEN, false, false);
	
	setup_Snake();
	
	draw_setPixel(headX, headY, COLOR_RED);
	
	produce_Food();
}


void adjust_State_Of_Game(){
	Direction get_Snake_Direction = js_getDirection();
	adjust_Snake_Direction(get_Snake_Direction);

	adjust_Snake_Buffer();
	
	
	walk_Snake();

	
	if (collision_Detection()){
		
		if (score > maxScore){
			maxScore = score; 
		}
		lose_Game(); 
	}

	
	if (did_Snake_Eat(food_Position_X, food_Posision_Y)){
		score++;
		produce_Food(); 
	}
	
	
	draw_filledRectangle(0, 0, 31, 4, COLOR_YELLOW);
	
	// Zeichne Spiel Feld
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_UP_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 0; i < 32; i++){
		draw_setPixel(i, FIELD_DOWN_ROW, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_LEFT_COLUMN, i, COLOR_WHITE);
	}
	for (uint8_t i = 5; i < 32; i++){
		draw_setPixel(FIELD_RIGHT_COLUMN, i, COLOR_WHITE);
	}
	
	draw_letter('s', 2, 0, COLOR_WHITE, false, false);
	
	draw_number(score, true, 11, 0, COLOR_BLUE, false, false);
	if (score > maxScore){
		maxScore = score;
	}
	draw_letter('H', 16, 0, COLOR_WHITE, false, false);
	draw_letter('S', 19, 0, COLOR_WHITE, false, false);
	
	draw_number(maxScore, true, 28, 0, COLOR_GREEN, false, false);
}


void walk_Snake(){
	  // lösche alten kopf
	  draw_setPixel(headX, headY, COLOR_BLACK);

	  if (snake_Direction == RIGHT) {
		  headX = headX + 1;
		  } else if (snake_Direction == LEFT) {
		  headX = headX - 1;
		  } else if (snake_Direction == UP) {
		  headY = headY - 1;
		  } else if (snake_Direction == DOWN) {
		  headY = headY + 1;
	  }

	  display_Snake();
}


void adjust_Snake_Direction(Direction newDirection){
	// erhalte neu head direction von joystick
	if (newDirection == JS_RIGHT) {
		snake_Direction = RIGHT;
		} else if (newDirection == JS_LEFT) {
		snake_Direction = LEFT;
		} else if (newDirection == JS_UP) {
		snake_Direction = UP;
		} else if (newDirection == JS_DOWN) {
		snake_Direction = DOWN;
	} 
}



void adjust_Snake_Buffer(){
	// Nur wenn schlange länge grösser als 1 hat
	if (lenght_Of_Snake > 1) {
		// kopfzeiger um 2 bewegen
		snake_Body.head += 2;
		if (snake_Body.head >= RINGBUFFER_SIZE) {
			snake_Body.head -= RINGBUFFER_SIZE;
		}

		// Byte-Index und Bit-Index berechnen
		uint16_t byteIndex = snake_Body.head / 8;  
		uint8_t  bitIndex  = snake_Body.head % 8;

		// aktualisiere SnakeBodyBuffer
		if (bitIndex == 0) {
			
			snake_Body.directions[byteIndex] &= 0xFC;        
			snake_Body.directions[byteIndex] |= snake_Direction;
		}
		else if (bitIndex == 2) {
			
			snake_Body.directions[byteIndex] &= 0xF3;      
			snake_Body.directions[byteIndex] |= (snake_Direction << 2);
		}
		else if (bitIndex == 4) {
			
			snake_Body.directions[byteIndex] &= 0xCF;        
			snake_Body.directions[byteIndex] |= (snake_Direction << 4);
		}
		else if (bitIndex == 6) {
			
			snake_Body.directions[byteIndex] &= 0x3F;        
			snake_Body.directions[byteIndex] |= (snake_Direction << 6);
		}

		// schwanz verschieben nur wenn kein essen gegessen wird
		if (!isFoodEaten) {
			snake_Body.tail += 2;
			if (snake_Body.tail >= RINGBUFFER_SIZE) {
				snake_Body.tail -= RINGBUFFER_SIZE;
			}
		}

		// isFoodEaten zurücksetzen
		isFoodEaten = 0;
	}
}


uint8_t load_Snake_Buffer_Bit_Pair(uint16_t bitPairPosition){
	// Bestimme in welchem Byte das Bit paar liegt
	uint16_t byteIndex = bitPairPosition / 8;
	// Bestimme an welcher Position im Byte das Bit-Paar beginnt (0, 2, 4 oder 6)
	uint8_t bitIndex = bitPairPosition % 8;
	// Lese das Byte aus dem Puffer
	uint8_t byteValue = snake_Body.directions[byteIndex];
	uint8_t pair = 0;

	// Je nach Bit Position die passenden 2 Bits maskieren und nach rechts schieben
	if (bitIndex == 0) {
		
		pair = byteValue & 0x03;        
	}
	else if (bitIndex == 2) {
		
		pair = (byteValue & 0x0C) >> 2; 
	}
	else if (bitIndex == 4) {
		
		pair = (byteValue & 0x30) >> 4;
	}
	else if (bitIndex == 6) {
		
		pair = (byteValue & 0xC0) >> 6; 
	}

	// Ergebnis zurückgeben
	return pair;
}


void display_Snake(){
	// Kopf zeichnen
	draw_setPixel(headX, headY, COLOR_RED);

	// wenn länge =1 dann fertig
	if (lenght_Of_Snake == 1) {
		return;
	}

	
	uint8_t x = headX;
	uint8_t y = headY;
	uint16_t pos = snake_Body.head;

	
	// Erstes Segment direkt hinter dem Kopf zeichnen
	// Richtung auslesen
	uint8_t dir = load_Snake_Buffer_Bit_Pair(pos);
	// Position anpassen
	if (dir == RIGHT) {
		x = x - 1;
		} else if (dir == LEFT) {
		x = x + 1;
		} else if (dir == UP) {
		y = y + 1;
		} else if (dir == DOWN) {
		y = y - 1;
		}
	// Körpersegment zeichnen
	draw_setPixel(x, y, COLOR_GREEN);
	

	// restlische segmente verfolgen
	while (pos != snake_Body.tail) {
		// Zum vorherigen Segment springen (2 Bit zurück)
		if (pos >= 2) {
			pos = pos - 2;
			} else {
			pos = RINGBUFFER_SIZE - (2 - pos);
		}

		// Richtung auslesen
		uint8_t dir = load_Snake_Buffer_Bit_Pair(pos);
		// Position anpassen
		if (dir == RIGHT) {
			x = x - 1;
			} else if (dir == LEFT) {
			x = x + 1;
			} else if (dir == UP) {
			y = y + 1;
			} else if (dir == DOWN) {
			y = y - 1;
		}
	
	}

	// alte Schwanz löschen
	draw_setPixel(x, y, COLOR_BLACK);
}


bool collision_Detection(){
	// Prüfe Kollision mit dem spielfeld
	if (headX >= FIELD_RIGHT_COLUMN) {
		return true;
	}
	if (headX <= FIELD_LEFT_COLUMN) {
		return true;
	}
	if (headY >= FIELD_DOWN_ROW) {
		return true;
	}
	
	if (headY <= FIELD_UP_ROW) {
		return true;
	}

	// kollision mit sich selbst prüfen
	// Wir verfolgen die Schlange rückwärts vom Kopf zum Schwanz
	uint8_t x = headX;
	uint8_t y = headY;
	uint16_t position = snake_Body.head;

	while (position != snake_Body.tail) {
		// Zum vorherigen Bit-Paar springen (2 Bit zurück)
		if (position >= 2) {
			position = position - 2;
			} else {
			position = RINGBUFFER_SIZE - (2 - position);
		}

		// Richtung aus dem Puffer auslesen
		uint8_t dir = load_Snake_Buffer_Bit_Pair(position);

		// Koordinate entsprechend anpassen
		if (dir == RIGHT) {
			x = x - 1;
			} else if (dir == LEFT) {
			x = x + 1;
			} else if (dir == UP) {
			y = y + 1;
			} else if (dir == DOWN) {
			y = y - 1;
		}

		// Prüfe ob kollision gibt
		if (x == headX && y == headY) {
			return true;
		}
	}

	// Keine Kollision gefunden
	return false;
}



void produce_Food(){
	// Spielfeld kordinate berechnen
	uint8_t minX   = FIELD_LEFT_COLUMN + 1;
	uint8_t maxX   = FIELD_RIGHT_COLUMN - 1;
	uint8_t widthX = maxX - minX + 1;
	uint8_t minY    = FIELD_UP_ROW + 1;
	uint8_t maxY    = FIELD_DOWN_ROW - 1;
	uint8_t heightY = maxY - minY + 1;

	// food in random spot in fieled zeigen
	food_Position_X = (rand() % widthX) + minX;
	food_Posision_Y = (rand() % heightY) + minY;

	// essen zeichnen
	draw_setPixel(food_Position_X, food_Posision_Y, COLOR_YELLOW);
}



bool did_Snake_Eat(uint8_t foodX, uint8_t foodY){
	bool eaten = false;

	// Kopf Position mit Essens Position vergleichen
	if (headX == foodX && headY == foodY) {
		// essen aus den display löschen
		draw_setPixel(foodX, foodY, COLOR_GREEN);

		// Schlange um 1 verlängern
		if (lenght_Of_Snake < MAXIMUM_LENGTH) {
			lenght_Of_Snake = lenght_Of_Snake + 1;
		}

		// Flag setzen, damit der Ringbuffer weiß, dass wir gefressen haben
		isFoodEaten = 1;

		eaten = true;
	}

	// Rückgabe, ob gegessen wurde
	return eaten;
}


