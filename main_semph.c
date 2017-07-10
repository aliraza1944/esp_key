//Interrupt Handling using Semaphore.

#include <stdio.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <esp_log.h>

#include "sdkconfig.h"

SemaphoreHandle_t xSemaphore = NULL;

static char tag[] = "test_intr";
static volatile uint8_t buffer[45];
static volatile uint8_t head, tail;

#define CLOCK (19)
#define DATA (18)

// interrupt service routine, called when the clock signal changes
void IRAM_ATTR clock_isr_handler(void* arg) {
  // notify the button task
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

// task that will react to clock changes
void clock_task(void* arg) {

	// infinite loop
	for(;;) {
		// wait for the notification from the ISR
		if(xSemaphoreTake(xSemaphore,portMAX_DELAY)) {

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
}

void app_main() {

  xSemaphore = xSemaphoreCreateBinary();

	ESP_LOGI(tag, ">> test1_task");

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

  xTaskCreate(clock_task, "clock_task", 4096, NULL, 10, NULL);
	gpio_install_isr_service(0);
	gpio_isr_handler_add(CLOCK, clock_isr_handler, NULL	);

}
