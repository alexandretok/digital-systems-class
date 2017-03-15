// Ler um sensor digital em A0 e ligar os LEDS em D2,D3,D4,D5,D6 (um para cada Volt do sensor) 

int main(void){

  ADMUX = 0b01000000; // Configurar o Vcc como referencia e a porta A0 como ADC
  ADCSRA = 0b10000100; // Ativa o ADC e configura o divisor do clock (16)

  DDRD = 0b01111100; // Configura as portas de D2 a D6 como saida

//  Serial.begin(9600);
  
  while(1){
    ADCSRA |= 0b01000000; // Inicia a leitura
    while(!(ADCSRA & 0b00010000)); // Aguarda termino da leitura
//    Serial.println(ADC);
    if(ADC <= 100)
      PORTD = 0b00000000;
    else if(ADC <= 200)
      PORTD = 0b00000100;
    else if(ADC <= 400)
      PORTD = 0b00001100;
    else if(ADC <= 600)
      PORTD = 0b00011100;
    else if(ADC <= 800)
      PORTD = 0b00111100;
    else PORTD = 0b01111100;
  }
}
