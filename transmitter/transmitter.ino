#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "BluetoothSerial.h"

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
// TRANSMITTER PINS
// =====================================================
#define BUTTON_PIN   27
#define HISTORY_PIN  25
#define BUZZER_PIN   4
#define GREEN_LED    32
#define RED_LED      13

// =====================================================
// OLED
// =====================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// BLUETOOTH
// =====================================================
BluetoothSerial SerialBT;

// =====================================================
// SETTINGS
// =====================================================
const unsigned long ACCEPT_TIMEOUT = 60000;

// =====================================================
// EMERGENCY VARIABLES
// =====================================================
bool waitingForAcceptance = false;

unsigned long emergencySentTime = 0;

// Current emergency message
String currentEmergencyMessage = "SOS";

// =====================================================
// HISTORY
// =====================================================
#define MAX_HISTORY 10

String historyMessage[MAX_HISTORY];
String historyStatus[MAX_HISTORY];

int historyCount = 0;

// Which history will be displayed next
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

  oled.drawStr(0, 15, line1.c_str());
  oled.drawStr(0, 32, line2.c_str());

  if (line3 != "") {
    oled.drawStr(0, 49, line3.c_str());
  }

  oled.sendBuffer();
}

// =====================================================
// ADD HISTORY
// =====================================================
// Newest history is always HISTORY 1
// =====================================================
void addHistory(String message, String status) {

  // Move old histories one position down
  for (int i = MAX_HISTORY - 1; i > 0; i--) {

    historyMessage[i] = historyMessage[i - 1];
    historyStatus[i] = historyStatus[i - 1];
  }

  // Newest history becomes HISTORY 1
  historyMessage[0] = message;
  historyStatus[0] = status;

  // Increase count until 10
  if (historyCount < MAX_HISTORY) {
    historyCount++;
  }

  Serial.println();
  Serial.println("========== HISTORY SAVED ==========");

  Serial.print("HISTORY 1 MESSAGE: ");
  Serial.println(message);

  Serial.print("STATUS: ");
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

  // ---------------------------------------------------
  // HISTORY TITLE
  // ---------------------------------------------------
  String title =
    "HISTORY " + String(index + 1);

  // ---------------------------------------------------
  // MESSAGE
  // ---------------------------------------------------
  String message = historyMessage[index];

  message.toUpperCase();

  // ---------------------------------------------------
  // STATUS
  // ---------------------------------------------------
  String status = historyStatus[index];

  status.toUpperCase();

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------
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

  // ---------------------------------------------------
  // SERIAL MONITOR
  // ---------------------------------------------------
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

      // ------------------------------------------------
      // NO HISTORY
      // ------------------------------------------------
      if (historyCount == 0) {

        showOLED(
          "HISTORY",
          "No history",
          "available"
        );

      }

      // ------------------------------------------------
      // SHOW HISTORY
      // ------------------------------------------------
      else {

        displayHistory(historyView);

        // Next press shows next history
        historyView++;

        // After last available history
        // go back to History 1
        if (historyView >= historyCount) {

          historyView = 0;
        }
      }

      // ------------------------------------------------
      // WAIT FOR BUTTON RELEASE
      // ------------------------------------------------
      while (digitalRead(HISTORY_PIN) == LOW) {

        delay(10);
      }
    }
  }
}

// =====================================================
// SEND EMERGENCY
// =====================================================
void sendEmergency(String message) {

  // Don't send another emergency while
  // waiting for receiver
  if (waitingForAcceptance) {

    Serial.println(
      "Already waiting for acceptance."
    );

    return;
  }

  // Save current emergency message
  currentEmergencyMessage = message;

  Serial.println();
  Serial.println("================================");
  Serial.println("EMERGENCY MESSAGE");
  Serial.println("================================");

  Serial.print("Message: ");
  Serial.println(message);

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------
  String displayMessage = message;

  displayMessage.toUpperCase();

  showOLED(
    "EMERGENCY",
    "SENDING...",
    displayMessage
  );

  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------
  digitalWrite(BUZZER_PIN, HIGH);

  delay(1000);

  digitalWrite(BUZZER_PIN, LOW);

  // ---------------------------------------------------
  // LoRa
  // ---------------------------------------------------
  LoRa.beginPacket();

  LoRa.print("EMERGENCY");

  LoRa.endPacket();

  Serial.println("EMERGENCY SENT");

  // ---------------------------------------------------
  // GREEN LED
  // ---------------------------------------------------
  digitalWrite(GREEN_LED, HIGH);

  showOLED(
    "STATUS:",
    "MESSAGE SENT",
    "WAITING..."
  );

  delay(5000);

  digitalWrite(GREEN_LED, LOW);

  // ---------------------------------------------------
  // START 60 SECOND TIMER
  // ---------------------------------------------------
  emergencySentTime = millis();

  waitingForAcceptance = true;

  Serial.println(
    "Waiting for receiver acceptance..."
  );

  Serial.println(
    "Timeout = 60 seconds"
  );
}

// =====================================================
// EMERGENCY ACCEPTED
// =====================================================
void emergencyAccepted() {

  waitingForAcceptance = false;

  Serial.println();
  Serial.println("================================");
  Serial.println("EMERGENCY ACCEPTED");
  Serial.println("================================");

  // ---------------------------------------------------
  // SAVE COMPLETE HISTORY
  // ---------------------------------------------------
  addHistory(
    currentEmergencyMessage,
    "ACCEPTED"
  );

  // Reset history viewing
  historyView = 0;

  // ---------------------------------------------------
  // GREEN LED
  // ---------------------------------------------------
  digitalWrite(RED_LED, LOW);

  digitalWrite(GREEN_LED, HIGH);

  // ---------------------------------------------------
  // NO BUZZER
  // ---------------------------------------------------
  digitalWrite(BUZZER_PIN, LOW);

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------
  showOLED(
    "STATUS:",
    "ACCEPTED",
    "Receiver responded"
  );

  // Green LED ON for 5 seconds
  delay(5000);

  digitalWrite(GREEN_LED, LOW);

  // ---------------------------------------------------
  // READY
  // ---------------------------------------------------
  showOLED(
    "SYSTEM READY",
    "Press Button",
    "or Bluetooth"
  );
}

