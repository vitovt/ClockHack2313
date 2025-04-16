/*
 * Created: 21/12/2019 19:11:45
 * Author : Vitovt
 * https://github.com/vitovt/ClockHack2313
 */
#define F_CPU 1000000UL // 1 MHz

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdbool.h>
#include <util/delay.h>

// PINS
//    1    RES
//    2    PD0    CC0 HEX DIGIT 0 COMMON CATHODE
//    3    PD1    CC1 HEX DIGIT 1 COMMON CATHODE
//    4    PA1 XTAL
//    5    PA0 XTAL
//    6    PD2    CC2 HEX DIGIT 2 COMMON CATHODE
//    7    PD3 SOUNDER
//    8    PD4 S1
//    9    PD5    S2
//    10    GND
//    11    PD6    CC3 HEX DIGIT 3 COMMON CATHODE
//    12    PB0    dp ANODE
//    13    PB1    g ANODE
//    14    PB2    c ANODE
//    15    PB3    d ANODE
//    16    PB4    e ANODE
//    17    PB5    b ANODE
//    18    PB6    f ANODE
//    19    PB7    a ANODE
//    20    V+ SUPPLY

// The LED display is wired so that the segment anodes are pulled high by
// resistors and the common cathodes pulled low by PD0,1,2 and 6. To drive the
// display we select a digit by setting its common cathode port pin to output,
// low and control segments by setting their port pins to input for on out
// output, low for off.
//
#define SEG_A 0b10000000 // PB7
#define SEG_B 0b00100000 // PB5
#define SEG_C 0b00000100 // PB2
#define SEG_D 0b00001000 // PB3
#define SEG_E 0b00010000 // PB4
#define SEG_F 0b01000000 // PB6
#define SEG_G 0b00000010 // PB1
#define SEG_H 0b00000001 // PB0 - dot

#define DIGIT_1 0b00000001 // PD0
#define DIGIT_2 0b00000010 // PD1
#define DIGIT_3 0b00000100 // PD2
#define DIGIT_4 0b01000000 // PD6

#define BUZZER 0b00001000  // PD3
#define BUTTON0 0b00010000 // PD4
#define BUTTON1 0b00100000 // PD5

#define ALL_HIGH 0b11111111
#define ALL_LOW 0b00000000

#define SYMBOL_H (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define SYMBOL_E (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define SYMBOL_L (SEG_D | SEG_E | SEG_F)
#define SYMBOL_O (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define SYMBOL_F (SEG_A | SEG_E | SEG_F | SEG_G)
#define SYMBOL_r (SEG_E | SEG_G)
#define SYMBOL_I (SEG_E | SEG_F)
#define SYMBOL_II (SEG_B | SEG_C | SEG_E | SEG_F)
#define SYMBOL_I2 (SEG_B | SEG_C)
#define SYMBOL_n (SEG_C | SEG_E | SEG_G)
#define SYMBOL_d (SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)
#define SYMBOL_S (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define SYMBOL_SPACE 0
#define SYMBOL_A (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define SYMBOL_t (SEG_D | SEG_E | SEG_F | SEG_C)
#define SYMBOL_G (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define SYMBOL_minus (SEG_G)
#define SYMBOL_C (SEG_A | SEG_D | SEG_E | SEG_F)
#define SYMBOL_U (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define SYMBOL_B (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)

/* Digit associations */
uint8_t number[] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,         // 0
    SEG_B | SEG_C,                                         // 1
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                 // 2
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                 // 3
    SEG_B | SEG_C | SEG_F | SEG_G,                         // 4
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                 // 5
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,         // 6
    SEG_A | SEG_B | SEG_C,                                 // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // 8
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G          // 9
};

/* Short IO Info *****
DDRx = Data direction register (0 - input, 1 - output)
PORTx = Output/Input: 0 - low, 1 - high
PORTB |= (1 << PB3);
PORTB &= ~(1 << PB3);
*/

// VARIABLES
const uint8_t digit_select[4] = {1, 2, 4, 64};
const uint8_t digit_codes[10] = {0xfc, 0x24, 0xba, 0xae, 0x66,
                                 0xce, 0xde, 0xa4, 0xfe, 0xee};
uint8_t timer[4] = {0, 0, 0, 0};
uint8_t timer_buf[4] = {0, 0, 0, 0};
volatile uint32_t debug_counter = 0;
uint16_t seconds_timer = 0;
uint16_t second_count = 1000;
volatile bool new_second = false;
volatile bool overflow = false;
volatile bool new_timer = false;
volatile bool timer_enabled = false;

