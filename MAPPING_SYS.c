#include <Servo.h>

#include <SoftwareSerial.h>



//SENSORES

//sensor delantero 0 ->

#define trigDelatero 3
#define echoDelatero 2

//sensor trasero 1 ->

#define trigerTrasero 5
#define echoTrasero 4

//sensor el de debajo 2 ->

#define trigerBajos 7
#define echoBajos 6

//sensor dinamico delantero 3 -> 
#define trigerDinamicDel 8
#define echoDinamicDel 9

//sensor dinamico trasero 4 ->

#define trigerDinamicTras 11
#define echoDinamicTras 10

#define vib_pin 3
#define VibracionParado 1
#define CIEGO 400

Servo myservo;
int vel =0;
int sentido = 0; //0 = horario, 1 = antihorario// velocidad del servo 
long duration, distance;
int posrelativa1, posrelativa2;
int sensoresdinamicos[18][2];
int sensores[3]; //4 estandares y la 5ª es el sensor para prevenir la caida
#define choque 10
const int distanciaCaida = 15; //queda por calcular
int angulo=181;
int sensor=-1;
int sentidoln=0;

//1b: codigo trabajo (0x11)

//2b: codigo grupo (0x7)

//3b: (1: hacia atras, 0: hacia delante)

//4b: grados (0x0-0xB4)

//5b-8b: 0x0

//byte trama[8];

//trama[0] = 0x11;

//trama[1] = 0x07;

//trama[2] = 0x00;

//trama[3] = 0x00;

//trama[4] = 0x00;

//trama[5] = 0x00;

//trama[6] = 0x00;

//trama[7] = 0x00;

byte trama[] = {0x11,0x07,0x00,0x00,0x00,0x00,0x00,0x00};

SoftwareSerial mySerial(10,11);



void inicializararraypos();
void inicializaarraybool();
void inicializaestaticos();
int calculaposicionplataforma(int id);
void SonarSensor(int trigPin,int echoPin );
void rotacionplataformahorario();
void rotacionplataformaantihorario();
bool comprueba();
void buscaDistanciaMaxima();

void setup(){
  Serial.begin (9600);
  pinMode(vib_pin,INPUT);
  pinMode(trigDelatero, OUTPUT);
  pinMode(echoDelatero, INPUT);
  pinMode(trigerTrasero, OUTPUT);
  pinMode(echoTrasero, INPUT);
  pinMode(trigerBajos, OUTPUT);
  pinMode(echoBajos, INPUT);
  pinMode(trigerDinamicDel, OUTPUT);
  pinMode(echoDinamicDel, INPUT);
  pinMode(trigerDinamicTras, OUTPUT);
  pinMode(echoDinamicTras, INPUT);

  myservo.attach(13);
  myservo.write(vel); 
  inicializararraypos();
}



void imprime(){
  int aux = vel/10;
  Serial.print(sensores[0]);
  Serial.print(" - ");
  Serial.print(sensores[1]);
  Serial.print(" - ");
  Serial.println(sensores[2]);
  Serial.print("***Delante***");

  for(int i = 0; i < 18; i++){
    Serial.print(i*10);
    Serial.print(": ");
    Serial.print(sensoresdinamicos[i][0]);
    Serial.print(" / ");
  }

  Serial.println("");
  Serial.print("***Atras***");

  for(int i = 0; i < 18; i++){
    Serial.print(i*10);
    Serial.print(": ");
    Serial.print(sensoresdinamicos[i][1]);
    Serial.print(" / ");
  }

  Serial.println("");
}

