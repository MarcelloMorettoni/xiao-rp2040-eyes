#include <Arduino.h>
#include <U8x8lib.h>

// --- CONFIGURATION ---
#define BUTTON_PIN D1 
#define LONG_PRESS_MS 3000

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// --- MODES ---
// 0=Idle, 1=Happy, 2=Angry, 3=Tired, 4=Love
int currentMode = 0; 

// --- BUTTON STATE ---
int lastButtonState = HIGH;
unsigned long buttonDownTime = 0;
bool ignoreNextRelease = false;

// --- ANIMATION STATE ---
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkInterval = 3000;
bool isBlinking = false;
int blinkType = 0; // 0=Both, 1=Left, 2=Right
int lookOffset = 0; 

void setup(void) {
  u8x8.begin();
  u8x8.setPowerSave(0);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8x8.clearDisplay();
}

void loop(void) {
  handleButton(); 

  // Select the animation based on current mode
  switch(currentMode) {
    case 0: runIdleAnimation(); break;
    case 1: drawHappy(); break;
    case 2: drawAngry(); break;
    case 3: drawTired(); break;
    case 4: drawLove(); break;
  }
}

// --- BUTTON LOGIC (Short vs Long Press) ---
void handleButton() {
  int reading = digitalRead(BUTTON_PIN);

  // 1. Button Just Pressed
  if (reading == LOW && lastButtonState == HIGH) {
    buttonDownTime = millis();
    ignoreNextRelease = false;
  }

  // 2. Button Held Down (Check for Long Press)
  if (reading == LOW && (millis() - buttonDownTime > LONG_PRESS_MS) && !ignoreNextRelease) {
    // LONG PRESS DETECTED -> RESET TO IDLE
    currentMode = 0;
    ignoreNextRelease = true; // Prevent switching mode again when user lets go
    u8x8.clearDisplay();
    // Optional: Flash screen to confirm reset
    u8x8.setInverseFont(1);
    u8x8.drawString(4,3," RESET ");
    delay(500);
    u8x8.setInverseFont(0);
    u8x8.clearDisplay();
  }

  // 3. Button Released (Short Click)
  if (reading == HIGH && lastButtonState == LOW) {
    if (!ignoreNextRelease) {
      // Normal click: Cycle to next mode
      currentMode++;
      if (currentMode > 4) currentMode = 0; // Cycle back to idle
      u8x8.clearDisplay(); // Clear screen for new expression
    }
  }

  lastButtonState = reading;
}

// --- MODE 0: IDLE (Advanced) ---
void runIdleAnimation() {
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime > nextBlinkInterval) {
    lastBlinkTime = currentTime;
    
    if (!isBlinking) {
      // START BLINK
      isBlinking = true;
      nextBlinkInterval = 200; 

      // DECIDE: Blink or Wink?
      // Constraint: Can ONLY wink if looking straight (offset 0)
      int dice = random(0, 10);
      if (lookOffset == 0 && dice > 5) {
        if (dice < 8) blinkType = 1; // Wink Left
        else blinkType = 2;          // Wink Right
      } else {
        blinkType = 0; // Standard Blink
      }

      u8x8.clearDisplay(); 

    } else {
      // END BLINK (Open Eyes)
      isBlinking = false;
      nextBlinkInterval = random(1000, 4000); 
      
      // MOVE EYES: Only allowed when eyes open again!
      int roll = random(0, 3);
      if (roll == 0) lookOffset = 0;    // Center
      if (roll == 1) lookOffset = -2;   // Left
      if (roll == 2) lookOffset = 2;    // Right
      
      u8x8.clearDisplay();
    }
  }

  // DRAW IDLE EYES
  int leftX = 3 + lookOffset;
  int rightX = 9 + lookOffset;

  if (isBlinking) {
    // Left Eye
    if (blinkType == 0 || blinkType == 1) { 
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.drawString(leftX, 4, "____");
    } else {
      u8x8.setFont(u8x8_font_open_iconic_play_4x4);
      u8x8.drawGlyph(leftX, 2, 'K');
    }
    // Right Eye
    if (blinkType == 0 || blinkType == 2) {
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.drawString(rightX, 4, "____");
    } else {
      u8x8.setFont(u8x8_font_open_iconic_play_4x4);
      u8x8.drawGlyph(rightX, 2, 'K');
    }
  } 
  else {
    // Standard Open Eyes
    u8x8.setFont(u8x8_font_open_iconic_play_4x4);
    u8x8.drawGlyph(leftX, 2, 'K'); 
    u8x8.drawGlyph(rightX, 2, 'K');
  }
}

// --- MODE 1: HAPPY (Clouds) ---
void drawHappy() {
  // Cloud Icon 'A' in Weather font looks like a curved eye
  u8x8.setFont(u8x8_font_open_iconic_weather_4x4);
  u8x8.drawGlyph(3, 2, 'A'); 
  u8x8.drawGlyph(9, 2, 'A');
}

// --- MODE 2: ANGRY (Triangles) ---
void drawAngry() {
  u8x8.setFont(u8x8_font_open_iconic_play_4x4);
  // 'E' is Left Triangle (Rewind) -> Put on Right Eye to point IN
  // 'A' is Right Triangle (Play) -> Put on Left Eye to point IN
  u8x8.drawGlyph(3, 2, 'A'); 
  u8x8.drawGlyph(9, 2, 'E'); 
}

// --- MODE 3: TIRED (Low Lines) ---
void drawTired() {
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  // Drawing lines lower down to look heavy
  u8x8.drawString(3, 4, "____"); 
  u8x8.drawString(9, 4, "____");
  
  // Zzz...
  u8x8.drawString(12, 1, "z");
  u8x8.drawString(13, 0, "z");
}

// --- MODE 4: LOVE (Text Heart) ---
void drawLove() {
  // Since heart icons vary, we use giant text
  u8x8.setFont(u8x8_font_profont29_2x3_r);
  u8x8.drawString(1, 2, "< 3"); // Coordinates to center it
}
