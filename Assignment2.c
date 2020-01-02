#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include "usb_serial.h"
#include <graphics.h>
#include <macros.h>
#include "lcd_model.h"
#include "cab202_adc.h"
#define SQRT(x, y) sqrt(x *x + y * y)
#define STATUS_BAR_HEIGHT 8
#define CHEESE_IMG 'C'
#define DOOR_IMG 'X'
#define PI 3.14159265358979323846264338327950288
#define FREQ (8000000.0)
#define PRESCALE (256.0)
//define adc value
int right_adc, left_adc;
//define wall cordinates
int wall_cordinate[4][4], wall_cordinate2[4][4], wall_initial_cordinate[4][4], num_wall;
volatile int wall_dx, wall_dy;
//define cheese cordinates
int cheese_cordinate[5][7][2];
int count_c, count_t, cheese_eaten, count_w;

//define trap cordinates
int trap_cordinate[5][5][2];
//define postion cordinates
int potion_x, potion_y;
int potion_cordinate[7][2];
//define door cordinates
int door_x, door_y;
int error;
int door_cordinate[8][2];
//game varible
bool enter_game, pause, game_paused, next_cheese, next_trap,
	clicked, clicked1, clicked2, clicked3, found_cheese, found_trap,
	game_start, found_cheese_number[5], found_trap_number[5],
	firework_hit[20], new_level, superSaya, found_potion, found_door,
	set_potion, drawpotion, draw, game_over, advance_level, set_door;
int level, score;

//define Jerry values
double jerry_x, jerry_y, distance;
int jerry_block[7][2], lives;

//define Tom values
double tom_x, tom_y, tom_dx, tom_dy, speed;
int tomX, tomY, jerryX, jerryY;
int tom_block[8][2];

//define Firework coordinate
double firework_x[20], firework_y[20], firework_dx[20], firework_dy[20], new_fx[20], new_fy[20];
int num_firework;

//TIME
volatile uint32_t overflow_counter = 0;
volatile uint32_t overflow_counter1 = 0;
volatile uint8_t overflow_counter4 = 0;
volatile uint32_t overflow_counter3_cheese[5];
volatile uint32_t overflow_counter3_potion = 0;
volatile uint32_t overflow_counter3_level_2 = 0;
volatile uint32_t overflow_counter3_trap[5];

int time;
double stop_time, current_time;

volatile uint8_t switch_state[7];
volatile uint8_t bit_counter[7];

uint8_t channel1;
uint8_t dir = 0xFF;

bool Cheese_Collision(int i);
bool Trap_Collision(int i);
bool Postion_Collision();
void setup_jerry();
void setup_potion();
bool Wall_Collision(int character_block[7][2], int wall_cordinate[4][4], int next_x, int next_y);
bool Jerry_Hit_Wall();

//debouncing
ISR(TIMER0_OVF_vect)
{
	if (enter_game)
	{
		overflow_counter++;
	}

	for (int i = 0; i < 7; i++)
	{
		bit_counter[i] = (bit_counter[i] << 1);
		bit_counter[i] &= 0b01111111;
	}

	bit_counter[0] |= BIT_VALUE(PIND, 1); // SWITCH UP

	bit_counter[1] |= BIT_VALUE(PINB, 7); // SWITCH DOWN

	bit_counter[2] |= BIT_VALUE(PINB, 1); // SWITCH LEFT

	bit_counter[3] |= BIT_VALUE(PIND, 0); // SWITCH RIGHT

	bit_counter[4] |= BIT_VALUE(PINB, 0); //SWITCH CENTER

	bit_counter[5] |= BIT_VALUE(PINF, 5); //BUTTON B

	bit_counter[6] |= BIT_VALUE(PINF, 6); //BUTTON A

	for (int i = 0; i < 7; i++)
	{
		if (bit_counter[i] == 0b01111111)
		{
			switch_state[i] = 1;
		}
		if (bit_counter[i] == 0)
		{
			switch_state[i] = 0;
		}
	}
}
//update wall movement
ISR(TIMER1_OVF_vect)
{
	if (enter_game)
	{
		overflow_counter1++;
	}
	if (overflow_counter1 > 2 && !pause)
	{
		if (right_adc > 341 * 2)
		{
			wall_dx = (wall_dx + 5) % (2);
			wall_dy = (wall_dy + 5) % (2);
		}

		if (right_adc >= 341 && right_adc <= 341 * 2)
		{
			wall_dx = (wall_dx + 5) % (1);
			wall_dy = (wall_dy + 5) % (1);
		}

		if (right_adc < 341)
		{
			wall_dx = -(wall_dx + 5) % (2);
			wall_dy = -(wall_dy + 5) % (2);
		}

		for (int i = 0; i < 4; i++)
		{
			if (right_adc < 341 || right_adc > 341 * 2)
			{
				if (wall_cordinate[i][0] < wall_cordinate[i][2] && wall_cordinate[i][1] > wall_cordinate[i][3])
				{
					wall_cordinate[i][0] += wall_dx;
					wall_cordinate[i][1] += wall_dy;
					wall_cordinate[i][2] += wall_dx;
					wall_cordinate[i][3] += wall_dy;
					if (Wall_Collision(jerry_block, wall_cordinate, -1, -1))
					{
						jerry_x++;
						jerry_y++;
					}
					if (Jerry_Hit_Wall() && Wall_Collision(jerry_block, wall_cordinate, -1, -1))
					{
						setup_jerry();
					}
				}
				if (wall_cordinate[i][0] == wall_cordinate[i][2])
				{
					wall_cordinate[i][0] -= wall_dx;
					wall_cordinate[i][2] -= wall_dx;
					if (Wall_Collision(jerry_block, wall_cordinate, 0, 0))
					{
						jerry_x--;
					}
					if (Jerry_Hit_Wall() && Wall_Collision(jerry_block, wall_cordinate, 0, 0))
					{
						setup_jerry();
					}
				}
				if (wall_cordinate[i][1] == wall_cordinate[i][3])
				{
					wall_cordinate[i][1] += wall_dy;
					wall_cordinate[i][3] += wall_dy;
					if (Wall_Collision(jerry_block, wall_cordinate, 0, 1))
					{
						jerry_y--;
					}
					if (Jerry_Hit_Wall() && Wall_Collision(jerry_block, wall_cordinate, 0, 1))
					{
						setup_jerry();
					}
				}
				if ((wall_cordinate[i][0] < wall_cordinate[i][2] && wall_cordinate[i][1] < wall_cordinate[i][3]))
				{
					wall_cordinate[i][0] -= wall_dx;
					wall_cordinate[i][1] += wall_dy;
					wall_cordinate[i][2] -= wall_dx;
					wall_cordinate[i][3] += wall_dy;
					if (Wall_Collision(jerry_block, wall_cordinate, 0, 0))
					{
						jerry_x--;
						jerry_y++;
					}
					if (Jerry_Hit_Wall() && Wall_Collision(jerry_block, wall_cordinate, 0, 0))
					{
						setup_jerry();
					}
				}
			}
			overflow_counter1 = 0;
		}
	}
}

