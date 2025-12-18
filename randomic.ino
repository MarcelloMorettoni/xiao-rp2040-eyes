#include <Arduino.h>
#include <U8x8lib.h>

// --- CONFIGURATION ---
#define BUTTON_PIN D1
#define LONG_PRESS_MS 3000

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

// --- MODES ---
// 0=Idle, 1=Happy, 2=Angry, 3=Tired, 4=Sleeping, 5=Luck, 6=Hypnotized, 7=Love
int currentMode = 0;

//ZZ
unsigned long lastZAnim = 0;
int zStep = 0;

// --- BUTTON STATE ---
int lastButtonState = HIGH;
unsigned long buttonDownTime = 0;
bool ignoreNextRelease = false;

// --- MODE STATE ---
bool modeChanged = true;

// --- IDLE ANIMATION STATE ---
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000;
unsigned long lastMoveTime = 0;
unsigned long nextMoveInterval = 2000;

bool isBlinking = false;
int blinkType = 0;
int lookOffset = 0;

// --- LOVE ANIMATION ---
unsigned long lastLoveAnim = 0;
bool loveExpanded = false;

void setup() {
  u8x8.begin();
  u8x8.setPowerSave(0);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8x8.clearDisplay();
}

void loop() {
  handleButton();

  switch (currentMode) {
    case 0: runIdleAnimation(); break;
    case 1: drawHappy(); break;
    case 2: drawAngry(); break;
    case 3: drawTired(); break;
    case 4: drawSleeping(); break;
    case 5: drawHypnotized(); break;
    case 6: drawLove(); break;
  }
}

// --- BUTTON LOGIC ---
void handleButton() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && lastButtonState == HIGH) {
    buttonDownTime = millis();
    ignoreNextRelease = false;
  }

  // Long press reset
  if (reading == LOW && millis() - buttonDownTime > LONG_PRESS_MS && !ignoreNextRelease) {
    currentMode = 0;
    ignoreNextRelease = true;
    modeChanged = true;
    u8x8.clearDisplay();
  }

  // Short press: cycle mode
  if (reading == HIGH && lastButtonState == LOW) {
    if (!ignoreNextRelease) {
      currentMode++;
      if (currentMode > 6) currentMode = 0;
      lookOffset = 0;
      modeChanged = true;
      u8x8.clearDisplay();
    }
  }

  lastButtonState = reading;
}

// --- MODE 0: IDLE ---

void runIdleAnimation() {
  unsigned long currentTime = millis();

  // 1. BLINK LOGIC
  // Handles opening and closing eyes, but NEVER moves them.
  if (currentTime - lastBlinkTime > nextBlinkInterval) {
    lastBlinkTime = currentTime;

    if (!isBlinking) {
      // START BLINK
      isBlinking = true;
      nextBlinkInterval = 200; // Blink duration

      // Decide: blink or wink
      int dice = random(0, 10);
      if (lookOffset == 0 && dice > 5) {
        blinkType = (dice < 8) ? 1 : 2; 
      } else {
        blinkType = 0; 
      }
      u8x8.clearDisplay(); // Clear to redraw closed state

    } else {
      // END BLINK
      isBlinking = false;
      nextBlinkInterval = random(1000, 4000); // Random wait until next blink
      u8x8.clearDisplay(); // Clear to redraw open state
    }
  }

  // 2. MOVE LOGIC
  // Only moves eyes if they are NOT blinking.
  // This prevents the "teleport while closed" glitch.
  if (currentTime - lastMoveTime > nextMoveInterval) {
    if (!isBlinking) {
      lastMoveTime = currentTime;
      nextMoveInterval = random(1500, 4000); 

      int roll = random(0, 3);
      int newOffset = (roll == 1) ? -2 : (roll == 2 ? 2 : 0);
      
      if (newOffset != lookOffset) {
        lookOffset = newOffset;
        u8x8.clearDisplay(); // Redraw in new position
      }
    }
  }

  int leftX = 3 + lookOffset;
  int rightX = 9 + lookOffset;

  // 3. DRAW
  if (isBlinking) {
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    // Left Eye Logic
    if (blinkType == 0 || blinkType == 1) {
      u8x8.drawString(leftX, 4, "____");
    } else {
      u8x8.setFont(u8x8_font_open_iconic_play_4x4);
      u8x8.drawGlyph(leftX, 2, 'K');
    }

    // Right Eye Logic
    if (blinkType == 0 || blinkType == 2) {
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.drawString(rightX, 4, "____");
    } else {
      u8x8.setFont(u8x8_font_open_iconic_play_4x4);
      u8x8.drawGlyph(rightX, 2, 'K');
    }

  } else {
    // Eyes open normally
    u8x8.setFont(u8x8_font_open_iconic_play_4x4);
    u8x8.drawGlyph(leftX, 2, 'K');
    u8x8.drawGlyph(rightX, 2, 'K');
  }
}


