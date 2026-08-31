#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>

// =====================================================
// LoRa PINS
// =====================================================
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

// =====================================================
// RECEIVER PINS
// =====================================================
#define ACCEPT_BUTTON_PIN 27
#define HISTORY_PIN       25

#define BUZZER_PIN        4
#define GREEN_LED         32
#define RED_LED           13

// =====================================================
// OLED
// =====================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// SETTINGS
// =====================================================
const unsigned long ACCEPT_TIMEOUT = 60000;

const unsigned long WARNING_45 = 45000;
const unsigned long WARNING_54 = 54000;

// =====================================================
// EMERGENCY VARIABLES
// =====================================================
bool emergencyActive = false;

unsigned long emergencyStartTime = 0;

bool warning45Done = false;
bool warning54Done = false;

// =====================================================
// HISTORY
// =====================================================
#define MAX_HISTORY 10

String historyMessage[MAX_HISTORY];
String historyStatus[MAX_HISTORY];

int historyCount = 0;

// History currently selected
int historyView = 0;

// =====================================================
// OLED FUNCTION
// =====================================================
void showOLED(
  String line1,
  String line2,
  String line3 = ""
) {

  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);

  oled.drawStr(
    0,
    15,
    line1.c_str()
  );

  oled.drawStr(
    0,
    32,
    line2.c_str()
  );

  if (line3 != "") {

    oled.drawStr(
      0,
      49,
      line3.c_str()
    );
  }

  oled.sendBuffer();
}

// =====================================================
// ADD HISTORY
// =====================================================
void addHistory(
  String message,
  String status
) {

  // Move old history down
  for (
    int i = MAX_HISTORY - 1;
    i > 0;
    i--
  ) {

    historyMessage[i] =
      historyMessage[i - 1];

    historyStatus[i] =
      historyStatus[i - 1];
  }

  // Newest = History 1
  historyMessage[0] = message;
  historyStatus[0] = status;

  if (historyCount < MAX_HISTORY) {
    historyCount++;
  }

  Serial.println();
  Serial.println("========== HISTORY SAVED ==========");

  Serial.print("Message: ");
  Serial.println(message);

  Serial.print("Status: ");
  Serial.println(status);

  Serial.println("===================================");
}

// =====================================================
// DISPLAY HISTORY
// =====================================================
void displayHistory(int index) {

  if (historyCount == 0) {

    showOLED(
      "HISTORY",
      "No history",
      "available"
    );

    return;
  }

  if (index >= historyCount) {
    index = 0;
  }

  String title =
    "HISTORY " + String(index + 1);

  String message =
    historyMessage[index];

  String status =
    historyStatus[index];

  message.toUpperCase();
  status.toUpperCase();

  // OLED
  oled.clearBuffer();

  oled.setFont(u8g2_font_6x10_tf);

  oled.drawStr(
    0,
    15,
    title.c_str()
  );

  oled.drawStr(
    0,
    34,
    message.c_str()
  );

  oled.drawStr(
    0,
    53,
    status.c_str()
  );

  oled.sendBuffer();

  // Serial
  Serial.println();
  Serial.println("==============================");

  Serial.println(title);
  Serial.println(message);
  Serial.println(status);

  Serial.println("==============================");
}