//delay for cheese and mouse trap and potion
ISR(TIMER3_OVF_vect)
{
	for (int i = 0; i < 5; i++)
	{
		if (found_cheese_number[i] == true)
		{
			overflow_counter3_cheese[i]++;
		}

		if (found_trap_number[i] == true)
		{
			overflow_counter3_trap[i]++;
		}

		if (overflow_counter3_cheese[i] > 3)
		{
			cheese_cordinate[i][0][0] = 5 + rand() % (LCD_X - 5 - 3);
			cheese_cordinate[i][0][1] = 10 + rand() % (LCD_Y - 10 - 3);
			bool collide = Cheese_Collision(i);
			while (collide)
			{
				cheese_cordinate[i][0][0] = 5 + rand() % (LCD_X - 5 - 3);
				cheese_cordinate[i][0][1] = 10 + rand() % (LCD_Y - 10 - 3);
				collide = Cheese_Collision(i);
			}
			int cx = cheese_cordinate[i][0][0];
			int cy = cheese_cordinate[i][0][1];
			cheese_cordinate[i][1][0] = cx + 1;
			cheese_cordinate[i][1][1] = cy;
			cheese_cordinate[i][2][0] = cx + 2;
			cheese_cordinate[i][2][1] = cy;
			cheese_cordinate[i][3][0] = cx;
			cheese_cordinate[i][3][1] = cy + 1;
			cheese_cordinate[i][4][0] = cx;
			cheese_cordinate[i][4][1] = cy + 2;
			cheese_cordinate[i][5][0] = cx + 1;
			cheese_cordinate[i][5][1] = cy + 2;
			cheese_cordinate[i][6][0] = cx + 2;
			cheese_cordinate[i][6][1] = cy + 2;
			overflow_counter3_cheese[i] = 0;
			found_cheese_number[i] = false;
		}

		if (overflow_counter3_trap[i] > 4)
		{
			trap_cordinate[i][0][0] = round(tom_x);
			trap_cordinate[i][0][1] = round(tom_y);
			bool collide = Trap_Collision(i);
			if (collide)
			{
				trap_cordinate[i][0][0] = round(tom_x) + 1;
				trap_cordinate[i][0][1] = round(tom_y) + 1;
			}
			int tx = trap_cordinate[i][0][0];
			int ty = trap_cordinate[i][0][1];
			trap_cordinate[i][1][0] = tx + 2;
			trap_cordinate[i][1][1] = ty;
			trap_cordinate[i][2][0] = tx + 1;
			trap_cordinate[i][2][1] = ty + 1;
			trap_cordinate[i][3][0] = tx;
			trap_cordinate[i][3][1] = ty + 2;
			trap_cordinate[i][4][0] = tx + 2;
			trap_cordinate[i][4][1] = ty + 2;
			overflow_counter3_trap[i] = 0;
			found_trap_number[i] = false;
		}
	}

	if (drawpotion)
	{
		overflow_counter3_level_2++;
	}

	if (overflow_counter3_level_2 > 10)
	{
		setup_potion();
		draw = true;
		overflow_counter3_level_2 = 0;
		drawpotion = false;
	}

	if (found_potion == true)
	{
		overflow_counter3_potion++;
	}

	if (overflow_counter3_potion > 17)
	{
		potion_x = round(tom_x);
		potion_y = round(tom_y);
		// bool collide = Postion_Collision();
		// while (collide)
		// {
		// 	potion_x = 5 + rand() % (LCD_X - 5 - 3);
		// 	potion_y = 10 + rand() % (LCD_Y - 10 - 3);
		// 	collide = Postion_Collision();
		// }
		int px = potion_x;
		int py = potion_y;
		potion_cordinate[0][0] = px;
		potion_cordinate[0][1] = py;
		potion_cordinate[1][0] = px - 1;
		potion_cordinate[1][1] = py + 1;
		potion_cordinate[2][0] = px - 1;
		potion_cordinate[2][1] = py + 2;
		potion_cordinate[3][0] = px;
		potion_cordinate[3][1] = py + 1;
		potion_cordinate[4][0] = px;
		potion_cordinate[4][1] = py + 2;
		potion_cordinate[5][0] = px + 1;
		potion_cordinate[5][1] = py + 1;
		potion_cordinate[6][0] = px + 1;
		potion_cordinate[6][1] = py + 2;
		overflow_counter3_potion = 0;
		found_potion = false;
		superSaya = false;
	}
}

//FLASH LIGHT IN SUPER MODE
ISR(TIMER4_OVF_vect)
{
	if (overflow_counter4 < channel1)
	{
		PORTB |= (1 << PINB2);
		PORTB |= (1 << PINB3);
	}
	else
	{
		PORTB &= ~(1 << PINB2);
		PORTB &= ~(1 << PINB3);
	}
	overflow_counter4++;
}

void pwm_light()
{
	if (dir & 0B00000010)
		channel1 += 10;
	else
		channel1 -= 10;

	if (channel1 > 254)
		dir &= ~0B00000010;
	else if (channel1 < 1)
		dir |= 0B00000010;
}

double get_elapsed_time()
{
	double time_count = (overflow_counter * 256.0 + TCNT0) * PRESCALE / FREQ;
	return time_count;
}

char time_buffer[20];

void draw_time(uint8_t x, uint8_t y, int value, colour_t colour)
{
	snprintf(time_buffer, sizeof(time_buffer), "%02d", value);
	draw_string(x, y, time_buffer, colour);
}

void draw_int(uint8_t x, uint8_t y, int value, colour_t colour)
{
	snprintf(time_buffer, sizeof(time_buffer), "%d", value);
	draw_string(x, y, time_buffer, colour);
}

void draw_double(uint8_t x, uint8_t y, double value, colour_t colour)
{
	snprintf(time_buffer, sizeof(time_buffer), "%lf", value);
	draw_string(x, y, time_buffer, colour);
}

void usb_serial_read_string(char *message)
{
	int c = 0;
	int buffer_count = 0;

	while (c != '\n')
	{
		c = usb_serial_getchar();
		message[buffer_count] = c;
		buffer_count++;
	}
}
void timer()
{
	if (!pause) //IF GAME NOT PAUSED
	{
		//GET THE GET_CURREN_TIME TO MINUS THE GAME START TIME
		//AND IF PAUSED RESET THE TIME_SECOND TO 0 AND PLUS THE SECOND WHEN STOPPED.
		time = get_elapsed_time() - current_time + stop_time;
	}

	if (pause)
	{
		if (game_paused)
		{
			//REMEBER THE EPOC IN SECOND
			stop_time = time;
		}
		//UPDATE THE CURRENT TIME
		current_time = get_elapsed_time();
	}
}

void draw_character(int x, int y, int width, int height, uint8_t image[10])
{
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			if (image[row] & (0b10000000 >> col))
			{
				draw_pixel(x + col, y + row, FG_COLOUR);
			}
		}
	}
}

