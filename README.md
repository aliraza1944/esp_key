# ESP32 Keyboard Interface.


Currently trying out different interrupt techniques using freeRTOS.

#Connections:

CLOCK = GPIO 19 (pulled up)
DATA = GPIO 18 (pulled up)

Interrupt called at NEGEDGE of Clock.

#Background

A key press on keyboard, initiates a series of signal changes on the clock
line (total 11). First is start bit, next 8 bits are data bits, then a
parity bit and then a stop bit.

At each NEGEDGE of the CLOCK, we have to read data bit and store it to a
uint8_t (byteIn) by shifting.

#Issue faced.
Whenever I press a key, the value returned by byteIn is usually 255, which
means that DATA line was pulled up when it was read. I think there is a problem
with the interrupt latency, it feels like that I am reading the DATA after the
appropriate. Which makes me read the value 255, meaning that DATA line was
pulled up when read.

#IMPORTANT

I have tried the same technique with ESP32 for reading DATA line in Arduino IDE
and it works absolutely fine. For interrupt I have used the attachInterrupt()
funtion.
