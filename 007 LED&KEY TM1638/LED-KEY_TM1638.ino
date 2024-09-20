// Definições

#define PinData 5
#define PinClock 6
#define PinStrobe 7
#define COUNTING_MODE 0
#define SCROLL_MODE 1
#define BUTTON_MODE 2

// Funtions Prototypes

void f_reset();
void f_sendCommand(uint8_t value);
bool f_counting();
bool f_scroll();
void f_buttons();
uint8_t f_read_buttons(void);
void f_setLed(uint8_t value, uint8_t position);

// setup

void setup()
{
  pinMode(PinData, OUTPUT);
  pinMode(PinClock, OUTPUT);
  pinMode(PinStrobe, OUTPUT);
  f_sendCommand(0x88);  // Set maximum display brightness
  f_reset();
}

// loop

void loop()
{
  static uint8_t mode = COUNTING_MODE;
  switch (mode)
  {
    case COUNTING_MODE:
      mode += f_counting();
      delay(500);
      break;
    case SCROLL_MODE:
      mode += f_scroll();
      break;
    case BUTTON_MODE:
      f_buttons();
      break;
  }
  delay(200);
}

void f_reset()
{
  f_sendCommand(0x40); // Set auto increment mode
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, 0xc0);   // Set starting address to 0
  for (uint8_t idx = 0; idx < 16; idx++)
  {
    shiftOut(PinData, PinClock, LSBFIRST, 0x00);
  }
  digitalWrite(PinStrobe, HIGH);
}

void f_sendCommand(uint8_t value)
{
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, value);
  digitalWrite(PinStrobe, HIGH);
}

bool f_counting()
{
  /*0*/ /*1*/ /*2*/ /*3*/ /*4*/ /*5*/ /*6*/ /*7*/ /*8*/ /*9*/
  uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
  static uint8_t digit = 0;
  f_sendCommand(0x40);
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, 0xc0);
  for (uint8_t idx = 0; idx < 8; idx++)
  {
    shiftOut(PinData, PinClock, LSBFIRST, digits[digit]);
    shiftOut(PinData, PinClock, LSBFIRST, 0x00);
  }
  digitalWrite(PinStrobe, HIGH);
  digit = ++digit % 10;
  return digit == 0;
}

bool f_scroll()
{
  uint8_t scrollText[] =
  {
    /* */ /* */ /* */ /* */ /* */ /* */ /* */ /* */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*H*/ /*E*/ /*L*/ /*L*/ /*O*/ /*.*/ /*.*/ /*.*/
    0x76, 0x79, 0x38, 0x38, 0x3f, 0x80, 0x80, 0x80,
    /* */ /* */ /* */ /* */ /* */ /* */ /* */ /* */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*H*/ /*E*/ /*L*/ /*L*/ /*O*/ /*.*/ /*.*/ /*.*/
    0x76, 0x79, 0x38, 0x38, 0x3f, 0x80, 0x80, 0x80,
  };
  static uint8_t index = 0;
  uint8_t scrollLength = sizeof(scrollText);
  f_sendCommand(0x40);
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, 0xc0);

  for (int idx = 0; idx < 8; idx++)
  {
    uint8_t c = scrollText[(index + idx) % scrollLength];
    shiftOut(PinData, PinClock, LSBFIRST, c);
    shiftOut(PinData, PinClock, LSBFIRST, c != 0 ? 1 : 0);
  }

  digitalWrite(PinStrobe, HIGH);
  index = ++index % (scrollLength << 1);
  return index == 0;
}

void f_buttons()
{
  uint8_t promptText[] =
  {
    /*P*/ /*r*/ /*E*/ /*S*/ /*S*/ /* */ /* */ /* */
    0x73, 0x50, 0x79, 0x6d, 0x6d, 0x00, 0x00, 0x00,
    /* */ /* */ /* */ /* */ /* */ /* */ /* */ /* */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*b*/ /*u*/ /*t*/ /*t*/ /*o*/ /*n*/ /*S*/ /* */
    0x7c, 0x1c, 0x78, 0x78, 0x5c, 0x54, 0x6d, 0x00,
    /* */ /* */ /* */ /* */ /* */ /* */ /* */ /* */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };
  static uint8_t block = 0;
  uint8_t textStartPos = (block / 4) << 3;

  for (uint8_t idx = 0; idx < 8; idx++)
  {
    f_sendCommand(0x44);
    digitalWrite(PinStrobe, LOW);
    shiftOut(PinData, PinClock, LSBFIRST, 0xC0 + (idx << 1));
    shiftOut(PinData, PinClock, LSBFIRST, promptText[textStartPos + idx]);
    digitalWrite(PinStrobe, HIGH);
  }

  block = (block + 1) % 16;
  uint8_t buttons = f_read_buttons();

  for (uint8_t idx = 0; idx < 8; idx++)
  {
    uint8_t mask = 0x1 << idx;
    f_setLed(buttons & mask ? 1 : 0, idx);
  }
}

uint8_t f_read_buttons(void)
{
  uint8_t buttons = 0;
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, 0x42);
  pinMode(PinData, INPUT);

  for (uint8_t idx = 0; idx < 4; idx++)
  {
    uint8_t pressed = shiftIn(PinData, PinClock, LSBFIRST) << idx;
    buttons |= pressed;
  }

  pinMode(PinData, OUTPUT);
  digitalWrite(PinStrobe, HIGH);
  return buttons;
}

void f_setLed(uint8_t value, uint8_t position)
{
  pinMode(PinData, OUTPUT);
  f_sendCommand(0x44);
  digitalWrite(PinStrobe, LOW);
  shiftOut(PinData, PinClock, LSBFIRST, 0xC1 + (position << 1));
  shiftOut(PinData, PinClock, LSBFIRST, value);
  digitalWrite(PinStrobe, HIGH);
}