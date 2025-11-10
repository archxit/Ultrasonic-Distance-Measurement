# Ultrasonic Distance Measurement System 🧭

**Microcontroller:** NXP LPC1768 (ARM Cortex-M3)  
**Peripherals:** HC-SR04 Ultrasonic Sensor, 16x2 LCD, LEDs (P0.4–P0.11), Buzzer (P0.22)  
**Authors:** Archit 

---

## 📘 Project Overview
This system measures distance using an ultrasonic sensor and displays it on a 16x2 LCD.  
Depending on the distance:
- LEDs and buzzer alert when an object is close (<10 cm)
- LEDs only when moderately close (<20 cm)

---

## ⚙️ Features
- Accurate distance measurement using timer capture.
- Real-time display on LCD (4-bit mode).
- LED and buzzer safety indicators.
- Fully interrupt-safe and modular C code for LPC1768.

---

## 🧩 Pin Connections

| Component | LPC1768 Pin | Function |
|------------|-------------|-----------|
| TRIG (HC-SR04) | P0.15 | Output trigger |
| ECHO (HC-SR04) | P0.16 | Input echo |
| LCD (D4–D7) | P0.23–P0.26 | Data lines |
| LCD (RS, EN) | P0.27, P0.28 | Control lines |
| LEDs | P0.4–P0.11 | Visual alert |
| Buzzer | P0.22 | Audio alert |

---

## 🖼️ Setup Images
<p align="center">
  <img src="images/working_demo.jpg" width="400"/>
  <br/>
  <em>Project prototype setup with LCD and sensor</em>
</p>

---

## 🧠 Working Principle
1. The LPC1768 sends a 10μs pulse to the ultrasonic sensor trigger pin.
2. The sensor emits sound waves and sets ECHO high until the wave returns.
3. Timer measures pulse width → converts to distance using:
