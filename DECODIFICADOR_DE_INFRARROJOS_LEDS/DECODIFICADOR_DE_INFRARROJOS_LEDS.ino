/*
 * IRremote: IRrecvDump - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * LG added by Darryl Smith (based on the JVC protocol)
 */

#include <IRremote.h>

#define IR_SENSOR_POWER 2

#define LED_AZUL 5
#define LED_VERDE 6
#define LED_ROJO 9

int RECV_PIN = 3; 

IRrecv irrecv(RECV_PIN);

decode_results results;

//--------------------------------
#define AND    &&
#define OR     ||
#define NOT    !

unsigned long Temporizador1;
unsigned char Intermitencia1;

unsigned long Temporizador2;
unsigned char Intermitencia2;

unsigned long Temporizador3;
unsigned char Intermitencia3;
//--------------------------------

int MODE_LEDS = 999;

#define INTENSIDAD_LED_MIN 0
#define INTENSIDAD_LED_MAX 15
int INTENSIDAD_LED_AZUL = 15;
int INTENSIDAD_LED_VERDE = 15;
int INTENSIDAD_LED_ROJO = 15;

int INTENSIDAD_LED_ANALOG_AZUL = 255;
int INTENSIDAD_LED_ANALOG_VERDE = 255;
int INTENSIDAD_LED_ANALOG_ROJO = 255;


#define VELOCIDAD_PARPADEO_MIN 0
#define VELOCIDAD_PARPADEO_MAX 10
int VELOCIDAD_PARPADEO_LED_AZUL = 5;
int VELOCIDAD_PARPADEO_LED_VERDE = 5;
int VELOCIDAD_PARPADEO_LED_ROJO = 5;

//DEFINICION PARA LOS VALORES DE RETARDO DE CADA LED
#define RETARDO_PARPADEO_MIN 17
#define RETARDO_PARPADEO_MAX 700
unsigned long RETARDO_PARPADEO_LED_AZUL = 130;
unsigned long RETARDO_PARPADEO_LED_VERDE = 300;
unsigned long RETARDO_PARPADEO_LED_ROJO = 400;

//---------------------------
#define TDELAY_TEST 5

void setup()
{
  Serial.begin(9600);

  pinMode(IR_SENSOR_POWER, OUTPUT);
  pinMode(13, OUTPUT);

  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  delay(100);
  digitalWrite(IR_SENSOR_POWER, HIGH);
  digitalWrite(13, HIGH);

  analogWrite(LED_AZUL, 0);
  analogWrite(LED_VERDE, 0);
  analogWrite(LED_ROJO, 0);
  
  delay(100);
  
  /*
  LOW: La interrupción se dispara cuando el pin es LOW.
  CHANGE: Se dispara cuando pase de HIGH a LOW o viceversa.
  RISING: Dispara en el flanco de subida (Cuando pasa de LOW a HIGH).
  FALLING: Dispara en el flanco de bajada (Cuando pasa de HIGH a LOW).
  */
  
  attachInterrupt(0, IR_Service, FALLING);
  delay(100);
  
  irrecv.enableIRIn(); // Start the receiver

  delay(100);
  testLeds();
  
}


//-------------------------------------------------
//NO TOCAR NINGUNO DE LOS SIGUIENTES CODIGOS

void testLeds(){
  for(int i = 0; i <= 255; i++){
    delay(TDELAY_TEST);
    analogWrite(LED_VERDE, i);
    analogWrite(LED_AZUL, i);
    analogWrite(LED_ROJO, i);
  }
  for(int i = 255; i >= 0; i--){
    delay(TDELAY_TEST);
    analogWrite(LED_VERDE, i);
    analogWrite(LED_AZUL, i);
    analogWrite(LED_ROJO, i);
  }
}

/*
La primera función que vamos a escribir se llamará iniTemp() y se encarga de arrancar un cronómetro 
(reloj secundario) indicando el tiempo que deseamos que se cuente. 

La segunda función será chkTemp() y nos dirá los milisegundos que  restan para que el 
cronómeto alcance el tiempo deseado.
*/
//iniTemp() Function
//Arranca un temporizador durante un numero de milisegundos
//Modo de Uso: iniTemp( &lT1, 1000);
void iniTemp(unsigned long *Cronometro, unsigned long Time)
{
  *Cronometro = millis() + Time; //Presición de Milisegundos
  //*Cronometro = micros() + Time; //Presición de Microsegundos
}

// chkTemp() Function.
// Devuelve en unidades de 1ms el tiempo que resta del temporizador
// Modo de Uso: nVar1 = iniTemp( &lT1 );
unsigned long chkTemp(unsigned long *Cronometro)
{
  if( *Cronometro > millis() )
  {
     return (*Cronometro - millis() ); //Presición de Milisegundos
     //return (*Cronometro - micros() ); //Presición de Microsegundos
  } 
  else
  {
     return 0; 
  }
}


