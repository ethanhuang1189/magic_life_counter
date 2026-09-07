#include <stdio.h>
#include "pico/stdlib.h"
#include <stdbool.h>
#define ENCODER_CLK 2
#define ENCODER_DT 3
#define ENCODER_SW 4

int main(void) {
  stdio_init_all();

  //initialize encoder_clk and encoder_dt (lets pico board read them)
  gpio_init(ENCODER_CLK);
  gpio_set_dir(ENCODER_CLK, GPIO_IN);

  gpio_init(ENCODER_DT);
  gpio_set_dir(ENCODER_DT, GPIO_IN);

  gpio_init(ENCODER_SW);
  gpio_set_dir(ENCODER_SW, GPIO_IN);
  gpio_pull_up(ENCODER_SW);

  //encoderPOS: the position of the encoder (goes up clockwise, down counter clockwise)
  int encoderPOS = 0;
  int lastCLK = gpio_get(ENCODER_CLK);

  bool encoderButtonIsPressed = false;
  bool encoderButtonIsOn = false;
  //first sets lastCLK which will always be 0.  In while loop, first grab CLK value and save it to current CLK
  //check if current is less than last clk, we're only checking when clk changes for the second time as everytime
  //the encoder rotates there are 4 possible positions and 2 of them clk changes
  //then if clk != dt add 1 (this only happens on clockwise turn)
  while (true) {
    
    encoderButtonIsOn = !gpio_get(ENCODER_SW);
    int currentCLK = gpio_get(ENCODER_CLK);

    if (!encoderButtonIsPressed && encoderButtonIsOn) {
      printf("button pressed!\n");
      sleep_ms(20);
      encoderButtonIsPressed = true;
    } else if (encoderButtonIsPressed && !encoderButtonIsOn) {
      sleep_ms(20);
      encoderButtonIsPressed = false;
    }

    

    if (currentCLK < lastCLK) {
      if (currentCLK != gpio_get(ENCODER_DT)) {
        encoderPOS++;
        //printf("test add 1: %d\n", encoderPOS);
      } else {
        encoderPOS--;
      }

      printf("health: %d\n", encoderPOS);

    }
    lastCLK = currentCLK;
    sleep_ms(1);
  }
}