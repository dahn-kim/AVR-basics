/*
The assignment has been done as a group of students: 
Christiane Huetter(me19m009), Daeun Kim(me19m005), Laura Kummer(me19m004)

what worked from own testing:
1. when one of two buttons is pressed for the first time, 
   it starts blinking from the direction that a user ordered.
2. while LEDs are blinking, pressing a different button to switch the direction
   : the direction changes only after holding button long OR pressing the button TWICE. 

*/


#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

int fromLeft=0; //assigning left-origin direction to a variable
int fromRight=0; //assigning right-origin direction to a variable 
 
void checkButtons(); //a function that checks if one of buttons is pressed 
void left_dir(); // a fuction gives LED direction from left to right
void right_dir(); // a fuction gives LED direction from right to left

int main()
{

  DDRB |= (1<<2)+(1<<3)+(1<<4); // output pins
  PORTB |= (1<<0)+(1<<1); // activating pullup resistors
  PORTB |= 0x00; // make sure all leds are off. 

  while(1){

    checkButtons(); // run the check button function. 
    
    }

}


void checkButtons(){

  if ((PINB &(1<<0))==0){ // check if PB0 inPin is pressed 
    fromLeft= !fromLeft; 
    fromRight=0;
    _delay_ms(5);
    while ((PINB &(1<<0))==0){}; // wait until the button is released
    left_dir(); // start the left-right LED function
    }

  else if ((PINB &(1<<1))==0){ // check if PB1 inPin is pressed 
    fromRight= !fromRight;
    fromLeft=0;
    _delay_ms(5);               // wait until the button is released
    while ((PINB &(1<<1))==0){}; 
    right_dir();      // start the right-left LED function
    }
  
}

void left_dir(){

  if (fromLeft == true) { 
  
  while(1){

    PORTB |= (1<<2); //turn PB2 on 
    _delay_ms(500);
    PORTB &= ~(1<<2); //turn PB2 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
                 
    PORTB |= (1<<3); //turn PB3 on 
    _delay_ms(500);
    PORTB &= ~(1<<3); //turn PB3 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
                 
    PORTB |= (1<<4); //turn PB4 on 
    _delay_ms(500);
    PORTB &= ~(1<<4); //turn PB4 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
   }

  }

}


void right_dir(){
  
  if (fromRight == true) {
   while(1){

    PORTB |= (1<<4);  //turn PB4 on
    _delay_ms(500);
    PORTB &= ~(1<<4); //turn PB4 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
                 
    PORTB |= (1<<3); //turn PB3 on
    _delay_ms(500);
    PORTB &= ~(1<<3); //turn PB3 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
                 
    PORTB |= (1<<2); //turn PB2 on 
    _delay_ms(500);
    PORTB &= ~(1<<2);  //turn PB2 off
    _delay_ms(500);

    checkButtons(); // go back to check any button was pressed 
   }
  }


 
  
}
  