void draw_start_up_screen()
{
	clear_screen();
	draw_string(10, 10, "n10560564", FG_COLOUR);
	draw_string(10, 20, "DANG VU ANH BUI", FG_COLOUR);
	draw_string(10, 30, "TOM & JERRY", FG_COLOUR);
	if (switch_state[5] == 1 && clicked)
	{
		enter_game = true;
		clicked = false;
		srand(TCNT0);
	}
	if (switch_state[5] == 0)
	{
		clicked = true;
	}
	show_screen();
}

void usb_serial_send(char *message)
{
	// Cast to avoid "error: pointer targets in passing argument 1
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *)message, strlen(message));
}

void setup_switches()
{
	CLEAR_BIT(DDRB, 0); //Center
	CLEAR_BIT(DDRB, 1); //Left
	CLEAR_BIT(DDRB, 7); //Down
	CLEAR_BIT(DDRD, 0); //Right
	CLEAR_BIT(DDRD, 1); //Up

	CLEAR_BIT(DDRF, 5); //SW2
	CLEAR_BIT(DDRF, 6); //SW1
}

void draw_status()
{
	draw_string(0, 0, "L: ", FG_COLOUR);
	draw_int(10, 0, level, FG_COLOUR);

	///Lives///
	uint8_t live[4] = {
		0b01100110,
		0b10011001,
		0b01000010,
		0b00011000};
	draw_character(18, 2, 8, 4, live);
	draw_string(27, 0, ":", FG_COLOUR);
	draw_int(32, 0, lives, FG_COLOUR);

	///Score///
	draw_string(38, 0, "S:", FG_COLOUR);
	draw_int(47, 0, score, FG_COLOUR);

	draw_time(59, 0, time / 60, FG_COLOUR);
	draw_char(70, 0, ':', FG_COLOUR);
	draw_time(74, 0, time % 60, FG_COLOUR);
	draw_line(0, 8, 83, 8, FG_COLOUR);
}

void wall_initial()
{
	if (level == 1)
	{
		int array_wall[4][4] = {{18, 15, 13, 25}, {25, 35, 25, 45}, {45, 10, 60, 10}, {58, 25, 72, 30}};

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				wall_cordinate[i][j] = array_wall[i][j];
				wall_initial_cordinate[i][j] = wall_cordinate[i][j];
			}
		}
	}
}

void draw_wall(void)
{
	for (int wall = 0; wall < 4; wall++)
	{
		draw_line(wall_cordinate[wall][0], wall_cordinate[wall][1], wall_cordinate[wall][2], wall_cordinate[wall][3], FG_COLOUR);
	}
}

double DistanceOf2Points(int x1, int y1, int x2, int y2)
{
	return SQRT((x1 - x2), (y1 - y2));
}

bool OnCollide(int x, int y, int x1, int y1, int x2, int y2)
{
	double dis1 = DistanceOf2Points(x, y, x1, y1);
	double dis2 = DistanceOf2Points(x, y, x2, y2);
	double dis3 = DistanceOf2Points(x1, y1, x2, y2);

	return abs(dis3 - (dis1 + dis2)) <= 10e-5;
}

bool Jerry_Hit_Wall()
{
	if (jerry_x < 0 || jerry_x + 2 >= LCD_X || jerry_y <= STATUS_BAR_HEIGHT || jerry_y + 3 >= LCD_Y)
	{
		return true;
	}
	return false;
}

// PIXEL PERFECT COLLISION

bool Check_Collision_Pixel(int x, int y)
{
	uint8_t bank_number;
	uint8_t pixel_location;

	bank_number = y >> 3;
	pixel_location = y & 7;
	if (screen_buffer[bank_number * LCD_X + x] & (1 << pixel_location))
	{
		return true;
	}

	return false;
}

bool collide_character(int char_block_a[7][2], int char_block_b[6][2])
{
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (char_block_a[i][0] == char_block_b[j][0] && char_block_a[i][1] == char_block_b[j][1])
			{
				return true;
			}
		}
	}
	return false;
}

int Check_Collision(int character_block[7][2], int wall_array[4][4], int next_step_x, int next_step_y)
{
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (OnCollide(character_block[i][0] + next_step_x, character_block[i][1] + next_step_y, wall_array[j][0], wall_array[j][1], wall_cordinate[j][2], wall_cordinate[j][3]))
			{
				return 1;
			}
		}
	}
	return 0;
}

bool Firework_Collision_Check(int x, int y, int wall_array[4][4])
{
	for (int i = 0; i < 4; i++)
	{
		if (OnCollide(x, y, wall_cordinate[i][0], wall_cordinate[i][1], wall_cordinate[i][2], wall_cordinate[i][3]))
		{
			return true;
		}
	}
	return false;
}

bool Firework_Collision_Tom(int x, int y, int jerry_array[7][2])
{
	for (int i = 0; i < 7; i++)
	{
		if (jerry_array[i][0] == x && jerry_array[i][1] == y)
		{
			return true;
		}
	}
	return false;
}

int Check_Collision_Cheese(int cheese_cordinate[5][7][2], int character_cordinate[7][2])
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			for (int k = 0; k < 7; k++)
			{
				if (cheese_cordinate[i][j][0] == character_cordinate[k][0] && cheese_cordinate[i][j][1] == character_cordinate[k][1])
				{
					return i;
				}
			}
		}
	}
	return -1;
}

int Check_Collision_Trap(int trap_cordinate[5][5][2], int character_cordinate[7][2])
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 7; k++)
			{
				if (trap_cordinate[i][j][0] == character_cordinate[k][0] && trap_cordinate[i][j][1] == character_cordinate[k][1])
				{
					return i;
				}
			}
		}
	}
	return -1;
}

int Check_Collision_Door(int door_cordinate[8][2], int character_cordinate[7][2])
{

	for (int i = 0; i < 8; i++)
	{
		for (int k = 0; k < 7; k++)
		{
			if (door_cordinate[i][0] == character_cordinate[k][0] && door_cordinate[i][1] == character_cordinate[k][1])
			{
				return 1;
			}
		}
	}
	return -1;
}

int Check_Collision_Potion(int potion_cordinate[7][2], int character_cordinate[7][2])
{
	for (int i = 0; i < 7; i++)
	{
		for (int k = 0; k < 7; k++)
		{
			if (potion_cordinate[i][0] == character_cordinate[k][0] && potion_cordinate[i][1] == character_cordinate[k][1])
			{
				return 1;
			}
		}
	}
	return -1;
}

bool Jerry_Collision_Comparision(int character_block[7][2], int x, int y, int next_x, int next_y)
{
	for (int i = 0; i < 7; i++)
	{
		if ((character_block[i][0] + next_x) == x && (character_block[i][1] + next_y) == y)
		{
			return true;
		}
	}
	return false;
}

bool Tom_Collision_Comparision(int character_block[6][2], int x, int y, double next_x, double next_y)
{
	for (int i = 0; i < 6; i++)
	{
		if ((character_block[i][0] + round(next_x)) == x && (character_block[i][1] + round(next_y)) == y)
		{
			return true;
		}
	}
	return false;
}