volatile uint32_t timer0 = 0;

uint8_t btn_up_time[2] = {0, 0};
volatile bool btn_press[2] = {false, false};

volatile uint8_t indicator[4];
volatile uint8_t activedigit = 0;

volatile bool creeping_enabled = false;
volatile uint8_t creeping_current = 0;
volatile uint16_t creeping_pause = 0;
volatile uint8_t creeping_delay = 30;
volatile char creeping_string[];
volatile uint8_t creeping_length;

// HELLO FrIEndS
volatile char creeping_string[] = {
    SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_H,     SYMBOL_E,
    SYMBOL_L,     SYMBOL_L,     SYMBOL_O,     SYMBOL_SPACE, SYMBOL_F,
    SYMBOL_r,     SYMBOL_I,     SYMBOL_E,     SYMBOL_n,     SYMBOL_d,
    SYMBOL_S,     SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE};

volatile bool beep_enabled = false;
volatile bool beepshort = false;
volatile uint8_t beep_duration = 0;

// LOCAL FUNCTION DECLARATIONS
void add_second(void);
void do_buttons(void);
void push_display(void);
void creeping_line(void);
void sound_beep(void);
void beep_long(void);
void beep_short(void);

// LOCAL DEFINITIONS
#define BTN_UP_MIN 20 // button debounce time, must have been released for 20 ms to register a  press
#define S1 0 // reset button on left
#define S2 1 // stop start button on right

/* TIME INT 1ms */
ISR(TIMER1_OVF_vect) // timer interrupt, set for 1ms interrupts
{

  TCNT1 =
      0xe000; // we are using 8MHz clock, this gives us 8192 counts per ms which
              // is close enough a really fun exercise is to work out how to
              // calibrate the timing by varying the value loaded into TCNT1
  timer0++;
  if (timer0 == 4000000000) {
    timer0 = 0;
  }

  creeping_line();
  push_display();
  /*
  if (btn_press[0]) {
          btn_press[0] = false;
          beep_long();
          }

  if (btn_press[1]) {
          btn_press[1] = false;
          beep_short();
          }
  */
  do_buttons();
  sound_beep();
}

void do_buttons(void) {
  uint8_t btn = 0;
  const uint8_t btn_mask[2] = {0x10, 0x20};

  for (btn = 0; btn < 2; btn++) {
    if ((PIND & btn_mask[btn]) == 0) // if button pressed
    {
      if (btn_up_time[btn] >= BTN_UP_MIN) {
        btn_press[btn] = true;
      }
      btn_up_time[btn] = 0;
    } else {
      btn_up_time[btn]++;
      if (btn_up_time[btn] > BTN_UP_MIN)
        btn_up_time[btn] = BTN_UP_MIN;
    }
  }
}

void push_display(void) {
  if (activedigit >= 4) {
    activedigit = 0;
  }

  DDRD = DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4 | BUZZER; // 0b01000111;
  // PORTD = 0xff;
  // PORTD = ~digit_select[activedigit];

  PORTD = PORTD | (DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4 |
                   BUZZER); // 1 to all digit select and 1 - buzzer off
  PORTD = PORTD & ~digit_select[activedigit];
  // PORTD = 0b00000111;

  DDRB = ~indicator[activedigit];

  activedigit++;
}

void creeping_line() {
  uint8_t display_size = 4;
#define line creeping_string

  uint8_t line_length = creeping_length;

  if (creeping_enabled) {

    if (++creeping_pause >= creeping_delay) {
      creeping_pause = 0;
      for (int i = 0; i <= display_size; i++) {
        indicator[i] = line[creeping_current + i];
      }
      creeping_current++;

      if (creeping_current == (line_length - display_size + 1)) {
        creeping_enabled = false;
      }
    }
  }
}

void sound_beep() {
#define BEEP_LIMIT 30

  if (beep_enabled) {
    if (++beep_duration < BEEP_LIMIT) {
      PORTD &= ~BUZZER; // on buzzer

      if (beepshort && beep_duration > (BEEP_LIMIT / 2 - BEEP_LIMIT / 10) &&
          beep_duration < (BEEP_LIMIT / 2 + BEEP_LIMIT / 10)) {
        PORTD |= BUZZER; // off buzzer
      }
    } else {
      beep_enabled = false;
      PORTD |= BUZZER; // off buzzer
    }
  }
}

