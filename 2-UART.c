
/*
The assignment has been done as a group of students: 
Christiane Huetter(me19m009), Daeun Kim(me19m005), Laura Kummer(me19m004)

what worked from own testing:
1. Button 1, 2 or 3 is pressed. Then the message for each button based on the assignment is transmitted and displayed in minicom.
2. If the character 1, 2 or 3  is sent from minicom, it is received by the microcontroller and the LED is lighting up by the predefined character. 
*/


#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


volatile char RxBuffer = 0;
unsigned char value;
unsigned char a;



// ------------------------------------------------------------
// INITIALIZATION
// ------------------------------------------------------------
int pressed = 0; // counter for the press button section
  
void init_uart (uint32_t baudrate){   // initialization 
  uint16_t regvalue = (F_CPU/8/baudrate)-1; // formula for baudrate 

  UBRR0H =  (uint8_t) (regvalue >> 8) ; // get high byte of regvalue
  UBRR0L =  (uint8_t) (regvalue & 0xff); // get low byte of regvalue 
  
  UCSR0B |= ( (1<<TXEN0) + (1<<RXEN0)); // enable receiver and transmitter
  UCSR0A |= (1<<U2X0); // enable double speed mode for UART: it clears errors and fasten baudrate

  UCSR0B |= (1<<RXCIE0);           //enable the receive complete interrupt 
  UCSR0B |= (1<<UDRIE0);           //empty interrupt enable
  
  DDRB |= (1<<PB0)+(1<<PB1)+(1<<PB2); // make PB0,PB1 and PB2 as OUTPUT pins for LEDs
  DDRD &= ~(1<<PD2)+ ~(1<<PD3) + ~(1<<PD4); // make PD2, PD3 and PD4 as inputs (buttons)
  
  PORTD |= (1<<PD2)+(1<<PD3)+(1<<PD4); // activate the pull up resistor for the three buttons

  }



// ------------------------------------------------------------
// INTERRUPT
// ------------------------------------------------------------
ISR(USART_RX_vect) // interrupt the service routine  
{ 
  RxBuffer=UDR0; //RxBuffer is a temporary memory buffer of RX (for receiving)
}


// ------------------------------------------------------------
// SENDING STRING
// ------------------------------------------------------------
void uart_sendstring(char *str) { // sending a string via a pointer for each index of the string
  
  while (*str){ //starting * str , pointing out 
    uart_transmit (*str); // transmitting 
    str++; //increment the index of the string by 1
    }
  } 

// ------------------------------------------------------------
// TRANSMITTING
// ------------------------------------------------------------
void uart_transmit(uint8_t value) { // the value is for input as the function is called
  while ((UCSR0A & (1<<5)) == 0); //wait until UDR0 is free to send
  UDR0 = value;        
  }

// ------------------------------------------------------------
// RECEIVING 
// ------------------------------------------------------------
unsigned char uart_receive(){ // receiving information 
  while ((UCSR0A & (1<<RXC0)) == 0); //wait until UDR0 received a character 
  return UDR0; 
   
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main()
{
  init_uart(115200); // baudrate 
  uart_sendstring("okay ready\n\n"); // sending "okay ready" 

  
// ------------------------------------------------------------
// PART 1 - BUTTON PRESSING 
// ------------------------------------------------------------
 while(1) 
  {


      if (!(PIND & (1<<PD2))) // button 1 is pressed 
      { 
        
        uart_sendstring("button 1 is pressed\n");
        _delay_ms(200);
  
      }
      else if (!(PIND & (1<<PD3))) // button 2 is pressed 
      {       
        uart_sendstring("button 2 is pressed\n"); 
        _delay_ms(200);
        
      }
      else if (!(PIND & (1<<PD4))) // button 3 is pressed 
      {       
        uart_sendstring("button 3 is pressed\n");
        _delay_ms(200);
      }
 
// ------------------------------------------------------------
// PART 2 - LED LIGHTING UP 
// ------------------------------------------------------------
       if (UDR0 == '1'){ // if "1" is typed through minicom
        PORTB |= (1<<PB0);// LED 1 is lighting up 
        _delay_ms(500);
        PORTB &= ~(1<<PB0);}
    
       else if ( UDR0 == '2'){// if "2" is typed through minicom
        PORTB |= (1<<PB1);// LED 2 is lighting up 
        _delay_ms(500);
        PORTB &= ~(1<<PB1); }
    
       else if ( UDR0 == '3'){// if "3" is typed through minicom
        PORTB |= (1<<PB2); // LED 3 is lighting up 
        _delay_ms(500);
        PORTB &= ~(1<<PB2); }
    

    UDR0 = 0; // return UDRO to initial state
  }
 
}