bool Wall_Collision(int character_block[7][2], int wall_cordinate[4][4], int next_x, int next_y)
{
	if (superSaya)
	{
		return false;
	}
	for (int wall = 0; wall < 4; wall++)
	{
		if (wall_cordinate[wall][0] == wall_cordinate[wall][2])
		{
			// Draw vertical line
			for (int i = wall_cordinate[wall][1]; (wall_cordinate[wall][3] > wall_cordinate[wall][1]) ? i <= wall_cordinate[wall][3] : i >= wall_cordinate[wall][3]; (wall_cordinate[wall][3] > wall_cordinate[wall][1]) ? i++ : i--)
			{
				if (Jerry_Collision_Comparision(character_block, wall_cordinate[wall][0], i, next_x, next_y))
				{
					return true;
				}
			}
		}
		else if (wall_cordinate[wall][1] == wall_cordinate[wall][3])
		{
			// Draw horizontal line
			for (int i = wall_cordinate[wall][0]; (wall_cordinate[wall][2] > wall_cordinate[wall][0]) ? i <= wall_cordinate[wall][2] : i >= wall_cordinate[wall][2]; (wall_cordinate[wall][2] > wall_cordinate[wall][0]) ? i++ : i--)
			{
				if (Jerry_Collision_Comparision(character_block, i, wall_cordinate[wall][1], next_x, next_y))
				{
					return true;
				}
			}
		}
		else
		{
			//Check from left pixel to right pixel of the wall, regardless of the order the endpoints are.
			if (wall_cordinate[wall][0] > wall_cordinate[wall][2])
			{
				int t = wall_cordinate[wall][0];
				wall_cordinate[wall][0] = wall_cordinate[wall][2];
				wall_cordinate[wall][2] = t;
				t = wall_cordinate[wall][1];
				wall_cordinate[wall][1] = wall_cordinate[wall][3];
				wall_cordinate[wall][3] = t;
			}

			// Get Bresenhaming method
			float dx = wall_cordinate[wall][2] - wall_cordinate[wall][0];
			float dy = wall_cordinate[wall][3] - wall_cordinate[wall][1];
			float err = 0.0;
			float derr = ABS(dy / dx);

			for (int x = wall_cordinate[wall][0], y = wall_cordinate[wall][1]; (dx > 0) ? x <= wall_cordinate[wall][2] : x >= wall_cordinate[wall][2]; (dx > 0) ? x++ : x--)
			{
				if (Jerry_Collision_Comparision(character_block, x, y, next_x, next_y))
				{
					return true;
				}
				err += derr;
				while (err >= 0.5 && ((dy > 0) ? y <= wall_cordinate[wall][3] : y >= wall_cordinate[wall][3]))
				{
					if (Jerry_Collision_Comparision(character_block, x, y, next_x, next_y))
					{
						return true;
					}
					y += (dy > 0) - (dy < 0);
					err -= 1.0;
				}
			}
		}
	}

	return false;
}

bool Wall_Collision2(int character_block[6][2], int wall_cordinate[4][4], double next_x, double next_y)
{
	for (int wall = 0; wall < 4; wall++)
	{
		if (wall_cordinate[wall][0] == wall_cordinate[wall][2])
		{
			// Check vertical wall
			for (int i = wall_cordinate[wall][1]; (wall_cordinate[wall][3] > wall_cordinate[wall][1]) ? i <= wall_cordinate[wall][3] : i >= wall_cordinate[wall][3]; (wall_cordinate[wall][3] > wall_cordinate[wall][1]) ? i++ : i--)
			{
				if (Tom_Collision_Comparision(character_block, wall_cordinate[wall][0], i, next_x, next_y))
				{
					return true;
				}
			}
		}
		else if (wall_cordinate[wall][1] == wall_cordinate[wall][3])
		{
			// Check horizontal wall
			for (int i = wall_cordinate[wall][0]; (wall_cordinate[wall][2] > wall_cordinate[wall][0]) ? i <= wall_cordinate[wall][2] : i >= wall_cordinate[wall][2]; (wall_cordinate[wall][2] > wall_cordinate[wall][0]) ? i++ : i--)
			{
				if (Tom_Collision_Comparision(character_block, i, wall_cordinate[wall][1], next_x, next_y))
				{
					return true;
				}
			}
		}
		else
		{
			//	Always draw from left to right, regardless of the order the endpoints are
			//	presented.
			if (wall_cordinate[wall][0] > wall_cordinate[wall][2])
			{
				int t = wall_cordinate[wall][0];
				wall_cordinate[wall][0] = wall_cordinate[wall][2];
				wall_cordinate[wall][2] = t;
				t = wall_cordinate[wall][1];
				wall_cordinate[wall][1] = wall_cordinate[wall][3];
				wall_cordinate[wall][3] = t;
			}

			// Get Bresenhaming...
			float dx = wall_cordinate[wall][2] - wall_cordinate[wall][0];
			float dy = wall_cordinate[wall][3] - wall_cordinate[wall][1];
			float err = 0.0;
			float derr = ABS(dy / dx);

			for (int x = wall_cordinate[wall][0], y = wall_cordinate[wall][1]; (dx > 0) ? x <= wall_cordinate[wall][2] : x >= wall_cordinate[wall][2]; (dx > 0) ? x++ : x--)
			{
				if (Tom_Collision_Comparision(character_block, x, y, next_x, next_y))
				{
					return true;
				}
				err += derr;
				while (err >= 0.5 && ((dy > 0) ? y <= wall_cordinate[wall][3] : y >= wall_cordinate[wall][3]))
				{
					if (Tom_Collision_Comparision(character_block, x, y, next_x, next_y))
					{
						return true;
					}
					y += (dy > 0) - (dy < 0);
					err -= 1.0;
				}
			}
		}
	}

	return false;
}

//=============================

void setup_jerry()
{
	if (level == 1)
	{
		jerry_x = 0;
		jerry_y = STATUS_BAR_HEIGHT + 1;
	}

	if (level == 2)
	{
		jerry_x = (double)jerryX;
		jerry_y = (double)jerryY;
	}
}

void draw_jerry()
{
	if (!superSaya)
	{
		uint8_t jerry[4] = {
			0b11100000,
			0b01000000,
			0b01000000,
			0b11000000};
		draw_character(jerry_x, jerry_y, 3, 4, jerry);
	}

	if (superSaya)
	{
		uint8_t jerryS[4] = {
			0b11110000,
			0b01100000,
			0b01100000,
			0b11100000};
		draw_character(jerry_x, jerry_y, 4, 4, jerryS);
	}
}

void setup_firework(int firework_number);
void up_level();

