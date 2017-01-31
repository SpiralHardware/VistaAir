// from https://www.hackster.io/tyeth/rotary-quadrature-encoder-let-s-make-a-digital-safe-769ca4


#define ENC_A 0
#define ENC_B 4

// initially was going to hand pull the required bits from esp32 gpio via
// the 32bit functions gpio_input_get() and gpio_input_get_high() and test
// something like gpio_input_get() & (1<<11 | 1<< 21) == (1<<11 | 1<<21)
// but looking for the docs on the functions I found this line in gpio.h

//#define GPIO_INPUT_GET(gpio_no)     ((gpio_no < 32)? ((gpio_input_get()>>gpio_no)&BIT0) : ((gpio_input_get_high()>>(gpio_no - 32))&BIT0))

// this will involve two calls to the function which is not okay really,
// as the pins may have changed value between calls potentially. One should
// suffice, maybe at the expense of a second variable or additional shifts.
#include <rom/gpio.h>

 
void setup()
{
  /* Setup encoder pins as inputs */
  pinMode(ENC_A, INPUT_PULLUP); // OR INPUT_PULLUP
  pinMode(ENC_B, INPUT_PULLUP);
  Serial.begin (115200);
  Serial.println("Start");
}
 
void loop()
{
 static uint8_t counter = 0;      //this variable will be changed by encoder input
 int8_t tmpdata;
 /**/
  tmpdata = read_encoder();
  if( tmpdata ) {
    Serial.print("Counter value: ");
    Serial.println(counter, DEC);
    counter += tmpdata;
  }
}
 
/* returns change in encoder state (-1,0,1) */
int8_t read_encoder()
{
  
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0;
  static uint32_t curval = 0;
  /**/
  old_AB <<= 2;                   //remember previous state
  //bit shift old_AB two positions to the left and store.

  curval = gpio_input_get();  // returns gpio pin status of pins - SEE DEFINE 

  //note to self: these curval bits are probably backwards...
 
  old_AB |= ( ( (curval & 1<< ENC_A ) >> ENC_A  | (curval & 1<< ENC_B ) >> (ENC_B - 1) ) & 0x03 ); 
  //add current state and hopefully truncate to 8bit 
 
  return ( enc_states[( old_AB & 0x0f )]);
  // return the array item that matches the known possible encoder states

  // Thanks to kolban in the esp32 channel, who has a great book on everything iot,
  // for his initial help at my panic on the esp32 gpio access. long live IRC :)
}