void beep_long() {
  beepshort = false;
  beep_duration = 0;
  beep_enabled = true;
}

void beep_short() {
  beepshort = true;
  beep_duration = 0;
  beep_enabled = true;
}

void creeping_start(char *line, uint8_t length, uint8_t delay) {
  creeping_current = 0;
  creeping_pause = 0;
  creeping_delay = delay;
  creeping_string = line;
  creeping_length = length;
  creeping_enabled = true;
}

void clear_screen_right() {
  const char delay = 100;

  for (int i = 0; i <= 3; i++) {
    indicator[i] = SYMBOL_I;
    _delay_ms(delay);
    indicator[i] = SYMBOL_II;
    _delay_ms(delay);
  }

  /*	indicator[0] = ALL_HIGH;
          indicator[1] = ALL_HIGH;
          indicator[2] = ALL_HIGH;
          indicator[3] = ALL_HIGH;
          _delay_ms(delay);

          indicator[0] = SYMBOL_II;
          indicator[1] = SYMBOL_II;
          indicator[2] = SYMBOL_II;
          indicator[3] = SYMBOL_II;
          _delay_ms(delay); */

  for (int i = 0; i <= 3; i++) {
    indicator[i] = SYMBOL_I2;
    _delay_ms(delay);
    indicator[i] = ALL_LOW;
    _delay_ms(delay);
  }
}

void clear_screen_left() {
  const char delay = 100;

  for (int i = 3; i >= 0; i--) {
    indicator[i] = SYMBOL_I2;
    _delay_ms(delay);
    indicator[i] = SYMBOL_II;
    _delay_ms(delay);
  }

  for (int i = 3; i >= 0; i--) {
    indicator[i] = SYMBOL_I;
    _delay_ms(delay);
    indicator[i] = ALL_LOW;
    _delay_ms(delay);
  }
}

void game_0() {
  indicator[0] = SYMBOL_n;
  indicator[1] = SYMBOL_O;
  indicator[2] = SYMBOL_minus;
  indicator[3] = SYMBOL_minus;

  _delay_ms(1000);
}

void game_1() {
#define MAXCUBEDIGIT 6
  int increment1 = 1;
  int increment2 = 1;

  clear_screen_right();

  int cub1 = 0;
  int cub2 = 0;

  cub1 = TCNT1 % 7;
  cub2 = TCNT1 % 7;

  //	srand(timer0);
  //	cub1 = rand() / (RAND_MAX / 6 + 1);

  //	srand(timer0);
  //	cub2 = rand() / (RAND_MAX / 6 + 1);

  indicator[0] = SYMBOL_C;
  indicator[1] = SYMBOL_U;
  indicator[2] = SYMBOL_B;
  indicator[3] = SYMBOL_E;
  _delay_ms(1000);

  indicator[2] = ALL_LOW;
  indicator[1] = ALL_LOW;
  while (1) {
    indicator[0] = number[cub1];
    indicator[3] = number[cub2];

    cub1 += increment1;
    cub2 += increment2;

    if (cub1 > MAXCUBEDIGIT) {
      cub1 = 1;
    }
    if (cub2 > MAXCUBEDIGIT) {
      cub2 = 1;
    }
    _delay_ms(100);

    if (btn_press[1]) {
      btn_press[1] = false;
      break;
    }
    if (btn_press[0]) {
      btn_press[0] = false;
      if (increment1) {
        increment1 = 0;
      } else {
        if (increment2) {
          increment2 = 0;
        } else {
          increment1 = 1;
          increment2 = 1;
        }
      }
    }
  }
  btn_press[1] = false;
}

void game_2() {
  clear_screen_right();
  indicator[0] = SYMBOL_G;
  indicator[1] = SYMBOL_O;
  indicator[2] = SYMBOL_minus;
  indicator[3] = number[2];

  _delay_ms(1000);
}