void update_jerry(int16_t char_code)
{
	int preState = 0;
	int jerry_array[7][2] = {
		{jerry_x, jerry_y}, {jerry_x + 1, jerry_y}, {jerry_x + 2, jerry_y}, {jerry_x + 1, jerry_y + 1}, {jerry_x + 1, jerry_y + 2}, {jerry_x + 1, jerry_y + 3}, {jerry_x, jerry_y + 3}};

	for (int i = 0; i < 7; i++)
	{
		jerry_block[i][0] = jerry_array[i][0];
		jerry_block[i][1] = jerry_array[i][1];
	}

	if ((switch_state[0] != preState || char_code == 'w') && (jerry_y > (STATUS_BAR_HEIGHT + 2)) && (!Wall_Collision(jerry_block, wall_cordinate, 0, -1))) ///DOWN
	{
		jerry_y -= distance;
	}

	if ((switch_state[1] != preState || char_code == 's') && ((jerry_y + 5) < LCD_Y) && (!Wall_Collision(jerry_block, wall_cordinate, 0, 1))) ///UP
	{
		jerry_y += distance;
	}

	if ((switch_state[2] != preState || char_code == 'a') && jerry_x > 0 && (!Wall_Collision(jerry_block, wall_cordinate, -1, 0))) ///Left
	{
		jerry_x -= distance;
	}

	if ((switch_state[3] != preState || char_code == 'd') && ((jerry_x + 3) < LCD_X) && (!Wall_Collision(jerry_block, wall_cordinate, 1, 0))) ///Right
	{
		jerry_x += distance;
	}

	if ((bit_counter[4] != preState || char_code == 'f') && num_firework < 20 && cheese_eaten >= 3)
	{
		setup_firework(num_firework);
		num_firework++;
		firework_hit[num_firework] = false;
	}

	if (char_code == 'p')
	{
		pause = !(pause);
		game_paused = !(game_paused);
	}

	if (char_code == 'l')
	{
		up_level();
	}
}

void intial_tom_movement()
{
	double step = speed;
	double tom_dir = rand() * PI * 2 / RAND_MAX;
	tom_dx = step * cos(tom_dir);
	tom_dy = step * sin(tom_dir);
	if (tom_dx < 0 && tom_dx > -0.5)
	{
		tom_dx -= 0.5;
	}
	if (tom_dx > 0 && tom_dx < 0.5)
	{
		tom_dx += 0.5;
	}
	if (tom_dy < 0 && tom_dy > -0.5)
	{
		tom_dy -= 0.5;
	}
	if (tom_dy > 0 && tom_dy < 0.5)
	{
		tom_dy += 0.5;
	}
}

void setup_tom()
{
	if (level == 1)
	{
		tom_x = LCD_X - 5;
		tom_y = LCD_Y - 9;
	}

	if (level == 2)
	{
		tom_x = (double)tomX;
		tom_y = (double)tomY;
	}
	intial_tom_movement();
}

void draw_tom(void)
{
	uint8_t tom[4] = {
		0b11100000,
		0b01000000,
		0b01000000,
		0b01000000};
	draw_character(tom_x, tom_y, 3, 4, tom);
}

void initial_tom()
{
	int tom_array[6][2] = {
		{tom_x, tom_y}, {tom_x + 1, tom_y}, {tom_x + 2, tom_y}, {tom_x + 1, tom_y + 1}, {tom_x + 1, tom_y + 2}, {tom_x + 1, tom_y + 3}};

	for (int i = 0; i < 6; i++)
	{
		tom_block[i][0] = tom_array[i][0];
		tom_block[i][1] = tom_array[i][1];
	}
}

bool move_tom()
{
	int new_x = round(tom_x + tom_dx);
	int new_y = round(tom_y + tom_dy);
	bool bounced = false;
	//BOUNCE TO BORDER AND ARBITRARY WALLS
	if (new_y <= (STATUS_BAR_HEIGHT + 1) || new_y + 3 >= LCD_Y || (Wall_Collision2(tom_block, wall_cordinate, 0, tom_dy)))
	{
		bounced = true;
		tom_dy = -tom_dy;
		return true;
	}

	if (new_x + 3 >= LCD_X || new_x <= 0 || (Wall_Collision2(tom_block, wall_cordinate, tom_dx, 0)))
	{
		bounced = true;
		tom_dx = -tom_dx;
		return true;
	}

	if (!bounced)
	{
		tom_x += tom_dx * left_adc / 1023.0;
		tom_y += tom_dy * left_adc / 1023.0;
	}
	return false;
}

void change_speed(bool collide)
{
	if (collide)
	{
		speed = (double)rand() / RAND_MAX * 0.1 + 0.5;
		intial_tom_movement();
	}
}

void update_tom()
{
	if (!pause)
	{
		initial_tom();
		bool isCollide = move_tom();
		change_speed(isCollide);
	}

	//COLLIDE WITH JERRY
	if (!superSaya)
	{
		if (collide_character(tom_block, jerry_block))
		{
			setup_tom();
			initial_tom();
			setup_jerry();
			lives--;
		}
	}
	if (superSaya)
	{
		if (collide_character(tom_block, jerry_block))
		{
			setup_tom();
			initial_tom();
			score++;
		}
	}
}

bool Cheese_Collision(int i)
{
	int cx = cheese_cordinate[i][0][0];
	int cy = cheese_cordinate[i][0][1];
	if (
		Check_Collision_Pixel(cx, cy) || Check_Collision_Pixel(cx + 1, cy) ||
		Check_Collision_Pixel(cx + 2, cy) || Check_Collision_Pixel(cx, cy + 1) ||
		Check_Collision_Pixel(cx, cy + 2) || Check_Collision_Pixel(cx + 1, cy + 2) ||
		Check_Collision_Pixel(cx + 2, cy + 2))
	{
		return true;
	}
	return false;
}

bool Trap_Collision(int i)
{
	int tx = trap_cordinate[i][0][0];
	int ty = trap_cordinate[i][0][1];
	if (
		Check_Collision_Pixel(tx, ty) || Check_Collision_Pixel(tx + 2, ty) ||
		Check_Collision_Pixel(tx + 1, ty + 1) || Check_Collision_Pixel(tx, ty + 2) ||
		Check_Collision_Pixel(tx + 2, ty + 2))
	{
		return true;
	}
	return false;
}

bool Door_Collision()
{
	int dx = door_x;
	int dy = door_y;
	if (Check_Collision_Pixel(dx, dy) || Check_Collision_Pixel(dx + 1, dy) ||
		Check_Collision_Pixel(dx + 2, dy) || Check_Collision_Pixel(dx, dy + 1) ||
		Check_Collision_Pixel(dx, dy + 2) || Check_Collision_Pixel(dx + 1, dy + 2) ||
		Check_Collision_Pixel(dx + 2, dy + 1) || Check_Collision_Pixel(dx + 2, dy + 2))
	{
		return true;
	}
	return false;
}

bool Postion_Collision()
{
	int px = potion_x;
	int py = potion_y;
	if (Check_Collision_Pixel(px, py) || Check_Collision_Pixel(px - 1, py + 1) ||
		Check_Collision_Pixel(px - 1, py + 2) || Check_Collision_Pixel(px, py + 1) ||
		Check_Collision_Pixel(px, py + 2) || Check_Collision_Pixel(px + 1, py + 1) ||
		Check_Collision_Pixel(px + 1, py + 2))
	{
		return true;
	}

	return false;
}

