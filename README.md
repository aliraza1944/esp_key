# ESP32 Keyboard Interface.

Library for interfacing a standard PS2 keyboard with ESP32 using esp-idf.
Keymaps need to be added for keyboards with different languages.

Default keymap = US (adapted from code by Paul Stoffregen <paul@pjrc.com>)

#Connections:

CLOCK = GPIO 19 (pulled up)
DATA = GPIO 18 (pulled up)

Interrupt called at NEGEDGE of Clock.
