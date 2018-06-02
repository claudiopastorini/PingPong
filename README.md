# PingPong

This repository contains all information regarding the code of [Seeduino LoRaWAN](https://www.seeedstudio.com/Seeeduino-LoRaWAN-p-2780.html) and [Adafruit Feather M0 Radio with LoRa Radio Module](https://www.adafruit.com/product/3178) for the PingPong hands-on example.

The example is a simple Ping Pong application with two different **LoRa** boards communicate each other. One board is the **Adafruit Feather M0 with LoRa Radio module** the other one is the **Seeduino LoRaWAN**. Both boards use the same Arduino sketch, the first using the **[RadioHead Driver](http://www.airspayce.com/mikem/arduino/RadioHead/)**, the other one instead use **AT commands over serial communication**.

## Instructions

### 1. Download the board definition
Using the Android IDE you have to add the `Additional Boards Manager URLs` for the Seeduino and Adafruit boards into the `Preferences` section: 

* https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
* https://raw.githubusercontent.com/Seeed-Studio/Seeed_Platform/master/package_seeeduino_boards_index.json

Now you can download the board definition via the `Boards Manager`:

* `Seeduino SAMD` for the Seeduino board
* `Adafruit SAMD` for the Adafruit board

### 2. Upload the code inside the boards
Upload the code to the devices (the same code works with both boards, it autodetects during compiling phase the right code to upload)

### 3. Testing board
You will see on the serial monitor of the boards `PING` and `PONG` messages sent via **LoRa**.

## Credits

Claudio Pastorini: [LinkedIn](https://www.linkedin.com/in/claudio-pastorini/) | [WebSite](https://claudiopastorini.github.io) | [GitHub](https://github.com/claudiopastorini)
