//We always have to include the library
#include "LedControl.h"

const byte PIN_LED_R = A5;
const byte PIN_LED_G = A4;
const byte PIN_LED_B = A3;
const byte PIN_MAX7221_SEL = A2;
const byte PIN_MAX7221_DATA = A1;
const byte PIN_MAX7221_CLK = A0;

/** \brief Number of rows in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_ROWS = 8;

/** \brief Number of columns in the C16/Plus4 keyboard matrix
 */
const byte MATRIX_COLS = 8;

const byte PIN_ROWS[MATRIX_ROWS] = {17 /* RXLED */, 15 /* SCK */, 16 /* MOSI */, 14 /* MISO */, 8, 9, 10, 11};
const byte PIN_COLS[MATRIX_COLS] = {3, 2, 0, 1, 4, 30 /* TXLED */, 12, 6};

LedControl lc(PIN_MAX7221_DATA, PIN_MAX7221_CLK, PIN_MAX7221_SEL, 1 /* Number of MAX72xx chips */);

/* we always wait a bit between updates of the display */
unsigned long DELAY_TIME = 35;

enum class Key {
  // Numbers
  _0,
  _1,
  _2,
  _3,
  _4,
  _5,
  _6,
  _7,
  _8,
  _9,
  
  // Letters
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  
  // Function
  F1,
  F2,
  F3,
  HELP,
  
  // Cursor
  UP,
  DOWN,
  LEFT,
  RIGHT,
  
  // Others
  ASTERISK,
  AT,
  CLEAR,
  CMD,
  COLON,
  COMMA,
  CTRL,
  DEL,
  ENTER,
  EQUAL,
  ESC,
  MINUS,
  PERIOD,
  PLUS,
  POUND,
  RUNSTOP,
  SEMICOLON,
  SHIFT,
  SLASH,
  SPACE
};

const byte N_KEYS = static_cast<byte> (Key::SPACE) + 1;


// Order in which they appear on the keyboard (+1 for 2 Shifts)
constexpr Key splash_order[N_KEYS + 1] = {
  Key::ESC, Key::_1, Key::_2, Key::_3, Key::_4, Key::_5, Key::_6, Key::_7, Key::_8, Key::_9, Key::_0, Key::LEFT, Key::RIGHT, Key::UP, Key::DOWN, Key::DEL,
  Key::CTRL, Key::Q, Key::W, Key::E, Key::R, Key::T, Key::Y, Key::U, Key::I, Key::O, Key::P, Key::AT, Key::PLUS, Key::MINUS, Key::CLEAR,
  Key::RUNSTOP, /* Shift Lock */ Key::A, Key::S, Key::D, Key::F, Key::G, Key::H, Key::J, Key::K, Key::L, Key::COLON, Key::SEMICOLON, Key::ASTERISK, Key::ENTER,
  Key::CMD, Key::SHIFT, Key::Z, Key::X, Key::C, Key::V, Key::B, Key::N, Key::M, Key::COMMA, Key::PERIOD, Key::SLASH, Key::SHIFT, Key::POUND, Key::EQUAL,
  Key::SPACE,
  Key::HELP, Key::F3, Key::F2, Key::F1    // Reverse order just to be cool ;)
};

constexpr Key keymap[MATRIX_ROWS][MATRIX_COLS] = {
  {Key::DEL,   Key::_3,    Key::_5, Key::_7, Key::_9, Key::DOWN,   Key::LEFT,      Key::_1},
  {Key::ENTER, Key::W,     Key::R,  Key::Y,  Key::I,  Key::P,      Key::ASTERISK,  Key::CLEAR},
  {Key::POUND, Key::A,     Key::D,  Key::G,  Key::J,  Key::L,      Key::SEMICOLON, Key::CTRL},
  {Key::HELP,  Key::_4,    Key::_6, Key::_8, Key::_0, Key::UP,     Key::RIGHT,     Key::_2},
  {Key::F1,    Key::Z,     Key::C,  Key::B,  Key::M,  Key::PERIOD, Key::ESC,       Key::SPACE},
  {Key::F2,    Key::S,     Key::F,  Key::H,  Key::K,  Key::COLON,  Key::EQUAL,     Key::CMD},
  {Key::F3,    Key::E,     Key::T,  Key::U,  Key::O,  Key::MINUS,  Key::PLUS,      Key::Q},
  {Key::AT,    Key::SHIFT, Key::X,  Key::V,  Key::N,  Key::COMMA,  Key::SLASH,     Key::RUNSTOP}
};

struct Position {
  byte row;
  byte col;
};

Position positions[N_KEYS];

void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity (0, 15);
  /* and clear the display */
  lc.clearDisplay (0);

  for (byte i = 0; i < N_KEYS; ++i) {
    Key k = static_cast<Key> (i);

    bool found = false;
    byte row, col;
    for (row = 0; row < MATRIX_ROWS && !found; ++row) {
      for (col = 0; col < MATRIX_COLS && !found; ++col) {
        if (keymap[row][col] == k) {
          positions[i].row = row;
          positions[i].col = col;
          found = true;
        }
      }
    }

    if (!found) {
      while (true) {
      }
    }
  }

  pinMode (PIN_LED_R, OUTPUT);
  pinMode (PIN_LED_G, OUTPUT);
  pinMode (PIN_LED_B, OUTPUT);


  for (byte i = 0; i < N_KEYS + 1; ++i) {
    const Position& pos = positions[static_cast<int> (splash_order[i])];
    lc.setLed(0, pos.row, (pos.col + 1) % 8, true);   // Library numbers columns differently, so cope with it
    delay(DELAY_TIME);
    lc.setLed(0, pos.row, (pos.col + 1) % 8, false);
  }
}


void loop() { 
  digitalWrite (PIN_LED_R, HIGH);
  delay (1000);
  digitalWrite (PIN_LED_R, LOW);
  delay (1000);
}
