#ifndef PS2_h
#define PS2_h

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "sdkconfig.h"

#define CLOCK (19)
#define DATA (18)

#define PS2_TAB				9
#define PS2_ENTER			13
#define PS2_LINEFEED			10
#define PS2_BACKSPACE			127
#define PS2_ESC				27
#define PS2_INSERT			0
#define PS2_DELETE			127
#define PS2_HOME			0
#define PS2_END				0
#define PS2_PAGEUP			25
#define PS2_PAGEDOWN			26
#define PS2_UPARROW			11
#define PS2_LEFTARROW			8
#define PS2_DOWNARROW			12
#define PS2_RIGHTARROW			21
#define PS2_F1				0
#define PS2_F2				0
#define PS2_F3				0
#define PS2_F4				0
#define PS2_F5				0
#define PS2_F6				0
#define PS2_F7				0
#define PS2_F8				0
#define PS2_F9				0
#define PS2_F10				0
#define PS2_F11				0
#define PS2_F12				0
#define PS2_SCROLL			0

void PS2Init();
bool kbd_available();
uint8_t Decode();
uint8_t read();

#endif
