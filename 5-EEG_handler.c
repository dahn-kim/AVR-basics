/*
MAM Assignment 05 - P2 Packet Stream
The assignment has been done as a group of students: 
Mohamed Ali Ataie(me19m016), Christiane Huetter(me19m009), Daeun Kim(me19m005), Laura Kummer(me19m004)
*/

#define F_CPU 16000000 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>

                         
#define HEADERLEN 4
#define CHANNEL 6
#define PACKETLEN (HEADERLEN + CHANNEL * 2 + 1)   //packet length definition


uint8_t button_val = 0;               // declaring button value 
uint8_t Buffer[PACKETLEN];  // declaring the buffer for package >> changed to uint8
uint8_t packet_counter = 0;            //declaring the packet counter >>changed to 0
uint8_t send_packet= 0;
uint8_t counter = 0;


// ------------------------------------------------------------------------
// INITIALIATION OF UART 
// ------------------------------------------------------------------------
void init_uart (uint32_t baudrate) 
{
    unsigned int ubrr = (F_CPU/8/baudrate)-1; 

    UBRR0H = (uint8_t)(ubrr>>8);      // Set baud rate by filling high and
    UBRR0L = (uint8_t)(ubrr&0xff);    // low byte of the Baud Rate Register
    UCSR0A |= (1<<U2X0);                    // double speed for UART
    UCSR0B  = (1<<RXEN0 ) ;// (1<<TXEN0); // >> changed, not used in code! |  (1<<RXCIE0);  // Enable receiver and transmitter and UART receive interrupts
}


// ------------------------------------------------------------------------
// INITIALIATION OF ADC 
// ------------------------------------------------------------------------
void adc_init(void) {
  ADMUX = (1<<REFS0);                             //Avcc(+5v) as voltage reference
  ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);    // Prescaler
  ADCSRA |= (1<<ADEN) | (1<<ADIE);                  // activate  ADC And ADC interrupt enable
}


// ------------------------------------------------------------------------
// BUTTON + ADC 
// ------------------------------------------------------------------------
uint8_t button_adc()                      //waiting for the button to be pressed
{
  uint8_t converting = 0; 
  
  if ((PIND & (1<<PD2)) == 0)     //if pressed 
  {
    converting =! converting;
    _delay_ms(5);                   // bouncing of button
    while((PIND & (1<<PD2)) == 0);  //nothing happen during pressed
    
    if(converting)
    {
      UCSR0B ^= (1<<TXEN0); // toggle transmission  
      button_val = 1;
    }
  }
  Buffer[16] = button_val;      //storing the button status to packet
  return button_val;
}


// ------------------------------------------------------------------------
// ADC INTERRUPT
//------------------------------------------------------------------------
ISR(ADC_vect)     
{
	Buffer[(counter*2)+5]= ADCL;   //storing the lower byte of the particular adc-channel
	Buffer[(counter*2)+4]= ADCH;	//storing the higher byte of the particular adc-channel
    

	counter++;                     //increasing the counter for changing the adc-channel
    if (counter==6)					//if all 6 channels is read, set the counter to 0
	{
	    counter=0;
	    send_packet= 1;                   //if all channels read and stored in packet, by setting variable send to 1 its possible in main to transmit the packet 
	}
	
	else if(counter < 6)			//changing to next channel to read
	{
	    ADMUX = (1<<REFS0)+ counter;  // choosing the next channel and 5v as reference
		  ADCSRA |= (1<<ADSC);           // single adc-conversion for the particular channel
	}

}	
	

// ------------------------------------------------------------------------
// TIMER INTERRUPT  
// ------------------------------------------------------------------------
ISR(TIMER0_OVF_vect){
  
  cli();                    //clear Interrupt
  TCNT0 = 11;               // setting the reload value
  packet_counter ++;

  if (packet_counter > 255)  // reseting of packet counter to avoid overflow
  {
    packet_counter = 0; 
  }
   
   Buffer[3] = packet_counter; //storing  the packet counter in the packet
		
		ADMUX = (1<<REFS0) ;      //  choosing  the internal ref 5V and channel 0
	  ADCSRA |= (1<<ADIF);     //  Reset any pending ADC interrupts
		ADCSRA |= (1<<ADSC);     //  turn on the single adc conversion
	 
   sei();                          // set Interrupt	
}


// ------------------------------------------------------------------------
// SENDING THE READ IN VALUES
// ------------------------------------------------------------------------
void uart_transmit(uint8_t c)               // helper function to put out a character on the UART
{                                    
   while ( !( UCSR0A & (1<<UDRE0)) )        // wait until the Data Register Empty bit is set (polling to send)
   { }
   UDR0 = c;                               // send out the integer
}


// ------------------------------------------------------------------------
// CONVERT INTEGER TO STRING  
// ------------------------------------------------------------------------
 void int_to_str (uint16_t value, char * target)
 {
   int8_t i;                // int does mean = signed = to not have an endless loop 
   for(i=3;i>=0;i--)
   {
     target[i] = (value % 10) + '0';
     value /= 10; 
   }
   target[4] = '\0'; //to set the last element of string 
 }


// ------------------------------------------------------------------------
// SEND STRING TO UART  
// ------------------------------------------------------------------------
void uart_sendstring (char * str)       
{
   while (*str) { uart_transmit(*str); str++; }   // walk through zero-terminated string
}



int main(){

  init_uart(115200);
  adc_init();         //initialising of adc
  PORTD |= (1<<PD2); // button pull-up resistor 

  // Defining the P2 PACKET 
  Buffer[0] = 0xA5; //declaring sync0-value
  Buffer[1] = 0x5A; //declaring sync1-value
  Buffer[2] = 2; //declaring version info
  // The Interrupt contains Buffer[3] = packet_counter;
  // Buffer[3 - 15] : Channel values are collected within the ISR(ADV_vect) 
  // Buffer[10] = button_val  
		
  sei();
  TIMSK0 |= (1<<TOIE0); // setting timer overflow interrupt enable at Timer 0
  TCCR0B |= (1<<CS02); // based on Timer 1 and prescaler (Division factor 8), active the specific bitpositions
  
  char mystring[4]; 
  
  while(1){
	  
    button_adc();    //waiting to press button

	if(send_packet==1){  //if all adc-channels are read, send the packet

		for(int i=0; i<=16; i++){	// checking the ADC value of each  channel
			int_to_str(Buffer[i], mystring);
			uart_sendstring(mystring); 
			uart_transmit('\n');
			uart_transmit('\r');
		}
		
		send_packet = 0;	// this stops sending until the next packet is fully read from adc
	}
  }
}
