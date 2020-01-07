#define F_CPU 16000000                  
#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>


void init_uart (uint32_t baudrate) 
{
    unsigned int ubrr = (F_CPU/8/baudrate)-1;

    UBRR0H = (uint8_t)(ubrr>>8);      // Set baud rate by filling high and
    UBRR0L = (uint8_t)(ubrr&0xff);    // low byte of the Baud Rate Register
    UCSR0A |= (1<<U2X0);                    // double speed for UART

    UCSR0B = ( 1<<TXEN0 | 1<<RXEN0 );              // Enable receiver and transmitter 
    UCSR0C = ( 1<<UCSZ01 | 1<<UCSZ00 ); // select character size, stop and parity bits: 8N1

    DDRB |= (1<<PB4); // Output of Piezo 
    PORTD |= (1<<PD4); // button
    
}

void uart_transmit(char c)               // helper function to put out a character on the UART
{
   // wait until the Data Register Empty bit is set (polling to send)
   while ( !( UCSR0A & (1<<UDRE0)) )
   { }
   UDR0 = c;                             // send out the character
}

void uart_sendstring (char * str)       
{
   while (*str) { uart_transmit(*str); str++; }   // walk through zero-terminated string
}



void adc_init(void) {
  ADMUX = (1<<REFS0);    //Avcc(+5v) as voltage reference
  ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);    // Prescaler
  ADCSRA |= (1<<ADEN);                  // activate  ADC
}


uint16_t adc_read(uint8_t channel)
{
  ADMUX = (1<<6) + channel ;  // select REFS0 bit for AVCC reference and select channel MUX

  ADCSRA |= (1<<ADSC);            // start single conversion
  while (ADCSRA & (1<<ADSC) );    // wait for completion


  uint16_t result;
  result = ADCL;
  result += (ADCH << 8);   // shift the high byte 8 positions to the left 
  

  return result;                                 // return result (16 bit )

}


void int_to_str (uint16_t value, char * target)
 {
   int8_t i; // int does mean = signed = to not have an endless loop 
   for(i=3; i>= 0; i--) // correct order 1 0 2 3 
   {
     target[i] = (value % 10) + '0';
     value /= 10; 
   }
   target[4] = '\0'; //to set the last element of string 
 }

uint16_t LDR_to_freq(uint16_t LDR)
{ 
   return (20*LDR+50);
  }

uint16_t freq_to_delay(uint16_t FREQ){


  uint16_t delay_ = 1000000/FREQ/2; 
  //delay_ = delay_/2.0;

  return delay_;
  }

void piezo (uint16_t DEL) { //parameter delay in microsecond
  
  // switch on output pin for delay
  PORTB |= (1<<4);
  func_delay(DEL);

  //switch off output pin for delay
  PORTB &= ~(1<<4); 
  func_delay(DEL);
  
  }  

void func_delay(uint16_t microseconds){
    
    for (; microseconds > 0; microseconds--)
    {
          _delay_us(1);
          
      }
  }  

  
int main()
{

  init_uart(115200); // Initialize the UART for 115200, 8N1
  adc_init();
  uint16_t LDR_input_value;     // LDR Input value
  char mystring[5];             // address via mystring[0] - mystring[4]
  uint8_t buttonpressed = 0;
  float factor = 19950.0/1023.0;
  while(1)
  {
// ------------------------------------------------
// reading and inverting the LDR value 
// ------------------------------------------------
     uart_sendstring("LDR: ");
     LDR_input_value = adc_read(0); // reading LDR Value
     LDR_input_value = 1023 - LDR_input_value; // inverting the LDR value
     int_to_str(LDR_input_value, mystring); 
     uart_sendstring(mystring);
     uart_transmit('\n');
     
// ------------------------------------------------
// convert from LDR to Freqeuncy
// ------------------------------------------------     

    uint16_t frequency = LDR_to_freq(LDR_input_value);

// ------------------------------------------------
// convert from frequency to delay
// ------------------------------------------------     

    uint16_t delay_value =  freq_to_delay(frequency);
// ------------------------------------------------
// make piezo sound with delay
// ------------------------------------------------       
    
    PORTB &= ~(1<<4);         
    
   if ((PIND & (1<<4)) == 0) {
      buttonpressed = !buttonpressed;
      while ((PIND & (1<<4)) == 0){};
     }

    if (buttonpressed){
        piezo(delay_value); 
      }
    else {
      
      PORTB &= ~(1<<4);
    }
  
  
  }

}