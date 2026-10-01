# ESP32 SOS Panic Remote - Proof of Concept (PoC)

An emergency alert demonstration system using an **ESP32**, a **single push button**, and an **Android phone** running a Web Bluetooth gateway.

---

## 1. Hardware Setup

### Wiring Diagram
| Component | ESP32 Pin | Notes |
| :--- | :--- | :--- |
| **Push Button (Leg 1)** | `GPIO 4` | Uses internal pull-up resistor |
| **Push Button (Leg 2)** | `GND` | Ground connection |
| **Feedback LED** | `GPIO 2` | Onboard blue LED on most ESP32 boards |

### Button Timing Logic
* **Double-Click** (<400ms gap) $\rightarrow$ **🚨 Medical Emergency** (Sends `0x01`, LED blinks 2x)
* **Hold for 3.0 seconds** $\rightarrow$ **⚠️ Safety / Threat Alarm** (Sends `0x02`, LED turns solid for 1.5s)
* **Single accidental click** $\rightarrow$ Automatically discarded (false positive filter)

---

## 2. Get Free CallMeBot WhatsApp API Key (30 Seconds)

1. Open WhatsApp on the phone that will receive the guardian alerts.
2. Send this exact message via WhatsApp to **`+34 644 20 97 51`** (or click this link on the phone: [https://wa.me/34644209751?text=I+allow+callmebot+to+send+me+messages](https://wa.me/34644209751?text=I+allow+callmebot+to+send+me+messages)):
   ```text
   I allow callmebot to send me messages
   ```
3. CallMeBot will instantly reply with:
   > *"CallMeBot API Activated! Your apikey is: 123456"*
4. Save your **Phone number** (including country code, e.g. `+919876543210`) and **API key**.

---

## 3. Flash the ESP32 Firmware

1. Open Arduino IDE.
2. Open [esp32_sos.ino](file:///c:/Users/Neeraj/Documents/antigravity/dazzling-mendeleev/esp32_sos/esp32_sos.ino).
3. Select your Board: **Tools > Board > ESP32 Dev Module** (or your specific ESP32 variant).
4. Select the COM Port.
5. Click **Upload**.
6. Open Serial Monitor at **115200 baud** to verify that it prints:
   `[BLE] Device ready. Advertising as 'ESP32-SOS-REMOTE'...`

---

## 4. Run the Web Dashboard on Android

> **Note on Web Bluetooth Security**: Google Chrome requires **HTTPS** (or `localhost`) to access Bluetooth and GPS.

### Option A: Free GitHub Pages (Easiest & Best for Demo)
1. Push this repository to GitHub.
2. Go to **Settings > Pages > Deploy from branch (main / web_app)**.
3. Open the resulting `https://<username>.github.io/...` URL in **Chrome on your Android phone**.

### Option B: Local Testing with Port Forwarding (Chrome DevTools)
1. Run a local server on your PC inside `web_app`:
   ```bash
   cd web_app
   python -m http.server 8000
   ```
2. Connect your Android phone to your PC with a USB cable (with USB Debugging ON).
3. On your PC, open Chrome and navigate to `chrome://inspect/#devices`.
4. Click **Port forwarding...** and map port `8000` to `localhost:8000`.
5. On your phone's Chrome browser, open `http://localhost:8000`.

---

## 5. Live Demonstration Sequence

1. Open the web app on your Android phone.
2. Enter the **Guardian Phone Number** and **CallMeBot API Key**, then tap **Save Settings**.
3. Tap **"🔗 Pair ESP32 Remote"** $\rightarrow$ Select **ESP32-SOS-REMOTE** from the Chrome pairing list.
4. **Demonstrate Medical Alert**:
   * Double-click the push button.
   * Phone screen flashes **BLUE (Medical Emergency)** and beeps.
   * Guardian's phone receives WhatsApp message with live Google Maps link.
5. **Demonstrate Safety Alert**:
   * Press and hold the push button for 3 seconds.
   * Phone screen flashes **RED (Safety Threat Alert)**.
   * Guardian's phone receives high-priority threat alert on WhatsApp.
