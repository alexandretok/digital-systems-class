/* Projeto de Sistemas Digitais - UFRN / DCA - 2017.1
 *
 * Componentes:
 * 1. Alexandre Justino
 * 2. Íkaro Breno
 * 3. João Fernandes
 * 4. Glauco Dantas
 *
 * Objetivo: Controlar um sistema de resfriamento, que possui os seguintes componentes:
 * 1. Ventilador
 * 2. Sensor analógico (luminosidade para verificar se há sol ou de temperatura)
 * 3. Um botão para ligar
 * 4. Três LEDs para informar o status do sistema
 *
 * Funcionamento do sistema:
 * 1. O motor do ventilador será controlado via PWM
 * 2. O LED 1 terá sempre o mesmo duty cicle do motor
 * 3. O LED 2 terá um duty cicle proporcional ao valor do sensor analógico (LDR ou temperatura)
 * 4. O LED 3 informa ON/OFF
 * 5. O motor deve seguir a seguinte curva de secagem:
 *   a. De 0 a 30 segundos: função linear crescente que vai de 0% a 30%
 *   b. De 30 a 60 segundos: função constante em 30%
 *   c. De 60 a 90 segundos: função linear crescente que vai de 30% a 75%
 *   d. De 90 a 120 segundos: função constante em 75%
 *   e. De 120 a 180 segundos: função linear decrescente que vai de 75% a 0%
 */

 /*
  * Pinagem:
  * Saída PWM 0A (do motor): PORTD6 (Pino 6 no Arduino)
  * Saída PWM 0B (do sensor): PORTD5 (Pino 5 no Arduino)
  * Entrada I/O do botão power: PORTD2 (Pino 2 no Arduino)
  */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char dutyCycle = 0; // Porcentagem do motor
volatile int quantidadeInterrupts = 0;
const char interruptsPorSegundo = 61; // (16MHz / prescaler * 256) (Fast PWM Mode) 
unsigned char segundosPassados = 0;

bool enviarDados = true;
bool sistemaAtivado = false;

float alfa = 1.0;

int main(void){

  ADMUX = 0b01000000; // Configurar o Vcc como referencia e a porta A0 como ADC
  ADCSRA = 0b10000100; // Ativa o ADC e configura o divisor do clock (16)

  DDRD = 1 << PORTD6 | 1 << PORTD5; // Configura portas I/O de saída e entrada
  PORTD = 1 << PORTD2; // Ativa o pull up do pino PORTD2 (Pino 2 no Arduino)

  // Clear OC0A on Compare Match, set OC0A at BOTTOM, (non-inverting mode)
  TCCR0A  = (1 << COM0A1);

  // Seta tipo de PWM para Fast PWM, TOP: 0xFF, Update of OCRx at BOTTOM, TOV flag set on MAX
  TCCR0A |= (1 << WGM00) | (1 << WGM01);

  // Ativa interrupt no overflow
  TIMSK0 = (1 << TOIE0);

  // Configura duty cycle inicial (zero)
  OCR0A = (dutyCycle/100.0) * 255;
  
  // Ativa interrupts externos
  sei();

  // Configura 1024 prescaler e também inicia o contador
  TCCR0B = (1 << CS00) | (1 << CS02);

  Serial.begin(9600);
  Serial.println("standBy:true;");

  while(1){
    if(sistemaAtivado){
      ADCSRA |= 0b01000000; // Inicia a leitura
      while(!(ADCSRA & 0b00010000)); // Aguarda termino da leitura

      // Alfa = valor de 0.9 a 1 que sera multiplicado pelo dutyCycle
      alfa = ADC * 0.000131579 + 0.9;
      if(alfa < 0.9) alfa = 0.9;
      if(alfa > 1) alfa = 1;
      
      dutyCycle = curvaDeResfriamento(float(segundosPassados) / 60.0) * alfa;
      
      // Verifica se ja se passou mais um segundo, se sim habilita envio de dados
      if(quantidadeInterrupts >= interruptsPorSegundo){
        quantidadeInterrupts = 0;
        segundosPassados++;
        enviarDados = true;
      }
  
      // Envia os dados uma vez por segundo
      if(enviarDados == true){
        
        /* 
         * ATENÇÃO ---->  (não podemos usar essas funções do Arduino!!!) 
         */
         
        Serial.print("tempo:");
        Serial.print(float(segundosPassados) / 60.0);
        Serial.print(";dutyCycle:");
        Serial.print(int(dutyCycle));
        Serial.println(";");
        enviarDados = false;
      }

      // Verifica se acabou o ciclo de resfriamento, se sim desligar o sistema e reiniciar as variaveis
      if(float(segundosPassados) / 60.0 >= 3.00){
        Serial.println("systemActivated:false;");
        Serial.println("standBy:true;");
        sistemaAtivado = false;
        segundosPassados = 0;
        quantidadeInterrupts = 0;
        dutyCycle = 0;
      }
    } else {
      // Fica verificando se o botão power foi pressionado
      if(~PIND & (1 << PORTD2)){
        sistemaAtivado = true;
        Serial.println("systemActivated:true;");
      }
    }
  }
}

/**
 * Essa função é chamada toda vez que há um
 * overflow no contador (apróximadamente a cada 16.4 milisegundos)
 */
ISR(TIMER0_OVF_vect){
  // Atualiza o duty cycle
  OCR0A = (dutyCycle/100.0) * 255;
  
  if(sistemaAtivado){
    quantidadeInterrupts++;
  }
}

/**
 * Essa função representa a curva de secagem
 */
char curvaDeResfriamento(double tempo){
  if(tempo <= 0.5)
    return 60 * tempo;
  else if(tempo <= 1.0)
    return 30;
  else if(tempo <= 1.5)
    return 90 * tempo - 60;
  else if(tempo <= 2.0)
    return 75;
  else if(tempo <= 3.0)
    return -75 * (tempo - 3);
  else return 0;
}