// =====================================================
// NOT ACCEPTED
// =====================================================
void emergencyNotAccepted() {

  waitingForAcceptance = false;

  Serial.println();
  Serial.println("================================");
  Serial.println("NOT ACCEPTED");
  Serial.println("60 SECOND TIMEOUT");
  Serial.println("================================");

  // ---------------------------------------------------
  // SAVE COMPLETE HISTORY
  // ---------------------------------------------------
  addHistory(
    currentEmergencyMessage,
    "NOT ACCEPTED"
  );

  // Reset history viewing
  historyView = 0;

  // ---------------------------------------------------
  // RED LED
  // ---------------------------------------------------
  digitalWrite(GREEN_LED, LOW);

  digitalWrite(RED_LED, HIGH);

  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------
  showOLED(
    "STATUS:",
    "NOT ACCEPTED",
    "60 sec timeout"
  );

  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------
  Serial.println("Buzzer warning!");

  digitalWrite(BUZZER_PIN, HIGH);

  delay(3000);

  digitalWrite(BUZZER_PIN, LOW);

  // Red LED remains ON for 2 seconds
  delay(2000);

  digitalWrite(RED_LED, LOW);

  // ---------------------------------------------------
  // READY
  // ---------------------------------------------------
  showOLED(
    "SYSTEM READY",
    "Press Button",
    "or Bluetooth"
  );
}

// =====================================================
// CHECK LoRa ACCEPTANCE
// =====================================================
void checkLoRaAcceptance() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String receivedMessage = "";

    while (LoRa.available()) {

      receivedMessage +=
        (char)LoRa.read();
    }

    receivedMessage.trim();

    Serial.print(
      "LoRa Received: "
    );

    Serial.println(
      receivedMessage
    );

    // -------------------------------------------------
    // ACCEPTED
    // -------------------------------------------------
    if (receivedMessage == "ACCEPTED") {

      if (waitingForAcceptance) {

        emergencyAccepted();
      }
    }
  }
}

// =====================================================
// CHECK 60 SECOND TIMEOUT
// =====================================================
void checkTimeout() {

  if (waitingForAcceptance) {

    unsigned long elapsedTime =
      millis() - emergencySentTime;

    // -------------------------------------------------
    // DISPLAY REMAINING TIME
    // -------------------------------------------------
    static unsigned long lastDisplay = 0;

    if (millis() - lastDisplay >= 1000) {

      lastDisplay = millis();

      unsigned long remaining = 0;

      if (elapsedTime < ACCEPT_TIMEOUT) {

        remaining =
          (ACCEPT_TIMEOUT - elapsedTime) / 1000;
      }

      Serial.print("Waiting... ");

      Serial.print(remaining);

      Serial.println(" seconds");

      String timeText =
        "Time: " +
        String(remaining) +
        " sec";

      showOLED(
        "MESSAGE SENT",
        "WAITING ACCEPT",
        timeText
      );
    }

    // -------------------------------------------------
    // TIMEOUT
    // -------------------------------------------------
    if (elapsedTime >= ACCEPT_TIMEOUT) {

      emergencyNotAccepted();
    }
  }
}

// =====================================================
// CHECK EMERGENCY BUTTON
// =====================================================
void checkButton() {

  if (!waitingForAcceptance) {

    if (digitalRead(BUTTON_PIN) == LOW) {

      delay(50);

      if (digitalRead(BUTTON_PIN) == LOW) {

        // Physical button sends SOS
        sendEmergency("SOS");

        // Wait for button release
        while (
          digitalRead(BUTTON_PIN) == LOW
        ) {

          delay(10);
        }
      }
    }
  }
}

// =====================================================
// CHECK BLUETOOTH
// =====================================================
void checkBluetooth() {

  if (SerialBT.available()) {

    String message =
      SerialBT.readStringUntil('\n');

    message.trim();

    Serial.print(
      "Bluetooth message: "
    );

    Serial.println(message);

    // Convert to lowercase
    message.toLowerCase();

    // -------------------------------------------------
    // ACCEPTED EMERGENCY MESSAGES
    // -------------------------------------------------
    if (
      message == "help" ||
      message == "emergency" ||
      message == "i need help" ||
      message == "sos"
    ) {

      sendEmergency(message);
    }
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
    BUTTON_PIN,
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
  Wire.begin(21, 22);

  oled.begin();

  showOLED(
    "EMERGENCY DEVICE",
    "STARTING..."
  );

  // ===================================================
  // LoRa SPI
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
  // BLUETOOTH
  // ===================================================
  SerialBT.begin(
    "Emergency_Transmitter"
  );

  Serial.println(
    "Bluetooth Started"
  );

  Serial.println(
    "Bluetooth Name:"
  );

  Serial.println(
    "Emergency_Transmitter"
  );

  // ===================================================
  // READY
  // ===================================================
  showOLED(
    "SYSTEM READY",
    "Press Button",
    "or Bluetooth"
  );

  Serial.println();
  Serial.println(
    "SYSTEM READY"
  );
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // Emergency button
  checkButton();

  // History button
  checkHistoryButton();

  // Bluetooth
  checkBluetooth();

  // LoRa acceptance
  checkLoRaAcceptance();

  // 60-second timeout
  checkTimeout();

  delay(10);
}
