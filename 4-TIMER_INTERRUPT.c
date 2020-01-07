/*
MAM Assignment 04 - Piezo frequencies 100 - 8000 Hz 
The assignment has been done as a group of students: 
Christiane Huetter(me19m009), Daeun Kim(me19m005), Laura Kummer(me19m004) 
And huge thanks to our colleague Ingo Weigel(me19m022) for providing guidelines and tutoring.

The task of this assignment is to change the tone of piezo buzzer, given by the input number typed from a user. (UART) 
This assignment included the usage of USART RX interrupt, ISR TIMER OVF intrrupt, configuration of presecaler and reload value and
Str/char to integer conversion. 
We chose atoi function for the char-int conversion, which might lead to offset of frequency.
*/

#define F_CPU 16000000 
                 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>


volatile char frequency[5]; 
volatile uint8_t str_position = 0; 
volatile bool transmission_completed = false;



// ------------------------------------------------------------------------
// INITIALIATION OF UART 
// ------------------------------------------------------------------------
void init_uart (uint32_t baudrate) 
{
    unsigned int ubrr = (F_CPU/8/baudrate)-1; 

    UBRR0H = (uint8_t)(ubrr>>8);      // Set baud rate by filling high and
    UBRR0L = (uint8_t)(ubrr&0xff);    // low byte of the Baud Rate Register
    UCSR0A |= (1<<U2X0);                    // double speed for UART
    UCSR0B  = (1<<TXEN0 ) |  (1<<RXEN0 )  |  (1<<RXCIE0);  // Enable receiver and transmitter and UART receive interrupts
}


// ------------------------------------------------------------------------
// RECEIVE VALUE 
// ------------------------------------------------------------------------
ISR(USART_RX_vect)
{ 
    cli(); // clear Interrupt 
    
    transmission_completed = false; // set boolean status for if statement execution 
    char received = UDR0; // receive value from UDR0
    
    if (str_position < 4 && received!='\n' && received!='\r' && received!='\t') // store each digit of frequency input, until is reaches a set value  
    { 
      frequency[str_position] = received;  // assign frequency to the received variable  
      str_position++; // increment 
    }
    else if(str_position == 4) // if end is reached stop transmitting / receiving 
    {
      frequency[str_position] = '\0';
       str_position = 0;
       transmission_completed = true;
      }
   
    else
    {
       frequency[str_position] = '\0';
       str_position = 0;
       transmission_completed = true;
      }
      
    sei(); // set Interrupt 
} 


// ------------------------------------------------------------------------
// RELOAD VALUE FROM FREQUENCY 
// ------------------------------------------------------------------------
uint16_t reloadvalue_from_freq(uint16_t freq)
{
  
  float tick = 0.0000005; // ticktime based on chosen prescaler 
  uint16_t input_freq = atoi(freq); // conversion of character array to integer 

  uint16_t time_of_freq = 1/(2*input_freq*tick); // calculate the time for the specific frequency 
  uint16_t reloadvalue = (65536 - time_of_freq); // calculate the reload value based on the time of frequency, also based on prescaler 
  
  if (input_freq <= 8000 && input_freq >= 100) // do not exceed 8000 Hz und go beneath 100 Hz ( based in the assignment ) 
  {
    return reloadvalue; // if in range, activate piezo 
  }
  else 
  {
    PORTB &= ~(1<<PB2); // if exceeding or too low values are put in, then turn piezo off
  }
  
} 


// ------------------------------------------------------------------------
// STORE RECEIVED 
// ------------------------------------------------------------------------
void uart_rx_isr(void) 
{
  char received = 0;
  received = UDR0;
}


// ------------------------------------------------------------------------
// STORE TRANSMITTED 
// ------------------------------------------------------------------------
void uart_tx_isr(void)
{ 
  char received = 0;
  // wait until the Data Register Empty bit is set (polling to send)
   while ( !( UCSR0A & (1<<UDRE0)) )
   { }
}


// ------------------------------------------------------------------------
// OVERFLOW INTERRUPT
// ------------------------------------------------------------------------
ISR(TIMER1_OVF_vect){
  
  cli(); //clear Interrupt
  TCNT1 = reloadvalue_from_freq(frequency); // setting the reload value
  PORTB ^= (1<<PB2); // toggle piezo 
  sei(); // set Interrupt
}


// ------------------------------------------------------------------------
// MAIN
// ------------------------------------------------------------------------
int main()
{
  init_uart(115200); // Initialize the UART for 115200, 8N1
  DDRB |= (1<<PB2); // Output of Piezo element
  TCCR1B |= (1<<CS11); // based on Timer 1 and prescaler (Division factor 8), active the specific bitpositions
  TIMSK1 |= (1<<TOIE1); // Timer overflow interrupt enable for the Timer 1 
  
  sei(); // set Interrupt 
  
  while(1);
}