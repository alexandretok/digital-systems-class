int main(void){

  ADMUX = 0b01000000; // Configurar o Vcc como referencia e a porta A0 como ADC
  ADCSRA = 0b10000100; // Ativa o ADC e configura o divisor do clock (16)

  DDRB = 1<<0; // Configura a porta de D2 como saida

//  Serial.begin(9600);

   // Set on match, clear on TOP
   TCCR1A  = ((1 << COM1A1) | (1 << COM1A0));

   // Phase + Frequency Correct PWM, Fcpu speed
   TCCR1B  = ((1 << CS10) | (1 << WGM13));

   OCR1A=0xff00;
  
  while(1){
    ADCSRA |= 0b01000000; // Inicia a leitura
    while(!(ADCSRA & 0b00010000)); // Aguarda termino da leitura 50 - 600
//    Serial.println(ADC);
    if(ADC <= 200)
      PORTD = 1<<2;
    else
      PORTD = 0;
  }
}