void setup_cheese()
{
	count_c = 0;
	cheese_eaten = 0;
	for (int i = 0; i < 5; i++)
	{
		cheese_cordinate[i][0][0] = 5 + rand() % (LCD_X - 5 - 3);
		cheese_cordinate[i][0][1] = 10 + rand() % (LCD_Y - 10 - 3);
		int cx = cheese_cordinate[i][0][0];
		int cy = cheese_cordinate[i][0][1];
		cheese_cordinate[i][1][0] = cx + 1;
		cheese_cordinate[i][1][1] = cy;
		cheese_cordinate[i][2][0] = cx + 2;
		cheese_cordinate[i][2][1] = cy;
		cheese_cordinate[i][3][0] = cx;
		cheese_cordinate[i][3][1] = cy + 1;
		cheese_cordinate[i][4][0] = cx;
		cheese_cordinate[i][4][1] = cy + 2;
		cheese_cordinate[i][5][0] = cx + 1;
		cheese_cordinate[i][5][1] = cy + 2;
		cheese_cordinate[i][6][0] = cx + 2;
		cheese_cordinate[i][6][1] = cy + 2;
		bool collide = Cheese_Collision(i);
		if (collide)
		{
			i = i - 1;
		}
	}
}

void draw_cheese()
{
	uint8_t cheese[3] = {
		0b11100000,
		0b10000000,
		0b11100000,
	};

	if (time >= 2 && time % 2 == 0 && next_cheese && count_c < 5)
	{
		count_c++;
		next_cheese = false;
	}

	else if (time % 2 != 0)
	{
		next_cheese = true;
	}

	for (int i = 0; i < count_c; i++)
	{
		draw_character(cheese_cordinate[i][0][0], cheese_cordinate[i][0][1], 3, 3, cheese);
	}
}

void update_cheese()
{
	int cheese_order = 0;
	cheese_order = Check_Collision_Cheese(cheese_cordinate, jerry_block) + 1;
	if (cheese_order != 0 && found_cheese)
	{
		cheese_cordinate[cheese_order - 1][0][0] = -5;
		cheese_cordinate[cheese_order - 1][0][1] = -5;
		found_cheese_number[cheese_order - 1] = true;
		found_cheese = false;
		cheese_eaten++;
		score++;
	}

	if (cheese_order == 0)
	{
		found_cheese = true;
	}
}

void setup_trap(int trap_number)
{
	trap_cordinate[trap_number][0][0] = round(tom_x);
	trap_cordinate[trap_number][0][1] = round(tom_y);
	bool collide = Trap_Collision(trap_number);
	if (collide)
	{
		trap_cordinate[trap_number][0][0] = round(tom_x) + 1;
		trap_cordinate[trap_number][0][1] = round(tom_y) + 1;
	}
	int tx = trap_cordinate[trap_number][0][0];
	int ty = trap_cordinate[trap_number][0][1];
	trap_cordinate[trap_number][1][0] = tx + 2;
	trap_cordinate[trap_number][1][1] = ty;
	trap_cordinate[trap_number][2][0] = tx + 1;
	trap_cordinate[trap_number][2][1] = ty + 1;
	trap_cordinate[trap_number][3][0] = tx;
	trap_cordinate[trap_number][3][1] = ty + 2;
	trap_cordinate[trap_number][4][0] = tx + 2;
	trap_cordinate[trap_number][4][1] = ty + 2;
}

void draw_trap()
{
	uint8_t trap[3] = {
		0b10100000,
		0b01000000,
		0b10100000};

	if (time >= 3 && time % 3 == 0 && next_trap && count_t < 5)
	{
		setup_trap(count_t);
		count_t++;
		next_trap = false;
	}

	else if (time % 3 != 0)
	{
		next_trap = true;
	}

	for (int i = 0; i < count_t; i++)
	{
		draw_character(trap_cordinate[i][0][0], trap_cordinate[i][0][1], 3, 3, trap);
	}
}

void update_trap()
{
	int trap_order = 0;
	trap_order = Check_Collision_Trap(trap_cordinate, jerry_block) + 1;
	if (trap_order != 0 && found_trap)
	{
		trap_cordinate[trap_order - 1][0][0] = -5;
		trap_cordinate[trap_order - 1][0][1] = -5;
		found_trap_number[trap_order - 1] = true;
		found_trap = false;
		if (!superSaya)
		{
			setup_jerry();
			lives--;
		}
	}

	if (trap_order == 0)
	{
		found_trap = true;
	}
}

void setup_door()
{
	if (cheese_eaten == 5)
	{
		door_x = 5 + rand() % (LCD_X - 5 - 5);
		door_y = 10 + rand() % (LCD_Y - 10 - 5);
		bool collide = Door_Collision();
		while (collide)
		{
			door_x = 5 + rand() % (LCD_X - 5 - 5);
			door_y = 10 + rand() % (LCD_Y - 10 - 5);
			collide = Door_Collision();
		}
		int dx = door_x;
		int dy = door_y;
		door_cordinate[0][0] = dx;
		door_cordinate[0][1] = dy;
		door_cordinate[1][0] = dx + 1;
		door_cordinate[1][1] = dy;
		door_cordinate[2][0] = dx + 2;
		door_cordinate[2][1] = dy;
		door_cordinate[3][0] = dx;
		door_cordinate[3][1] = dy + 1;
		door_cordinate[4][0] = dx;
		door_cordinate[4][1] = dy + 2;
		door_cordinate[5][0] = dx + 1;
		door_cordinate[5][1] = dy + 2;
		door_cordinate[6][0] = dx + 2;
		door_cordinate[6][1] = dy + 1;
		door_cordinate[7][0] = dx + 2;
		door_cordinate[7][1] = dy + 2;
	}
}

void draw_door()
{
	if (cheese_eaten >= 5 && level == 1)
	{
		uint8_t door[3] = {
			0b11100000,
			0b10100000,
			0b11100000};
		draw_character(door_x, door_y, 3, 3, door);
	}
}

void update_door()
{
	if (level == 1)
	{
		int collide = Check_Collision_Door(door_cordinate, jerry_block);
		if (collide == 1 && found_door)
		{
			up_level();
			found_door = false;
		}
		if (collide == -1)
		{
			found_door = true;
		}
	}
}

void setup_firework_values()
{
	num_firework = 0;
}

void setup_firework(int firework_number)
{
	if (cheese_eaten >= 3)
	{
		firework_x[firework_number] = round(jerry_x + 1);
		firework_y[firework_number] = round(jerry_y + 1);
		double step = 0.8;
		double t1 = tom_x - firework_x[firework_number];
		double t2 = tom_y - firework_y[firework_number];
		double d = sqrt(t1 * t1 + t2 * t2);

		firework_dx[firework_number] = step * (t1 * 0.5) / d;
		firework_dy[firework_number] = step * (t2 * 0.5) / d;
	}
}