/*
Antes de utilizar estas dos funciones y ver con detalle cómo están escritas, 
vamos a construir una tercera, a la que llamaremos parpadeo() y que utiliza las 
dos anteriores para generar una intermitencia de una frecuencia determinada. 
Su código es el siguiente:
*/

// Parpadeo() Function.
// Parpadeo
// Modo de Uso: Parpadeo( input, 1000, &lT1, &salida_parpadeo)

void Parpadeo( unsigned char in, 
               unsigned long tiempo, 
               unsigned long *Cronometro, 
               unsigned char *out)
{
   if ( in AND NOT ( (chkTemp(Cronometro) > 0 ? 1 : 0 ) ) )
   {
      iniTemp( Cronometro, tiempo );
      *out = NOT *out;
   } 
   else if( NOT in )
   {
      *out = 0; 
   }
}

void Pulso(unsigned char Task, 
           unsigned long Time, 
           unsigned long *AuxT1, 
           unsigned char *AuxTm1, 
           unsigned char *Out)
{
  if (Task AND NOT *AuxTm1)
  {
     iniTemp( AuxT1, Time );
     *AuxTm1 = 1; 
  }
  else if (NOT Task AND *AuxTm1)
  {
     *AuxTm1 = 0; 
  }
  *Out = ( ( chkTemp( AuxT1 ) > 0 ? 1 : 0 ) ) AND Task;
}
//-------------------------------------------------