void inicializaSensores(){ 

  SonarSensor(trigDelatero, echoDelatero);

  if(distance > CIEGO){
    sensores[0] = 500;
  }
  else sensores[0] = distance;

  SonarSensor(trigerTrasero, echoTrasero);
  if(distance > CIEGO){
    sensores[1] = 500;
  }
  else sensores[1] = distance;


  SonarSensor(trigerBajos, echoBajos);
  if(distance > CIEGO){
    sensores[2] = 500;
  }
  else sensores[2] = distance;


  SonarSensor(trigerDinamicDel, echoDinamicDel);
  if(distance > CIEGO){
    sensoresdinamicos[(vel/10)][0] = 500;
  }
  else sensoresdinamicos[(vel/10)][0] = distance;

  SonarSensor(trigerDinamicTras, echoDinamicTras);
  if(distance > CIEGO){
    sensoresdinamicos[(vel/10)][1] = 500;
  }
  else sensoresdinamicos[(vel/10)][1] = distance;

  if(comprueba()){
    //MIRAR TRAMA TE VAS A CAER BAJOS, CHOCAR ALANTE, CHOCAR ATRAS  estaticos
    buscaDistanciaMaxima();
    Serial.println("CAMBIO DE SENTIDO");
    Serial.print("sensor: ");
    Serial.print(sensor);
    Serial.print(" angulo: ");
    Serial.println(angulo);
    if(sensor == 1 || sensor == 4 || sensor == 2){ //hacia atras
      trama[2] = 0x01;
      sentidoln = 1;

    }
    else if(sensor == 0  ||sensor == 3){ //hacia delante
      trama[2] = 0x00;
      sentidoln = 0;
    }
    trama[3] = angulo;

    if(mySerial.available()) mySerial.write((uint8_t*)trama, sizeof(trama));  
    imprimeTrama();
  }
  else angulo = 181;

  if(sentidoln==1){
    Serial.println("VOY HACIA ATRAS");
  }
  else if(sentidoln==0){
    Serial.println("VOY HACIA  DELANTE");
  }


}



bool compruebaDinamico(){
  angulo=181;
  sensor=-1;
  int res = false;

  for(int i=0; i < 18; ++i){
    //choque = 10
    if(sensoresdinamicos[i][0] <= choque){
      angulo=i*10;
      sensor=3;
      res = true;
      break;
    }
    if(sensoresdinamicos[i][1] <= choque){
      angulo=i*10;
      sensor=4;
      res = true;
      break;
    }
  }
  return res;
}



bool comprueba(){
  angulo=181;
  sensor=-1;
  int res = false;

  for(int j=0;j<2;++j){
    if(sensores[j]<=choque){
      sensor=j;
      res = true;
      break;
    }
  }
  if(sensores[2] > distanciaCaida){
    sensor = 2;
    res = true;
  }

  return res;
}
void buscaDistanciaMaxima(){
  int maxim=0;

  for(int i=0; i < 18; ++i){
    if(sensoresdinamicos[i][0]>maxim){
      sensor=3;
      maxim=sensoresdinamicos[i][0];
      angulo=i*10;
    }
    if(sensoresdinamicos[i][1]>maxim){
      sensor=4;
      maxim=sensoresdinamicos[i][1];
      angulo=i*10;
    }
  } 
}

void rotacionplataformahorario(){
  vel += 10;
  myservo.write(vel);   
}

void rotacionplataformaantihorario(){
  vel -= 10;
  myservo.write(vel);       
}



void rotacionPlataforma(){
  if(sentido==0){
    rotacionplataformahorario();
    if(vel>=180){
      sentido=1;
      if(compruebaDinamico()){
        buscaDistanciaMaxima();
        if(sensor==3){
          trama[2] = 0x00; //delante
        }
        else if(sensor==4){
          trama[2] = 0x01;
        }
        trama[3] = angulo;
        if(mySerial.available()) mySerial.write((uint8_t*)trama, sizeof(trama));
      }
    }
  }

  if(sentido==1){
    rotacionplataformaantihorario();
    if(vel<=0){
      sentido=0;
      if(compruebaDinamico()){
        buscaDistanciaMaxima();
        if(sensor==3){
          trama[2] = 0x00; //atras
        }
        else if(sensor==4){
          trama[2] = 0x01;
        }
        trama[3] = angulo;
        if(mySerial.available()) mySerial.write((uint8_t*)trama, sizeof(trama));
        imprimeTrama();
      }
    }   
  }
  delay(100);    
}



void inicializararraypos(){
  for(int i = 0; i < 18; i++){
    sensoresdinamicos[i][0] = 0;
    sensoresdinamicos[i][1] = 0;
  }
}

void inicializaestaticos(){  
  for(int i = 0; i < 3; i++){
    sensores[i] = 0;
  }
}

void SonarSensor(int trigPin,int echoPin ){
  distance=0;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration/58);
}

void imprimeTrama(){
  for(int i=0;i < 8;i++){
    Serial.print((trama[i]), HEX);
    Serial.print("_");
    if(i==7)Serial.println();
  }
}

void loop() {
  int val;
  // val=digitalRead(vib_pin);
  //val devuelve el valor de la vibracion
  //Habria que hacer pruebas para saber que datos son ruido 
  //Y que valores significa que el coche se mueve
  //if(val>=VibracionParado) //si el coche se mueve ejecuta el codigo (1 como referencia)
  //  {
  for(int i = 0; i < 18; i++){
    inicializaSensores();
    rotacionPlataforma();
  }
  imprime();

}
