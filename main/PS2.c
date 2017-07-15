#include "PS2.h"

static volatile uint8_t buffer[45];
static volatile uint8_t head, tail;

static uint8_t CharBuffer=0;
static uint8_t UTF8next=0;

#define BREAK     0x01
#define MODIFIER  0x02
#define SHIFT_L   0x04
#define SHIFT_R   0x08
#define ALTGR     0x10
#define CTRL      0x20

#define PS2_KEYMAP_SIZE 136

char Keymap_US [2][PS2_KEYMAP_SIZE] = {
  // without shift
	{0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
	0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '`', 0,
	0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'q', '1', 0,
	0, 0, 'z', 's', 'a', 'w', '2', 0,
	0, 'c', 'x', 'd', 'e', '4', '3', 0,
	0, ' ', 'v', 'f', 't', 'r', '5', 0,
	0, 'n', 'b', 'h', 'g', 'y', '6', 0,
	0, 0, 'm', 'j', 'u', '7', '8', 0,
	0, ',', 'k', 'i', 'o', '0', '9', 0,
	0, '.', '/', 'l', ';', 'p', '-', 0,
	0, 0, '\'', 0, '[', '=', 0, 0,
	0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, ']', 0, '\\', 0, 0,
	0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
	PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
	0, 0, 0, PS2_F7 },
  // with shift
	{0, PS2_F9, 0, PS2_F5, PS2_F3, PS2_F1, PS2_F2, PS2_F12,
	0, PS2_F10, PS2_F8, PS2_F6, PS2_F4, PS2_TAB, '~', 0,
	0, 0 /*Lalt*/, 0 /*Lshift*/, 0, 0 /*Lctrl*/, 'Q', '!', 0,
	0, 0, 'Z', 'S', 'A', 'W', '@', 0,
	0, 'C', 'X', 'D', 'E', '$', '#', 0,
	0, ' ', 'V', 'F', 'T', 'R', '%', 0,
	0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
	0, 0, 'M', 'J', 'U', '&', '*', 0,
	0, '<', 'K', 'I', 'O', ')', '(', 0,
	0, '>', '?', 'L', ':', 'P', '_', 0,
	0, 0, '"', 0, '{', '+', 0, 0,
	0 /*CapsLock*/, 0 /*Rshift*/, PS2_ENTER /*Enter*/, '}', 0, '|', 0, 0,
	0, 0, 0, 0, 0, 0, PS2_BACKSPACE, 0,
	0, '1', 0, '4', '7', 0, 0, 0,
	'0', '.', '2', '5', '6', '8', PS2_ESC, 0 /*NumLock*/,
	PS2_F11, '+', '3', '-', '*', '9', PS2_SCROLL, 0,
	0, 0, 0, PS2_F7 }
};

static void IRAM_ATTR handler(){

	uint32_t now_ms = xTaskGetTickCount();
	static volatile uint8_t byteIn = 0;
	static volatile uint8_t bitcount = 0;
	uint8_t volatile val,n;
	val = gpio_get_level(DATA);
	static uint32_t prev_ms = 0;

	if(now_ms - prev_ms > 250){
		 bitcount = 0;
		 byteIn = 0;
	}

	prev_ms = now_ms;
	n = bitcount - 1;
	if (n <= 7) {
		byteIn |= (val << n);
	}

	bitcount++;
	if (bitcount == 11) {
		uint8_t i = head + 1;
		if (i >= 45) i = 0;
		if (i != tail) {
			buffer[i] = byteIn;
			head = i;
		}
		bitcount = 0;
		byteIn = 0;
	}

}