int main(void) {
  /* Interrupts */
  TCCR1A = 0;
  TCCR1B = 0x01; // timer in timer mode, clock full speed
  TIMSK = 0x80;  // mask for overflow is enabled

  // Set all DIGIT select on PortD to LOW
  DDRD = DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4 | BUZZER;     // 0b01001111;
  PORTD = ~(DIGIT_1 | DIGIT_2 | DIGIT_3 | DIGIT_4 | BUZZER); // 0b10111000;

  DDRB = 0x00;  // this is the default but it doesn't hurt to be sure, all
                // segments as inputs
  PORTB = 0x00; // all segment drives low (when set as outputs)

  // DDRD = 0x08;            // sounder as output, all else as inputs
  // PORTD = 0x38;            // pull ups on the switch pins activated, sounder
  // off

  // LEDs are on portB 0 and 1
  DDRB = ~ALL_HIGH; // All segments HIGH

  _delay_ms(1000);
  DDRB = ALL_HIGH;
  PORTD |= BUZZER;

  _delay_ms(1000);

  PORTD &= ~BUZZER;

  sei(); // clear global interrupt mask - enable interrupts
  // cli();						//disable interrupts

  // HELLO FrIEndS
  char line[] = {SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE,
                 SYMBOL_H, SYMBOL_E, SYMBOL_L, SYMBOL_L, SYMBOL_O,
                 SYMBOL_SPACE,
                 SYMBOL_F, SYMBOL_r, SYMBOL_I, SYMBOL_E, SYMBOL_n, SYMBOL_d, SYMBOL_S,
                 SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE, SYMBOL_SPACE};

  creeping_start(line, sizeof(line), 33);

  while (creeping_enabled) {
  }

  _delay_ms(1500);
  indicator[0] = SYMBOL_G;
  indicator[1] = SYMBOL_A;
  indicator[2] = SYMBOL_minus;
  indicator[3] = number[1]; // O

  beep_long();
  int game = 1;
  while (1) {

    indicator[3] = number[game];

    if (btn_press[0]) {
      btn_press[0] = false;
      beep_long();
      game++;
    }

    // if (btn_press[1]) {
    //	btn_press[1] = false;
    //	beep_short();
    //	game--;
    //	}

    if (game > 9) {
      game = 1;
    }
    if (game < 1) {
      game = 9;
    }

    if (btn_press[1]) {
      btn_press[1] = false;

      switch (game) {

      case 1:
        game_1();
        break;

      case 2:
        game_2();
        break;

      default:
        game_0();
        break;
      }

      clear_screen_left();

      indicator[0] = SYMBOL_G;
      indicator[1] = SYMBOL_A;
      indicator[2] = SYMBOL_minus;
      indicator[3] = number[1]; // O

      beep_short();
      game = 1;
    }

    //	clear_screen_left();

    /*
    indicator[3] = SYMBOL_H; //H
    indicator[2] = 0b00000000; //empty
    indicator[1] = 0b00000000; //empty
    indicator[0] = 0b00000000; //empty

    _delay_ms(1500);
    indicator[3] = SYMBOL_E; //E
    indicator[2] = SYMBOL_H; //H
    indicator[1] = 0b00000000; //empty
    indicator[0] = 0b00000000; //empty

    _delay_ms(1500);
    indicator[3] = SYMBOL_L; //L
    indicator[2] = SYMBOL_E; //E
    indicator[1] = SYMBOL_H; //H
    indicator[0] = 0b00000000; //empty


    _delay_ms(1500);
    indicator[3] = SYMBOL_O; //O
    indicator[2] = SYMBOL_L; //L
    indicator[1] = SYMBOL_E; //E
    indicator[0] = SYMBOL_H; //H

    _delay_ms(1500);
    indicator[3] = 0b00000000; //empty
    indicator[2] = 0b00000000; //empty
    indicator[1] = 0b00000000; //empty
    indicator[0] = 0b00000000; //empty

    _delay_ms(1000);
    indicator[3] = SYMBOL_O; //O
    indicator[2] = SYMBOL_L; //L
    indicator[1] = SYMBOL_E; //E
    indicator[0] = SYMBOL_H; //H

    _delay_ms(1500);
    indicator[3] = 0b00000000; //empty
    indicator[2] = 0b00000000; //empty
    indicator[1] = 0b00000000; //empty
    indicator[0] = 0b00000000; //empty

    _delay_ms(1000);
    indicator[3] = SYMBOL_O; //O
    indicator[2] = SYMBOL_L; //L
    indicator[1] = SYMBOL_E; //E
    indicator[0] = SYMBOL_H; //H

    _delay_ms(1500);
    indicator[3] = 0b00000000; //empty
    indicator[2] = 0b00000000; //empty
    indicator[1] = 0b00000000; //empty
    indicator[0] = 0b00000000; //empty
    */
    //_delay_ms(500);
  }
}
