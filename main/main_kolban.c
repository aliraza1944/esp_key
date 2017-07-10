#if 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "sdkconfig.h";
//#include "PS2Keyboard.h"

//PS2 Connections
#define DATA 18           //serially recieve data
#define CLOCK 19          //Clock is used to generate interrupts
//End PS2 Connections

static char tag[] = "clock_intr";
static QueueHandle_t q1;

//ESP32 interrupt procedure, copied from gpio_example_main.c
#define ESP_INTR_FLAG_DEFAULT 0


static void handler(void *args) {
	gpio_num_t gpio;
	gpio = CLOCK;
	xQueueSendToBackFromISR(q1, &gpio, NULL);
}


//Function Prototypes
void PS2Init();
static inline uint8_t get_scan_code(void);


// Keyboard Driver Variables
//static volatile uint8_t bitcount,calls;
static volatile uint8_t buffer[45];
static volatile uint8_t head, tail;


void app_main()
{
  ESP_LOGD(tag, ">> test1_task");
	gpio_num_t gpio;
	q1 = xQueueCreate(10, sizeof(gpio_num_t));
  PS2Init();

  //start gpio task
  xTaskCreate(clock_interrupt, "clock_interrupt", 2048, NULL, 10, NULL);

  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(CLOCK, gpio_isr_handler, (void*) CLOCK);

  while(1) {
      //printf("%u\n", (unsigned) tab_dat1);
      //vTaskDelay(1000 / portTICK_RATE_MS);
      //uint8_t code = get_scan_code();
      //printf("%d\n",code );
  }
}

void PS2Init()
{
// Set GPIO interrupts
//gpio_set_intr_type(gpio_num_t gpio_num, gpio_int_type_t intr_type)
  gpio_set_direction(DATA, GPIO_MODE_INPUT);
  gpio_set_direction(CLOCK, GPIO_MODE_INPUT);
  gpio_set_intr_type(CLOCK, GPIO_INTR_NEGEDGE);
  gpio_set_pull_mode(CLOCK, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(DATA, GPIO_PULLUP_ONLY);
  gpio_intr_enable(CLOCK);
}



static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void clock_interrupt(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            //printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
                static uint8_t byteIn = 0, bitcount = 0;
                uint8_t val,n;
                val = gpio_get_level(DATA);
                static uint32_t prev_ms = 0;

                uint32_t now_ms = xTaskGetTickCount();

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
                  printf("Code: %d\n",byteIn);
              		if (i >= 45) i = 0;
              		if (i != tail) {
              			buffer[i] = byteIn;
              			head = i;
              		}
              		bitcount = 0;
              		byteIn = 0;
              	}
       }
   }
}

static inline uint8_t get_scan_code(void)
{
	uint8_t c, i;

	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= 45) i = 0;
	c = buffer[i];
	tail = i;
	return c;
}
#endif
/*
 * Test interrupt handling on a GPIO.
 * In this fragment we watch for a change on the input signal
 * of GPIO 25.  When it goes high, an interrupt is raised which
 * adds a message to a queue which causes a task that is blocking
 * on the queue to wake up and process the interrupt.
 */
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "sdkconfig.h"

static char tag[] = "test_intr";
static QueueHandle_t q1;
static volatile uint8_t buffer[45];
static volatile uint8_t head, tail;

#define CLOCK (19)
#define DATA (18)

static void IRAM_ATTR handler(void *args) {
	gpio_num_t gpio;
	gpio = CLOCK;
  xQueueSendToBackFromISR(q1, &gpio, NULL);
}

static void clock_interrupt(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(q1, &io_num, portMAX_DELAY)) {
								uint32_t now_ms = xTaskGetTickCount();
                static uint8_t byteIn = 0;
								static uint8_t bitcount = 0;
                uint8_t val,n;
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
                  ESP_LOGI("CLK_INTR","Code: %d",byteIn);
              		if (i >= 45) i = 0;
              		if (i != tail) {
              			buffer[i] = byteIn;
              			head = i;
              		}
              		bitcount = 0;
              		byteIn = 0;
              	}
       }
   }
   vTaskDelete(NULL);
}

static inline uint8_t get_scan_code(void)
{
	uint8_t c, i;

	i = tail;
	if (i == head) return 0;
	i++;
	if (i >= 45) i = 0;
	c = buffer[i];
	tail = i;
	return c;
}

void app_main() {
	ESP_LOGI(tag, ">> test1_task");
	gpio_num_t gpio;
	q1 = xQueueCreate(40, 10 * sizeof(gpio_num_t));
	if (q1 == 0)
		printf("failed to create queue.\n");

	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = GPIO_SEL_19;
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig.intr_type    = GPIO_INTR_NEGEDGE;
	gpio_config(&gpioConfig);

  gpio_config_t gpioConfig1;
	gpioConfig1.pin_bit_mask = GPIO_SEL_18;
	gpioConfig1.mode         = GPIO_MODE_INPUT;
	gpioConfig1.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig1.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig1.intr_type    = GPIO_INTR_DISABLE;
	gpio_config(&gpioConfig1);

	xTaskCreate(clock_interrupt, "clock_interrupt", 4096, NULL, 100, NULL);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(CLOCK, handler, NULL	);

}
