# ESP32C6 Wifi Sniffer

This project is intended to be paired with the [desktop receiver project](https://github.com/ctrousdale/wifi-sniffer-receiver) in order to receive and parse packets via Wireshark.

I created this project as an intro to ESP32 programming, and to explore my interest in creating cybersecurity tools.

## Architecture
This project takes advantage of FreeRTOS tasks to collect packets and transmit them over USB via UART. Packet ordering is therefore not guaranteed, and is influenced by a few variables (onboard WiFi module receive speed, UART queue length and processing, etc.). Instead, ensure that Wireshark (or whichever packet analysis software) respects the sent Timestamp.
Additionally, you can pipe results from the ESP32 into some buffer that can be processed later (deferred processing). I've specified Wireshark here because of its popularity and the ability to read live data from filesystem pipes.

### Language
I've moved the project over from C to C++. C has a certain elegance to it, but I vastly prefer the use of smart pointers and other modern C++ concepts to mitigate memory leaks and the like.

## How to use
This project can most easily be run in VScode with the ESP-IDF plugin. I'm exploring ways to continue development with neovim.