// --- MODE 1: HAPPY ---
void drawHappy() {
  u8x8.setFont(u8x8_font_open_iconic_arrow_4x4);
  u8x8.drawGlyph(3, 2, 'S');
  u8x8.drawGlyph(9, 2, 'S');
}

// --- MODE 2: ANGRY (FROZEN) ---
void drawAngry() {
  if (!modeChanged) return;
  modeChanged = false;

  u8x8.clearDisplay();

  u8x8.setFont(u8x8_font_open_iconic_play_4x4);
  u8x8.drawGlyph(3, 2, 'K');
  u8x8.drawGlyph(9, 2, 'K');

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(3, 1, "\\__");
  u8x8.drawString(9, 1, " __/");
}

// --- MODE 3: TIRED ---
void drawTired() {
  u8x8.setFont(u8x8_font_open_iconic_play_2x2);
  u8x8.drawGlyph(2, 4, 'K');
  u8x8.drawGlyph(4, 4, 'K');
  u8x8.drawGlyph(10, 4, 'K');
  u8x8.drawGlyph(12, 4, 'K');
}

// --- MODE 4: SLEEPING ---
void drawSleeping() {
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // Closed eyes (static)
  u8x8.drawString(3, 4, "____");
  u8x8.drawString(9, 4, "____");

  unsigned long now = millis();
  if (now - lastZAnim < 500) return;  // animation speed
  lastZAnim = now;

  // Clear only the Z area
  u8x8.drawString(13, 0, " ");
  u8x8.drawString(14, 1, " ");
  u8x8.drawString(15, 2, " ");

  // Draw Z drifting upward
  switch (zStep) {
    case 0:
      u8x8.drawString(13, 2, "Z");
      break;
    case 1:
      u8x8.drawString(14, 1, "z");
      break;
    case 2:
      u8x8.drawString(15, 0, "z");
      break;
  }

  zStep++;
  if (zStep > 2) zStep = 0;
}


// --- MODE 5: HYPNOTIZED ---
void drawHypnotized() {
  u8x8.setFont(u8x8_font_profont29_2x3_r);
  u8x8.drawString(3, 2, "@");
  u8x8.drawString(9, 2, "@");
}

// --- MODE 6: LOVE (ANIMATED DOTS) ---
void drawLove() {
  unsigned long now = millis();
  if (now - lastLoveAnim < 250) return;
  lastLoveAnim = now;

  loveExpanded = !loveExpanded;
  u8x8.clearDisplay();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  if (loveExpanded) {
    u8x8.drawString(4, 1, " ..   .. ");
    u8x8.drawString(3, 2, " .... .... ");
    u8x8.drawString(3, 3, " ......... ");
    u8x8.drawString(4, 4, " ....... ");
    u8x8.drawString(5, 5, " ..... ");
    u8x8.drawString(6, 6, " ... ");
    u8x8.drawString(7, 7, " . ");
  } else {
    u8x8.drawString(4, 2, "  .. ..  ");
    u8x8.drawString(4, 3, "  .....  ");
    u8x8.drawString(4, 4, "   ...   ");
    u8x8.drawString(4, 5, "    .    ");
  }
}