void draw_firework()
{
	for (int i = 0; i < num_firework; i++)
	{
		draw_pixel(round(firework_x[i]), round(firework_y[i]), FG_COLOUR);
	}
}

void update_firework()
{
	for (int i = 0; i < num_firework; i++)
	{
		if (!firework_hit[i])
		{
			double step = 3;
			double t1 = tom_x - firework_x[i];
			double t2 = tom_y - firework_y[i];
			double d = sqrt(t1 * t1 + t2 * t2);

			firework_dx[i] = step * (t1 * 0.5) / d;
			firework_dy[i] = step * (t2 * 0.5) / d;

			new_fx[i] = round(firework_x[i] + firework_dx[i]);
			new_fy[i] = round(firework_y[i] + firework_dy[i]);
			bool collide_firework = false;
			//IF FIREWORK HITS WALL
			if (Firework_Collision_Check(round(new_fx[i]), round(new_fy[i]), wall_cordinate) || new_fx[i] == 0 || new_fx[i] == LCD_X - 1 || new_fy[i] == STATUS_BAR_HEIGHT + 1 || new_fy[i] == LCD_Y - 1)
			{
				firework_x[i] = -10;
				firework_y[i] = -10;
				collide_firework = true;
				firework_hit[i] = true;
			}
			//IF FIREWORK HITS CHARACTER
			if (Firework_Collision_Tom(round(new_fx[i]), round(new_fy[i]), tom_block))
			{
				// lives_tom--;
				// score_jerry++;
				firework_x[i] = -10;
				firework_y[i] = -10;
				collide_firework = true;
				firework_hit[i] = true;
				setup_tom();
				score++;
				error++;
			}
			// 		//MOVE ON IF NO COLLISION DETECTED
			if (!(collide_firework))
			{
				firework_x[i] += firework_dx[i];
				firework_y[i] += firework_dy[i];
			}
		}
	}
}

void setup_potion()
{
	potion_x = round(tom_x);
	potion_y = round(tom_y);
	// bool collide = Postion_Collision();
	// while (collide)
	// {
	// 	potion_x = 5 + rand() % (LCD_X - 5 - 3);
	// 	potion_y = 10 + rand() % (LCD_Y - 10 - 3);
	// 	collide = Postion_Collision();
	// }
	int px = potion_x;
	int py = potion_y;
	potion_cordinate[0][0] = px;
	potion_cordinate[0][1] = py;
	potion_cordinate[1][0] = px - 1;
	potion_cordinate[1][1] = py + 1;
	potion_cordinate[2][0] = px - 1;
	potion_cordinate[2][1] = py + 2;
	potion_cordinate[3][0] = px;
	potion_cordinate[3][1] = py + 1;
	potion_cordinate[4][0] = px;
	potion_cordinate[4][1] = py + 2;
	potion_cordinate[5][0] = px + 1;
	potion_cordinate[5][1] = py + 1;
	potion_cordinate[6][0] = px + 1;
	potion_cordinate[6][1] = py + 2;
}

void draw_potion()
{
	if (draw)
	{
		uint8_t potion[3] = {
			0b01000000,
			0b11100000,
			0b11100000};
		draw_character(potion_x, potion_y, 3, 3, potion);
	}
}

void update_potion()
{
	int collide = Check_Collision_Potion(potion_cordinate, jerry_block);

	if (collide == 1 && set_potion)
	{
		superSaya = true;
		found_potion = true;
		set_potion = false;
		potion_x = -5;
		potion_y = -5;
	}

	if (collide == -1)
	{
		set_potion = true;
	}
}

