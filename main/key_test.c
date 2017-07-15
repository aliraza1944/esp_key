#include "PS2.h"

void app_main(){

    ESP_LOGI("KBD", ">> Keyboard Test Start");
    PS2Init();

    uint8_t c;

  	while(1){

      if(kbd_available()){

       c = read();

       if (c == PS2_ENTER) {
         printf("\n");
       } else if (c == PS2_TAB) {
         printf("[Tab]");
       } else if (c == PS2_ESC) {
         printf("[ESC]");
       } else if (c == PS2_PAGEDOWN) {
         printf("[PgDn]");
       } else if (c == PS2_PAGEUP) {
         printf("[PgUp]");
       } else if (c == PS2_LEFTARROW) {
         printf("[Left]");
       } else if (c == PS2_RIGHTARROW) {
         printf("[Right]");
       } else if (c == PS2_UPARROW) {
         printf("[Up]");
       } else if (c == PS2_DOWNARROW) {
         printf("[Down]");
       } else if (c == PS2_DELETE) {
         printf("[Del]");
       } else {
         printf("%c", c);
       }
   }
  }
}
