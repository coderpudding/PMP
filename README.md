

# My PMP

> A prototype portable media player built around the ESP32-S3.

This is my first hardware project. Since I dont know pcb designing, I built this prototype using off-the-shelf modules to learn the complete hardware design process, later on I plan to integrate everything onto a single PCB and turn it into the compact clip-on media player I originally envisioned

---

## The Idea

I wanted a dedicated music player for running without the distractions of a smartphone

The original concept was a clip-on device that simply plays music and gets out of the way, This prototype focuses on validating the core hardware before moving to a custom PCB and a much smaller enclosure

---

## Features

- Shake to wake
- Touchscreen interface
- Built-in speaker
- Wi-Fi music transfer
- MicroSD card storage
- Deep sleep support

---

## Transferring Songs

The ESP32 hosts its own Wi-Fi access point.

Connect to the device from your phone or laptop, open a browser, and upload songs directly to the SD card. No dedicated app, card reader, or special software is required.

---

## Circuit Diagram

![Circuit Diagram](circuitdiagram.png)

The entire system is centered around the ESP32-S3. Audio is handled through an I2S amplifier, storage is provided by a MicroSD card, while the display, touch controller, RTC, and motion sensor communicate over SPI and I2C. Every GPIO was planned beforehand to avoid pin conflicts and simplify the firmware.

---

## Enclosure Design

### Overall Enclosure

![Overall Enclosure](Full enclosure.png)

The enclosure was designed around the hardware rather than forcing the hardware to fit inside an existing case. The shell was built first, followed by the screen opening, speaker grille, USB-C cutout, SD card slot, and magnet mounting holes.

### Rear Panel

![Back Mesh](back mesh.png)

The rear panel contains the speaker grille, allowing sound to exit while maintaining the strength of the enclosure. Every opening was positioned to line up with the internal hardware before any printing.

---

## Bill of Materials

The complete parts list, purchase links, and pricing can be found in [bom.csv](bom.csv).

---

## Future Plans

This prototype validates the overall hardware architecture. Future revisions will focus on:

- Designing a custom PCB
- Reducing the enclosure size
- Returning to the original clip-on form factor
- Better battery integration
- Reducing internal wiring

---
## built for [macondo](https://macondo.hackclub.com/)
---