// =====================================================
// HISTORY BUTTON
// =====================================================
void checkHistoryButton() {

  if (digitalRead(HISTORY_PIN) == LOW) {

    delay(50);

    if (digitalRead(HISTORY_PIN) == LOW) {

      Serial.println("History button pressed");

      if (historyCount == 0) {

        showOLED(
          "HISTORY",
          "No history",
          "available"
        );

      } else {

        displayHistory(historyView);

        // Next press = next history
        historyView++;

        if (historyView >= historyCount) {
          historyView = 0;
        }
      }

      // Wait for button release
      while (digitalRead(HISTORY_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

// =====================================================
// START EMERGENCY
// =====================================================
void startEmergency() {

  if (emergencyActive) {

    Serial.println(
      "Emergency already active."
    );

    return;
  }

  emergencyActive = true;

  emergencyStartTime = millis();

  warning45Done = false;
  warning54Done = false;

  // LEDs OFF initially
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  showOLED(
    "EMERGENCY!",
    "MESSAGE RECEIVED",
    "60 sec to accept"
  );

  Serial.println();
  Serial.println("================================");
  Serial.println("EMERGENCY RECEIVED");
  Serial.println("================================");
}

// =====================================================
// 45 SECOND WARNING
// =====================================================
void warningAt45Seconds() {

  Serial.println(
    "45 SECONDS - WARNING"
  );

  // Red LED ON
  digitalWrite(
    RED_LED,
    HIGH
  );

  showOLED(
    "WARNING!",
    "45 SECONDS",
    "PLEASE ACCEPT"
  );

  // Buzzer ON for 2 seconds
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(2000);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // Red LED remains ON
}

// =====================================================
// 54 SECOND WARNING
// =====================================================
void warningAt54Seconds() {

  Serial.println(
    "54 SECONDS - WARNING"
  );

  // Red LED remains ON
  digitalWrite(
    RED_LED,
    HIGH
  );

  showOLED(
    "WARNING!",
    "54 SECONDS",
    "ACCEPT NOW!"
  );

  // Buzzer ON for 2 seconds
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(2000);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // Red LED remains ON
}

// =====================================================
// ACCEPT EMERGENCY
// =====================================================
void acceptEmergency() {

  if (!emergencyActive) {

    Serial.println(
      "No active emergency."
    );

    return;
  }

  // Stop emergency timer
  emergencyActive = false;

  // Save history
  addHistory(
    "EMERGENCY",
    "ACCEPTED"
  );

  historyView = 0;

  // Send ACCEPTED to transmitter
  LoRa.beginPacket();

  LoRa.print("ACCEPTED");

  LoRa.endPacket();

  Serial.println(
    "ACCEPTED sent to transmitter"
  );

  // ---------------------------------------------------
  // LEDs
  // ---------------------------------------------------
  digitalWrite(
    RED_LED,
    LOW
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  digitalWrite(
    GREEN_LED,
    HIGH
  );

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------
  showOLED(
    "STATUS:",
    "ACCEPTED",
    "HELP IS COMING"
  );

  // Green LED ON for 5 seconds
  delay(5000);

  digitalWrite(
    GREEN_LED,
    LOW
  );

  // Ready
  showOLED(
    "SYSTEM READY",
    "Waiting...",
    "for emergency"
  );
}

// =====================================================
// NOT ACCEPTED
// =====================================================
void emergencyNotAccepted() {

  emergencyActive = false;

  Serial.println();
  Serial.println("================================");
  Serial.println("EMERGENCY NOT ACCEPTED");
  Serial.println("60 SECOND TIMEOUT");
  Serial.println("================================");

  // Save history
  addHistory(
    "EMERGENCY",
    "NOT ACCEPTED"
  );

  historyView = 0;

  // Red LED ON
  digitalWrite(
    GREEN_LED,
    LOW
  );

  digitalWrite(
    RED_LED,
    HIGH
  );

  // OLED
  showOLED(
    "TIMEOUT!",
    "NOT ACCEPTED",
    "60 SECONDS"
  );

  // Final buzzer
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(3000);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // Red LED stays for 2 more seconds
  delay(2000);

  digitalWrite(
    RED_LED,
    LOW
  );

  // Ready
  showOLED(
    "SYSTEM READY",
    "Waiting...",
    "for emergency"
  );
}

// =====================================================
// ACCEPT BUTTON
// =====================================================
void checkAcceptButton() {

  if (
    digitalRead(
      ACCEPT_BUTTON_PIN
    ) == LOW
  ) {

    delay(50);

    if (
      digitalRead(
        ACCEPT_BUTTON_PIN
      ) == LOW
    ) {

      if (emergencyActive) {

        acceptEmergency();
      }

      // Wait for release
      while (
        digitalRead(
          ACCEPT_BUTTON_PIN
        ) == LOW
      ) {

        delay(10);
      }
    }
  }
}

// =====================================================
// EMERGENCY TIMER
// =====================================================
void checkEmergencyTimer() {

  if (!emergencyActive) {
    return;
  }

  unsigned long elapsed =
    millis() - emergencyStartTime;

  // ===================================================
  // 45 SECONDS
  // ===================================================
  if (
    elapsed >= WARNING_45 &&
    !warning45Done
  ) {

    warning45Done = true;

    warningAt45Seconds();
  }

  // ===================================================
  // 54 SECONDS
  // ===================================================
  if (
    elapsed >= WARNING_54 &&
    !warning54Done
  ) {

    warning54Done = true;

    warningAt54Seconds();
  }

  // ===================================================
  // COUNTDOWN
  // ===================================================
  if (elapsed < ACCEPT_TIMEOUT) {

    unsigned long remaining =
      (ACCEPT_TIMEOUT - elapsed) / 1000;

    String timeText =
      "Time: " +
      String(remaining) +
      " sec";

    showOLED(
      "EMERGENCY!",
      "ACCEPT BUTTON",
      timeText
    );
  }

  // ===================================================
  // 60 SECOND TIMEOUT
  // ===================================================
  if (elapsed >= ACCEPT_TIMEOUT) {

    emergencyNotAccepted();
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // ===================================================
  // PINS
  // ===================================================
  pinMode(
    ACCEPT_BUTTON_PIN,
    INPUT_PULLUP
  );

  pinMode(
    HISTORY_PIN,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  pinMode(
    GREEN_LED,
    OUTPUT
  );

  pinMode(
    RED_LED,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  digitalWrite(
    GREEN_LED,
    LOW
  );

  digitalWrite(
    RED_LED,
    LOW
  );

  // ===================================================
  // OLED
  // ===================================================
  Wire.begin(
    21,
    22
  );

  oled.begin();

  showOLED(
    "EMERGENCY DEVICE",
    "RECEIVER",
    "STARTING..."
  );

  // ===================================================
  // LoRa
  // ===================================================
  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_SS
  );

  LoRa.setPins(
    LORA_SS,
    LORA_RST,
    LORA_DIO0
  );

  if (!LoRa.begin(433E6)) {

    Serial.println(
      "LoRa initialization FAILED!"
    );

    showOLED(
      "LoRa ERROR!",
      "Check wiring"
    );

    while (1) {

      digitalWrite(
        RED_LED,
        HIGH
      );

      delay(500);

      digitalWrite(
        RED_LED,
        LOW
      );

      delay(500);
    }
  }

  Serial.println(
    "LoRa Started Successfully"
  );

  // ===================================================
  // READY
  // ===================================================
  showOLED(
    "SYSTEM READY",
    "Waiting...",
    "for emergency"
  );

  Serial.println();
  Serial.println(
    "RECEIVER SYSTEM READY"
  );
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ---------------------------------------------------
  // Check LoRa
  // ---------------------------------------------------
  int packetSize =
    LoRa.parsePacket();

  if (packetSize) {

    String message = "";

    while (LoRa.available()) {

      message +=
        (char)LoRa.read();
    }

    message.trim();

    Serial.print(
      "LoRa Received: "
    );

    Serial.println(
      message
    );

    // Emergency received
    if (
      message == "EMERGENCY"
    ) {

      startEmergency();
    }
  }

  // ---------------------------------------------------
  // Accept button
  // ---------------------------------------------------
  checkAcceptButton();

  // ---------------------------------------------------
  // History button
  // ---------------------------------------------------
  checkHistoryButton();

  // ---------------------------------------------------
  // Timer
  // ---------------------------------------------------
  checkEmergencyTimer();

  delay(10);
}
