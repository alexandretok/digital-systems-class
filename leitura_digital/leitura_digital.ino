// Ler a entrada digital D2 e ligar um led em D3
// Ler a porta analogica 0 e 

int main(void){

  DDRD =  1<<3; // Configura D2 como entrada e D3 como saida
  
  while(1){
    PORTD = (PIND & 1<<2) << 1;
  }
}