void PS2Init(){
	//Config CLOCK pin for NEGEDGE INTRS
	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = (1 << CLOCK);
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig.intr_type    = GPIO_INTR_NEGEDGE;
	gpio_config(&gpioConfig);

	//Config DATA pin in PULLUP mode
  gpio_config_t gpioConfig1;
	gpioConfig1.pin_bit_mask = (1 << DATA);
	gpioConfig1.mode         = GPIO_MODE_INPUT;
	gpioConfig1.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig1.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig1.intr_type    = GPIO_INTR_DISABLE;
	gpio_config(&gpioConfig1);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(CLOCK, handler, NULL	);
}

static inline uint8_t get_scan_code()
{
	uint8_t c, i;

	i = tail;
	if (i == head)
    return 0;
	i++;

  if (i >= 45)
    i = 0;
	c = buffer[i];
	tail = i;

  return c;
}

uint8_t Decode(){
	static uint8_t state=0;
	uint8_t s;
	char c;

	while (1) {
		s = get_scan_code();
		if (!s)
			return 0;
		if (s == 0xF0) {
			state |= BREAK;
			//printf("State F0: %d\n",state);
		} else if (s == 0xE0) {
			state |= MODIFIER;
			//printf("State E0 : %d\n",state);
		} else {
			if (state & BREAK) {
				if (s == 0x12) {
					state &= ~SHIFT_L;
					//printf("L_SHIFT   :        %d\n",state);
				} else if (s == 0x59) {
					state &= ~SHIFT_R;
					//printf("R_SHIFT   :        %d\n",state);
				} else if (s == 0x14) {
					state &= ~CTRL;
					//printf("CTRL      :        %d\n",state);
				} else if (s == 0x11 && (state & MODIFIER)) {
					state &= ~ALTGR;
					//printf("ALT       :        %d\n",state);
				}
				state &= ~(BREAK | MODIFIER);
				//printf("BREAK | MODIFIER  :  %d\n",state);
				continue;
			}
			if (s == 0x12) {
				state |= SHIFT_L;
				continue;
			} else if (s == 0x59) {
				state |= SHIFT_R;
				continue;
			} else if (s == 0x14) {
				state |= CTRL;
				continue;
			} else if (s == 0x11 && (state & MODIFIER)) {
				state |= ALTGR;
			}
			c = 0;
			//printf("state   :   %d\n",state);
			if (state & MODIFIER) {
				switch (s) {
				  case 0x70: c = PS2_INSERT;      break;
				  case 0x6C: c = PS2_HOME;        break;
				  case 0x7D: c = PS2_PAGEUP;      break;
				  case 0x71: c = PS2_DELETE;      break;
				  case 0x69: c = PS2_END;         break;
				  case 0x7A: c = PS2_PAGEDOWN;    break;
				  case 0x75: c = PS2_UPARROW;     break;
				  case 0x6B: c = PS2_LEFTARROW;   break;
				  case 0x72: c = PS2_DOWNARROW;   break;
				  case 0x74: c = PS2_RIGHTARROW;  break;
				  case 0x4A: c = '/';             break;
				  case 0x5A: c = PS2_ENTER;       break;
				  default: break;
				}
			}

			else if (state & (SHIFT_L | SHIFT_R)) {
				if (s < PS2_KEYMAP_SIZE)
					c = Keymap_US[1][s];
			} else {
				if (s < PS2_KEYMAP_SIZE)
					c = Keymap_US[0][s];
			}

			state &= ~(BREAK | MODIFIER);
			if (c)
				return c;
		}
	}
}

bool kbd_available() {
	if (CharBuffer || UTF8next)
		return true;
	CharBuffer = Decode();
	if (CharBuffer)
		return true;
	return false;
}

uint8_t read(){
	uint8_t result;

	result = UTF8next;
	if (result) {
		UTF8next = 0;
	}
	else {
		result = CharBuffer;
		if (result) {
			CharBuffer = 0;
		} else {
			result = Decode();
		}
		if (result >= 128) {
			UTF8next = (result & 0x3F) | 0x80;
			result = ((result >> 6) & 0x1F) | 0xC0;
		}
	}
	if (!result) return -1;
	return result;
}
