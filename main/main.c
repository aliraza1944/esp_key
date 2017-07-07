
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
//#include "PS2Keyboard.h"

//PS2 Connections
#define DATA 18           //serially recieve data
#define CLOCK 19          //Clock is used to generate interrupts
//End PS2 Connections

//ESP32 interrupt procedure, copied from gpio_example_main.c
#define ESP_INTR_FLAG_DEFAULT 0
static xQueueHandle gpio_evt_queue = NULL;
static void IRAM_ATTR gpio_isr_handler(void* arg);
static void clock_interrupt(void* arg);



//Function Prototypes
void PS2Init();
static inline uint8_t get_scan_code(void);


// Keyboard Driver Variables
//static volatile uint8_t bitcount,calls;
static volatile uint8_t buffer[45];
static volatile uint8_t head, tail;


void app_main()
{

  PS2Init();
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

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
//gpio_intr_disable(DATA);

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

                /*if(bitcount == 1){
                  printf("start bit \n" );
                }

                if(bitcount>2 && bitcount<11) {
            			         byteIn = (byteIn>>1);
            			            if( gpio_get_level(DATA) )
            				          byteIn |= 0x80;
                           printf("data : %d\n",gpio_get_level(DATA) );
                           bitcount--;
                }

                if(bitcount == 2){
                  printf("parity bit \n");
                  bitcount--;
                }

                if(bitcount == 1){
                  printf("Stop Bit \n");
                  bitcount--;
                }

                if(bitcount == 0) {
          			//Decode(byteIn);
                    printf("byte : %d \n",byteIn);
                    bitcount = 11;
          		  }*/
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