void update_wall()
{
	if (right_adc < 341 || right_adc > 341 * 2)
	{
		if (Wall_Collision2(tom_block, wall_cordinate, 0, 2 * tom_dy) && Wall_Collision2(tom_block, wall_cordinate, 2 * tom_dx, 0))
		{
			setup_tom();
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (wall_cordinate[i][0] < wall_cordinate[i][2] && wall_cordinate[i][1] > wall_cordinate[i][3])
		{
			if (wall_cordinate[i][3] >= LCD_Y || wall_cordinate[i][0] >= LCD_X || wall_cordinate[i][3] <= STATUS_BAR_HEIGHT || wall_cordinate[i][0] <= 0)
			{
				wall_cordinate[i][0] = wall_initial_cordinate[i][0];
				wall_cordinate[i][1] = wall_initial_cordinate[i][1];
				wall_cordinate[i][2] = wall_initial_cordinate[i][2];
				wall_cordinate[i][3] = wall_initial_cordinate[i][3];
			}
		}
		if (wall_cordinate[i][0] == wall_cordinate[i][2])
		{
			if (wall_cordinate[i][0] <= 0 || wall_cordinate[i][0] >= LCD_X)
			{
				wall_cordinate[i][0] = wall_initial_cordinate[i][0];
				wall_cordinate[i][1] = wall_initial_cordinate[i][1];
				wall_cordinate[i][2] = wall_initial_cordinate[i][2];
				wall_cordinate[i][3] = wall_initial_cordinate[i][3];
			}
		}
		if (wall_cordinate[i][1] == wall_cordinate[i][3])
		{
			if (wall_cordinate[i][1] >= LCD_Y || wall_cordinate[i][1] <= STATUS_BAR_HEIGHT)
			{
				wall_cordinate[i][0] = wall_initial_cordinate[i][0];
				wall_cordinate[i][1] = wall_initial_cordinate[i][1];
				wall_cordinate[i][2] = wall_initial_cordinate[i][2];
				wall_cordinate[i][3] = wall_initial_cordinate[i][3];
			}
		}
		if (wall_cordinate[i][0] < wall_cordinate[i][2] && wall_cordinate[i][1] < wall_cordinate[i][3])
		{
			if (wall_cordinate[i][2] == LCD_Y)
			{
				wall_cordinate[i][0] = wall_initial_cordinate[i][0];
				wall_cordinate[i][1] = wall_initial_cordinate[i][1];
				wall_cordinate[i][2] = wall_initial_cordinate[i][2];
				wall_cordinate[i][3] = wall_initial_cordinate[i][3];
			}
		}
	}
}

char buffer[200];

void print_process()
{
	static uint8_t closeState = 0;
	int16_t char_code = usb_serial_getchar();
	update_jerry(char_code);
	if (char_code >= 0)
	{
		snprintf(buffer, sizeof(buffer), "received '%c'\r\n", char_code);
		usb_serial_send(buffer);
	}

	if (char_code == 'i')
	{
		snprintf(buffer, sizeof(buffer), "===GAME INFO===\r\nTIME: %02d:%02d\r\nLEVEL: %d \r\nJERRY LIVES: %d\r\nSCORE : %d\r\nFIREWOKRS: %d\r\nMOUSETRAP:%d\r\nCHEESE:%d\r\nCHEESE CONSUMED:%d\r\nSUPER JERRY:%s\r\nPAUSE:%s", time / 60, time % 60, level, lives, score, num_firework, count_t, count_c, cheese_eaten, (superSaya ? "YES" : "NO"), (pause ? "YES" : "NO"));
		usb_serial_send(buffer);
	}

	if (switch_state[0] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received  up switch %lf %lf\r\n", firework_x[5], firework_y[10]);
		usb_serial_send(buffer);
	}

	if (switch_state[1] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received  down switch %lf\r\n", speed);
		usb_serial_send(buffer);
	}

	if (switch_state[2] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received  left switch %lf\r\n", speed);
		usb_serial_send(buffer);
	}

	if (switch_state[3] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received  right switch %lf\r\n", speed);
		usb_serial_send(buffer);
	}

	if (switch_state[4] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received  center switch\r\n");
		usb_serial_send(buffer);
	}

	if (switch_state[5] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received BUTTON B %d %d\r\n", jerryX, jerryY);
		usb_serial_send(buffer);
	}

	if (switch_state[6] != closeState)
	{
		snprintf(buffer, sizeof(buffer), "received BUTTON A %d\r\n", num_firework); //error
		usb_serial_send(buffer);
	}
}

void reading_file()
{
	if (usb_serial_available())
	{
		char tx_buffer[100];
		int c = usb_serial_getchar();
		if (c == 'T')
		{
			usb_serial_read_string(tx_buffer);
			usb_serial_send(tx_buffer);
			sscanf(tx_buffer, "%d %d", &tomX, &tomY);
		}

		if (c == 'J')
		{
			usb_serial_read_string(tx_buffer);
			usb_serial_send(tx_buffer);
			sscanf(tx_buffer, "%d %d", &jerryX, &jerryY);
		}

		if (c == 'W')
		{
			char wall[200];
			usb_serial_read_string(wall);
			usb_serial_send(wall);
			sscanf(wall, "%d %d %d %d", &wall_cordinate2[num_wall][0], &wall_cordinate2[num_wall][1], &wall_cordinate2[num_wall][2], &wall_cordinate2[num_wall][3]);
			num_wall++;
		}
	}
}

void up_level()
{
	level = 2;
	clicked2 = false;
	new_level = true;
	drawpotion = true;
}

void end_game();

void update_game()
{
	if(cheese_eaten == 5 && set_door){
		setup_door();
		set_door = false;
	}

	if (switch_state[5] != 0 && clicked1)
	{
		pause = !(pause);
		game_paused = !(game_paused);
		clicked1 = false;
	}

	if (switch_state[5] == 0)
	{
		clicked1 = true;
	}

	if (switch_state[6] != 0 && !advance_level && clicked2)
	{
		game_over = true;
		clicked2 = false;
	}

	if (switch_state[6] == 0)
	{
		clicked2 = true;
	}

	if (switch_state[6] != 0 && clicked2)
	{
		if (advance_level)
		{
			up_level();
			clicked2 = false;
			advance_level = false;
		}
	}

	if (switch_state[6] == 0)
	{
		clicked2 = true;
	}

	if (new_level)
	{
		//reset array
		memset(wall_cordinate, 0, sizeof(wall_cordinate));
		memset(jerry_block, 0, sizeof(jerry_block));
		memset(tom_block, 0, sizeof(tom_block));
		memset(cheese_cordinate, 0, sizeof(cheese_cordinate));
		memset(door_cordinate, 0, sizeof(door_cordinate));
		setup_jerry();
		setup_tom();
		for (int i = 0; i < 4; i++)
		{
			wall_cordinate[i][0] = wall_cordinate2[i][0];
			wall_cordinate[i][1] = wall_cordinate2[i][1];
			wall_cordinate[i][2] = wall_cordinate2[i][2];
			wall_cordinate[i][3] = wall_cordinate2[i][3];
			wall_initial_cordinate[i][0] = wall_cordinate[i][0];
			wall_initial_cordinate[i][1] = wall_cordinate[i][1];
			wall_initial_cordinate[i][2] = wall_cordinate[i][2];
			wall_initial_cordinate[i][3] = wall_cordinate[i][3];
		}
		setup_cheese();
		setup_firework_values();
		new_level = false;
	}

	if (lives == 0)
	{
		game_over = true;
	}

	if (superSaya)
	{
		pwm_light();
		DDRB = 0xFF;
	}
}

void draw_all()
{
	clear_screen();

	draw_status();
	draw_wall();
	draw_jerry();
	draw_tom();
	draw_cheese();
	draw_trap();
	draw_door();
	draw_firework();
	if (level == 2)
	{
		draw_potion();
	}
	show_screen();
}
void setup();

void end_game()
{
	clear_screen();
	draw_string(15, 15, "GAME OVER!", FG_COLOUR);
	draw_string(15, 25, "PRESS A", FG_COLOUR);
	draw_string(15, 35, "TO RESTART!", FG_COLOUR);
	if (switch_state[6] != 0 && clicked2)
	{
		setup();
		overflow_counter = 0;
	}
	if (switch_state[6] == 0)
	{
		clicked2 = true;
	}
	show_screen();
}

void setup(void)
{
	enter_game = game_paused = pause = new_level = superSaya = drawpotion = draw = found_potion = game_over = false;
	next_cheese = set_door = next_trap = clicked = clicked1 = clicked2 = found_cheese = found_trap = game_start = set_potion = found_door = advance_level = true;
	speed = 0.5;
	distance = 1;
	count_t = 0;
	lives = 5;
	level = 1;
	error = 0;
	score = 0;
	num_wall = 0;
	//TIMER 0
	TCCR0A = 0;
	TCCR0B = 0b100;
	TIMSK0 = 1;
	//TIMER 1
	TCCR1A = 0;
	TCCR1B = 0b010;
	TIMSK1 = 1;
	//TIMER 3
	TCCR3A = 0;
	TCCR3B = 0b011;
	TIMSK3 = 1;
	//TIMER 4
	CLEAR_BIT(TCCR4B, WGM02);
	TCCR4B |= (1 << CS00);
	SET_BIT(TIMSK4, TOIE4);
	channel1 = 0 * 50;
	DDRB = 0xFF;
	sei();
	setup_switches();
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	SET_BIT(DDRC, 2);
	usb_init();
	adc_init();
}

void process(void)
{
	reading_file();
	if (!enter_game)
	{
		wall_initial();
		draw_start_up_screen();
		setup_jerry();
		setup_tom();
	}
	else
	{
		right_adc = adc_read(1);
		left_adc = adc_read(0);
		distance = 2 * 1 * left_adc / 1023.0;
		print_process();
		timer();
		draw_all();
		if (game_start)
		{
			setup_cheese();
			setup_firework_values();
			game_start = false;
		}
		update_tom();
		update_cheese();
		update_trap();
		update_game();
		update_firework();
		update_wall();
		update_potion();
		update_door();
		PORTC |= (1 << 7);
	}
}

int main(void)
{
	setup();

	for (;;)
	{
		if (!game_over)
		{
			process();
			_delay_ms(10);
		}
		else
		{
			end_game();
		}
	}

	return 0;
}