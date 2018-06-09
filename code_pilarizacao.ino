DESENVOLVIDO POR PATRICK MARTINS
COMENTADO POR ITALO
//========================================
// PINAGEM DO MOTOR
// pino motor1 = gnd
// pino motor2 = gnd
// pino motor3 = D2 (Encoder fase A)
// pino motor4 = D4 (Encoder fase B)
// pino motor5 = 3.3v (Alimentação do Encoder)
// pino motor6 = saida tip 122 = D3  
//=========================================
#include <termostato.h> //  https://github.com/pattrickx/termostato    // Biblioteca usada para medir a temperatura;
termostato termo;

const byte Encoder_C1 = 2;
const byte Encoder_C2 = 4;
byte Encoder_C1Last;
int duracao;     // Variável que permite saber o número de pulsos do motor;
boolean Direcao; // Variável que permite saber o sentido da rotação do motor;

#define motor  3  // Pino digital 3;
#define temp   A0 // Pino analógico A0;
#define cooler 7  // Pino digital 7;
#define bomba  8  // Pino digital 8;
#define forno  12 // Pino digital 12;
#define but1   5  // Pino digital 5;
#define but2   6  // Pino digital 6;

int but11=0;  // Variável auxiliar que indica se o sistema está ligado;
int i=0;      // Variável para a medição do tempo de gotejamento;
int timer1=0; // Variável para a medição do tempo de agitação;
int i1=0;     // Variável para a medição do tempo de aquecimento do processo até 85 graus;
int tempo = 3600; 
void setup() {
   
  Serial.begin(57600); // Velocidade serial do arduino;
  
  pinMode(temp, INPUT);
  pinMode(motor, OUTPUT);
  pinMode(cooler, OUTPUT);
  pinMode(bomba, OUTPUT);
  pinMode(forno, OUTPUT);
  pinMode(but1, INPUT);
  pinMode(but2, INPUT);
  Serial.print("iniciado");
  termo.setPinact(forno); // Seleciona pino de ação do termostato;
  termo.setTempend(85);   // Seleciona temperatura de desligamento ou ligamento;
  termo.Thermistor(temp); // Seleciona pino de entrada do termistor (sensor NTC);
  termo.setRes(12000);    // Indica a resistência elétrica do termistor;
  EncoderInit();
  digitalWrite(forno, HIGH);// Desliga o forno;
}

void loop() {
   
  int valor = 100;         // Define o pulso do motor via PWM;
  if (digitalRead(but1) == HIGH && but11==0){  // Verifica se botão 1 foi pressionado
    but11=1;}                                  // para ligar o processo;
  delay(200);
 
  while(i<tempo && but11==1){ // Define tempo de funcionamento do ciclo de aquecimento;
    
    if (digitalRead(but1) == HIGH){ // Verifica se botão 1 foi novamente pressionado 
    but11=0;}                       // para parar o processo em caso de emergência;
    
    /// Rotina para acelerar o motor em caso de necessidade;
    if(digitalRead(but2)==HIGH&& valor == 100){
    valor=255;}
    else if(digitalRead(but2)==HIGH && valor == 255){
    valor=100;}

    /// Controle de acionamento dos coolers;
    if(termo.getTemp()>50){      // Temperaturas superiores a 50 graus o cooler liga;
      digitalWrite(cooler,HIGH); 
    }
    else if(termo.getTemp()<40 ){// Temperaturas inferiores a 40 graus o cooler desliga;
      digitalWrite(cooler,LOW);
    }

    /// Acionamento da bomba;
    if(termo.getTemp()>50){     // Temperaturas superiores a 50 graus a bomba liga;
      digitalWrite(bomba,HIGH); 
    }
    
    double temp = termo.getTemp();   //Recebe temperatura do sensor;   
    termo.actionHot();              // Função de aquecimento;
    delay(800);
   
    if (termo.getTemp()>=84){   // Inicia timer de agitação;
      timer1=1;
      analogWrite(motor, valor); // Aciona o motor de mistura via PWM;
    }

     if(timer1==0){ // Tempo de aquecimento do início do processo até atingir os 85 graus;
      i1++;         
    }
    
    if (timer1==1){ // Timer contador do gotejamento;
      i++; 
    }
    
    ////// Encoder;
    Serial.print("Rotações por minuto do motor: ");
    Serial.println(duracao/10);
    duracao=0;
    
    delay(100);
      
    Serial.print("Temperatura do processo: ");
    Serial.println(temp);             // Imprime na tela serial o valor da temperatura; 
    
    Serial.println(digitalRead(but2));// Mostra o status do botão 2;
    Serial.println(digitalRead(but1));// Mostra o status do botão 1;
    Serial.println(but11);            // Mostra o status do sistema, 1 para Ligado e 0 para Desligado; 
    
    Serial.print("Pulso do motor P W M:");
    Serial.println(valor);            // Mostra na tela serial o valor de PWM do motor;

    Serial.print("Tempo em segundos de aquecimento :");// Mostra na tela serial o tempo decorrido desde o
    Serial.println(i1);                               // começo do processo até atingir os 85 graus;
                            
    Serial.print("Tempo em segundos de gotejamento:");// Mostra na tela serial o tempo decorrido desde o 
    Serial.println(i);                                // inicio do processo de gotejamento;
    
    Serial.print("Temperatura do microcontrolador:");   
    Serial.println(GetTemp(),1);      // Mostra na tela serial a temperatura do microcontrolador da 
    Serial.println("");               // placa Arduino para analisar se está havendo sobreaquecimento;
  }
  analogWrite(motor, 0);
      but11=0;
   if (i==tempo){
    termo.actionHot();
      Serial.print("Temperatura do processo: ");
      Serial.println(termo.getTemp());
      Serial.print("Temperatura do microcontrolador:");
      Serial.println(GetTemp(),1);
   }
   if(digitalRead(but2==HIGH && i==tempo)){ // Rotina para encerrar o processo a qualquer momento
    digitalWrite(bomba, LOW);              // após o período de gotejamento;
    digitalWrite(forno, HIGH);       
    digitalWrite(cooler, LOW);
    i=0;
   }
   
   duracao=0;
}

///Funcionamento do encoder;

void EncoderInit(){
  pinMode(Encoder_C2, INPUT);
  attachInterrupt(0, calculapulso, CHANGE);
}

void calculapulso(){
  int Lstate = digitalRead(Encoder_C1);
  if ((Encoder_C1Last == LOW) && Lstate == HIGH)
  {
    int val = digitalRead(Encoder_C2);
    if (val == LOW && Direcao)
    {
      Direcao = false; //Reverse;
    }
    else if (val == HIGH && !Direcao)
    {
      Direcao = true;  //Forward;
    }
  }
  Encoder_C1Last = Lstate;

  if (!Direcao)  duracao++;
  else  duracao--;
}

/// Sensor de temperarura interno do arduino;
double GetTemp(void){
  unsigned int valorADC;
  double temperatura;
 
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3)); //Seleciona tensão interna de 1,1 V e canal 8;
  ADCSRA |= _BV(ADEN);              // Habilita o conversor AD;
 
  delay(20);                        // Aguarda estabilização da tensão;
 
  ADCSRA |= _BV(ADSC);              // Inicia a conversão;
                                  
  while (bit_is_set(ADCSRA,ADSC));  //Aguarda o fim da conversão;
 
  valorADC = ADCW;                  // Lê o valor da conversão; 
 
  temperatura = (valorADC - 324.31 ) / 1.22; // Calcula o valor da temperatura; 
 
  return (temperatura);             // Retorna o valor da temperatura em °C;
}