void loop() {
  
  if (irrecv.decode(&results)) {
    //Serial.println(results.value, HEX);
   
    switch (results.value)
    {   case 0xFF629D: //BOTON - UP
          Serial.println("Boton: [ARRIBA]");
          POWER_IR_SENSOR(false);

          if (MODE_LEDS == 1 || MODE_LEDS == 2 || MODE_LEDS == 3){
            
              switch(MODE_LEDS) {
                case 1:
                  if (INTENSIDAD_LED_AZUL < INTENSIDAD_LED_MAX) {
                    INTENSIDAD_LED_AZUL = INTENSIDAD_LED_AZUL + 1;
                    INTENSIDAD_LED_ANALOG_AZUL = map(INTENSIDAD_LED_AZUL, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_AZUL, INTENSIDAD_LED_ANALOG_AZUL);
                  }
                break;

                case 2:
                  if (INTENSIDAD_LED_VERDE < INTENSIDAD_LED_MAX) {
                    INTENSIDAD_LED_VERDE = INTENSIDAD_LED_VERDE + 1;
                    INTENSIDAD_LED_ANALOG_VERDE = map(INTENSIDAD_LED_VERDE, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_VERDE, INTENSIDAD_LED_ANALOG_VERDE);
                  }
                break;

                case 3:
                  if (INTENSIDAD_LED_ROJO < INTENSIDAD_LED_MAX) {
                    INTENSIDAD_LED_ROJO = INTENSIDAD_LED_ROJO + 1;
                    INTENSIDAD_LED_ANALOG_ROJO = map(INTENSIDAD_LED_ROJO, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_ROJO, INTENSIDAD_LED_ANALOG_ROJO);
                  }
                break;
              }
          }
             break;
             
        case 0xFFA857: //BOTON - DOWN
          Serial.println("Boton: [ABAJO]");
          POWER_IR_SENSOR(false);

          if (MODE_LEDS == 1 || MODE_LEDS == 2 || MODE_LEDS == 3){
            
            switch(MODE_LEDS) {
                case 1:
                  if (INTENSIDAD_LED_AZUL > INTENSIDAD_LED_MIN) {
                    INTENSIDAD_LED_AZUL = INTENSIDAD_LED_AZUL - 1;
                    INTENSIDAD_LED_ANALOG_AZUL = map(INTENSIDAD_LED_AZUL, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_AZUL, INTENSIDAD_LED_ANALOG_AZUL);
                  }
                break;

                case 2:
                  if (INTENSIDAD_LED_VERDE > INTENSIDAD_LED_MIN) {
                    INTENSIDAD_LED_VERDE = INTENSIDAD_LED_VERDE - 1;
                    INTENSIDAD_LED_ANALOG_VERDE = map(INTENSIDAD_LED_VERDE, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_VERDE, INTENSIDAD_LED_ANALOG_VERDE);
                  }
                break;

                case 3:
                  if (INTENSIDAD_LED_ROJO > INTENSIDAD_LED_MIN) {
                    INTENSIDAD_LED_ROJO = INTENSIDAD_LED_ROJO - 1;
                    INTENSIDAD_LED_ANALOG_ROJO = map(INTENSIDAD_LED_ROJO, INTENSIDAD_LED_MIN, INTENSIDAD_LED_MAX, 0, 255);
                    analogWrite(LED_ROJO, INTENSIDAD_LED_ANALOG_ROJO);
                  }
                break;
              }   
          }
             break;
             
        case 0xFF22DD: //BOTON - LEFT
          Serial.println("Boton: [IZQUIERDA]");
          POWER_IR_SENSOR(false);

          if (MODE_LEDS == 7 || MODE_LEDS == 8 || MODE_LEDS == 9 || MODE_LEDS == 0){
            
            switch(MODE_LEDS) {
                case 7:
                  if (VELOCIDAD_PARPADEO_LED_AZUL < VELOCIDAD_PARPADEO_MAX) {
                    VELOCIDAD_PARPADEO_LED_AZUL = VELOCIDAD_PARPADEO_LED_AZUL + 1;
                    RETARDO_PARPADEO_LED_AZUL = map(VELOCIDAD_PARPADEO_LED_AZUL, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;

                case 8:
                  if (VELOCIDAD_PARPADEO_LED_VERDE < VELOCIDAD_PARPADEO_MAX) {
                    VELOCIDAD_PARPADEO_LED_VERDE = VELOCIDAD_PARPADEO_LED_VERDE + 1;
                    RETARDO_PARPADEO_LED_VERDE = map(VELOCIDAD_PARPADEO_LED_VERDE, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;

                case 9:
                  if (VELOCIDAD_PARPADEO_LED_ROJO < VELOCIDAD_PARPADEO_MAX) {
                    VELOCIDAD_PARPADEO_LED_ROJO = VELOCIDAD_PARPADEO_LED_ROJO + 1;
                    RETARDO_PARPADEO_LED_ROJO = map(VELOCIDAD_PARPADEO_LED_ROJO, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;
              }
          }
             break;
             
        case 0xFFC23D: //BOTON - RIGHT
          Serial.println("Boton: [DERECHA]");
          POWER_IR_SENSOR(false);

          if (MODE_LEDS == 7 || MODE_LEDS == 8 || MODE_LEDS == 9 || MODE_LEDS == 0){

            switch(MODE_LEDS) {
                case 7:                  
                  if (VELOCIDAD_PARPADEO_LED_AZUL > VELOCIDAD_PARPADEO_MIN) {
                    VELOCIDAD_PARPADEO_LED_AZUL = VELOCIDAD_PARPADEO_LED_AZUL - 1;
                    RETARDO_PARPADEO_LED_AZUL = map(VELOCIDAD_PARPADEO_LED_AZUL, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;

                case 8:
                  if (VELOCIDAD_PARPADEO_LED_VERDE > VELOCIDAD_PARPADEO_MIN) {
                    VELOCIDAD_PARPADEO_LED_VERDE = VELOCIDAD_PARPADEO_LED_VERDE - 1;
                    RETARDO_PARPADEO_LED_VERDE = map(VELOCIDAD_PARPADEO_LED_VERDE, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;

                case 9:
                  if (VELOCIDAD_PARPADEO_LED_ROJO > VELOCIDAD_PARPADEO_MIN) {
                    VELOCIDAD_PARPADEO_LED_ROJO = VELOCIDAD_PARPADEO_LED_ROJO - 1;
                    RETARDO_PARPADEO_LED_ROJO = map(VELOCIDAD_PARPADEO_LED_ROJO, VELOCIDAD_PARPADEO_MIN, VELOCIDAD_PARPADEO_MAX, RETARDO_PARPADEO_MIN, RETARDO_PARPADEO_MAX);
                  }
                break;
              }
          }
             break;
             
        case 0xFF02FD: //BOTON - OK
          Serial.println("Boton: [OK]");
          POWER_IR_SENSOR(false);
             break;
             
        case 0xFF6897: //BOTON - 1
          Serial.println("Boton: [1]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_AZUL, INTENSIDAD_LED_ANALOG_AZUL);
          MODE_LEDS = 1;
             break;
             
        case 0xFF9867: //BOTON - 2
          Serial.println("Boton: [2]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_VERDE, INTENSIDAD_LED_ANALOG_VERDE);
          MODE_LEDS = 2;
             break;
             
        case 0xFFB04F: //BOTON - 3
          Serial.println("Boton: [3]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_ROJO, INTENSIDAD_LED_ANALOG_ROJO);
          MODE_LEDS = 3;
             break;
             
        case 0xFF30CF: //BOTON - 4
          Serial.println("Boton: [4]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_AZUL, LOW);
          MODE_LEDS = 4;
             break;
             
        case 0xFF18E7: //BOTON - 5
          Serial.println("Boton: [5]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_VERDE, LOW);
          MODE_LEDS = 5;
             break;
             
        case 0xFF7A85: //BOTON - 6
          Serial.println("Boton: [6]");
          POWER_IR_SENSOR(false);
          analogWrite(LED_ROJO, LOW);
          MODE_LEDS = 6;
             break;
             
        case 0xFF10EF: //BOTON - 7
          Serial.println("Boton: [7]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 7;
             break;
             
        case 0xFF38C7: //BOTON - 8
          Serial.println("Boton: [8]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 8;
             break;
             
        case 0xFF5AA5: //BOTON - 9
          Serial.println("Boton: [9]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 9;
             break;
             
        case 0xFF4AB5: //BOTON - 0
          Serial.println("Boton: [0]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 0; //PARPADEO: TODOS LOS LEDS
             break;
             
        case 0xFF42BD: //BOTON - *
          Serial.println("Boton: [*]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 0xFF; //APAGA TODOS LOS LEDS
             break;
             
        case 0xFF52AD: //BOTON - #
          Serial.println("Boton: [#]");
          POWER_IR_SENSOR(false);
          MODE_LEDS = 0xFF; //APAGA TODOS LOS LEDS
             break;
      }
      POWER_IR_SENSOR(true);
      irrecv.resume(); // Receive the next value
      delay(100);
  }

  switch (MODE_LEDS)
    {
      case 7: //PARPADEO: LED AZUL
        //Activacion = 1;
        parpadeoAleatorio_LED_AZUL(1);
        break;
        
      case 8: //PARPADEO: LED VERDE
        //Activacion = 1;
        parpadeoAleatorio_LED_VERDE(1);
        break;
        
      case 9: //PARPADEO: LED ROJO
        //Activacion = 1;
        parpadeoAleatorio_LED_ROJO(1);
        break;

      case 0: //PARPADEO: TODOS LOS LEDS
        //Activacion = 1;
        parpadeoAleatorio(1);
        break;
        
      case 0xFF: //APAGA TODOS LOS LEDS
        analogWrite(LED_AZUL, 0);
        analogWrite(LED_VERDE, 0);
        analogWrite(LED_ROJO, 0);
        break;
    }
}

void POWER_IR_SENSOR(boolean value){
  digitalWrite(IR_SENSOR_POWER, value);
}

void IR_Service(){
    Serial.println("Interrupcion Lanzada correctamente!");
}

void parpadeoAleatorio_LED_AZUL(int Activacion){

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_AZUL, &Temporizador2, &Intermitencia2);
  //digitalWrite(LED_AZUL, Intermitencia2);
  if (Intermitencia2){
    analogWrite(LED_AZUL, INTENSIDAD_LED_ANALOG_AZUL); 
  } else {
    analogWrite(LED_AZUL, 0); 
  }
}

void parpadeoAleatorio_LED_VERDE(int Activacion){

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_VERDE, &Temporizador1, &Intermitencia1);
  //digitalWrite(LED_VERDE, Intermitencia1);
  if (Intermitencia1){
    analogWrite(LED_VERDE, INTENSIDAD_LED_ANALOG_VERDE); 
  } else {
    analogWrite(LED_VERDE, 0); 
  }
}

void parpadeoAleatorio_LED_ROJO(int Activacion){

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_ROJO, &Temporizador3, &Intermitencia3);
  digitalWrite(LED_ROJO, Intermitencia3);
  if (Intermitencia3){
    analogWrite(LED_ROJO, INTENSIDAD_LED_ANALOG_ROJO); 
  } else {
    analogWrite(LED_ROJO, 0); 
  }
}

void parpadeoAleatorio(int Activacion){

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_AZUL, &Temporizador2, &Intermitencia2);
  //digitalWrite(LED_AZUL, Intermitencia2);
  if (Intermitencia2){
    analogWrite(LED_AZUL, INTENSIDAD_LED_ANALOG_AZUL); 
  } else {
    analogWrite(LED_AZUL, 0); 
  }

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_VERDE, &Temporizador1, &Intermitencia1);
  //digitalWrite(LED_VERDE, Intermitencia1);  
  if (Intermitencia1){
    analogWrite(LED_VERDE, INTENSIDAD_LED_ANALOG_VERDE); 
  } else {
    analogWrite(LED_VERDE, 0); 
  }

  Parpadeo(Activacion, RETARDO_PARPADEO_LED_ROJO, &Temporizador3, &Intermitencia3);
  //digitalWrite(LED_ROJO, Intermitencia3);
  if (Intermitencia3){
    analogWrite(LED_ROJO, INTENSIDAD_LED_ANALOG_ROJO); 
  } else {
    analogWrite(LED_ROJO, 0); 
  }
}
