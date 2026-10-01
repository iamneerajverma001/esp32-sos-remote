/*
 * ======================================================================================
 * Project: ESP32 SOS Panic Remote (BLE GATT Server)
 * Emergency Triggers:
 *   1. Double-Click   -> MEDICAL EMERGENCY (Value: 0x01)
 *   2. Hold for 3 sec -> SAFETY / PANIC THREAT (Value: 0x02)
 *
 * Hardware Wiring:
 *   - Push Button: Between GPIO 4 and GND (uses internal pull-up)
 *   - Status LED : Onboard LED (usually GPIO 2)
 * ======================================================================================
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BUTTON_PIN      4     // Push button pin (to GND)
#define LED_PIN         2     // Onboard LED for physical feedback

// Custom BLE UUIDs
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Timing Configurations (milliseconds)
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long DOUBLE_CLICK_WINDOW = 400; // Max gap between clicks for double-click
const unsigned long LONG_PRESS_DURATION = 3000; // 3.0 seconds hold for safety alert

// State tracking
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long pressStartTime = 0;
unsigned long releaseTime = 0;
int clickCount = 0;
bool longPressTriggered = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Phone Connected!");
    // Quick double blink for connection
    digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW); delay(100);
    digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW);
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Phone Disconnected. Restarting advertising...");
    pServer->startAdvertising();
  }
};

void sendEmergencySignal(uint8_t alertType) {
  if (pCharacteristic == NULL) return;

  uint8_t payload[1] = { alertType };
  pCharacteristic->setValue(payload, 1);
  pCharacteristic->notify();

  if (alertType == 0x01) {
    Serial.println("[ALERT] >>> MEDICAL EMERGENCY TRIGGERED (0x01) <<<");
    // Visual Feedback: 2 short blinks
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_PIN, HIGH); delay(150);
      digitalWrite(LED_PIN, LOW); delay(150);
    }
  } else if (alertType == 0x02) {
    Serial.println("[ALERT] >>> SAFETY / THREAT ALARM TRIGGERED (0x02) <<<");
    // Visual Feedback: 1 long solid pulse (1.5 seconds)
    digitalWrite(LED_PIN, HIGH);
    delay(1500);
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n--- ESP32 SOS Remote Initializing ---");

  // Initialize BLE Device
  BLEDevice::init("ESP32-SOS-REMOTE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create GATT Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create GATT Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());

  // Initial dummy state (0x00 = idle)
  uint8_t initialValue[1] = { 0x00 };
  pCharacteristic->setValue(initialValue, 1);

  pService->start();

  // Configure Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Device ready. Advertising as 'ESP32-SOS-REMOTE'...");
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  // Debouncing logic
  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      // Button is PRESSED (Active LOW)
      if (buttonState == LOW) {
        pressStartTime = now;
        longPressTriggered = false;
      } 
      // Button is RELEASED
      else {
        unsigned long pressDuration = now - pressStartTime;
        if (!longPressTriggered) {
          if (pressDuration < 600) { // Valid short click
            clickCount++;
            releaseTime = now;
          }
        }
      }
    }
  }

  // 1. Check for Long Press (Safety Threat) while button is still held down
  if (buttonState == LOW && !longPressTriggered) {
    if ((now - pressStartTime) >= LONG_PRESS_DURATION) {
      longPressTriggered = true;
      clickCount = 0; // Reset any clicks
      sendEmergencySignal(0x02); // 0x02 = Safety Threat
    }
  }

  // 2. Check for Double Click (Medical) after click window closes
  if (clickCount > 0 && (now - releaseTime) > DOUBLE_CLICK_WINDOW) {
    if (clickCount >= 2) {
      sendEmergencySignal(0x01); // 0x01 = Medical Emergency
    } else {
      // Single click ignored to prevent accidental false alarms in pocket
      Serial.println("[INFO] Single click discarded (accidental press filter).");
    }
    clickCount = 0;
  }

  lastButtonState = reading;
}
