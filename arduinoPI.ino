#include <LiquidCrystal.h>
#include <Servo.h>
#include <math.h>
#include <EEPROM.h>

struct Solo {
  char tipoDeSolo[20];
  char capacidadeDeRetencao[30];
};
typedef struct Solo TpSolo;

struct Planta {
  int id;
  char nome[56];
  char tipo[20];
  char faseCrescimento[20];
  TpSolo solo;
};
typedef struct Planta TpPlanta;

void escreverPlanta(unsigned int endereco, TpPlanta planta) {
  EEPROM.put(endereco, planta);
}

TpPlanta lerPlanta(int endereco) {
  TpPlanta planta;
  EEPROM.get(endereco, planta);
  return planta;
}

void escreverSolo(unsigned int endereco, TpSolo solo) {
  EEPROM.put(endereco, solo);
}

TpSolo lerSolo(unsigned int endereco) {
  TpSolo solo;
  EEPROM.get(endereco, solo);
  return solo;
}

LiquidCrystal LCD(12, 11, 5, 4, 3, 2);

void armazenarSolo() {
  TpSolo solo1;
  strcpy(solo1.tipoDeSolo, "Argiloso");
  strcpy(solo1.capacidadeDeRetencao, "Alta");
  
  
  Solo solo2;
  strcpy(solo2.tipoDeSolo, "Arenoso");
  strcpy(solo2.capacidadeDeRetencao, "Média");
  
  Solo solo3;
  strcpy(solo3.tipoDeSolo, "Siltoso");
  strcpy(solo3.capacidadeDeRetencao, "Baixa");
  
  Solo solo4;
  strcpy(solo4.tipoDeSolo, "Calcário");
  strcpy(solo4.capacidadeDeRetencao, "Muito alta");
  
  Solo solo5;
  strcpy(solo5.tipoDeSolo, "Pedregoso");
  strcpy(solo5.capacidadeDeRetencao, "Muito baixa");

  // Armazenamento das instâncias de Solo em diferentes endereços
  escreverSolo(0x0100, solo1);
  escreverSolo(0x0150, solo2);
  escreverSolo(0x01A0, solo3);
  escreverSolo(0x01F0, solo4);
  escreverSolo(0x0240, solo5);
}

void armazenarPlantas() {
  // Exemplo 1: Margarida
  TpPlanta margarida;
  margarida.id = 1;
  strcpy(margarida.nome, "Margarida");
  strcpy(margarida.tipo, "Flor");
  strcpy(margarida.faseCrescimento, "Curta");
  TpSolo soloMargarida = lerSolo(0x01A0);
  margarida.solo = soloMargarida;
  escreverPlanta(0x0400, margarida);

  // Exemplo 2: Girassol
  TpPlanta girassol;
  girassol.id = 2;
  strcpy(girassol.nome, "Girassol");
  strcpy(girassol.tipo, "Flor");
  strcpy(girassol.faseCrescimento, "Media");
  TpSolo soloGirassol = lerSolo(0x01F0);
  girassol.solo = soloGirassol;
  escreverPlanta(0x0550, girassol);


  // Exemplo 3: Tulipa
  TpPlanta tulipa;
  tulipa.id = 3;
  strcpy(tulipa.nome, "Tulipa");
  strcpy(tulipa.tipo, "Flor");
  strcpy(tulipa.faseCrescimento, "Media");
  TpSolo soloTulipa = lerSolo(0x0240);
  tulipa.solo = soloTulipa;
  escreverPlanta(0x06A0, tulipa);


  // Exemplo 4: Carvalho
  TpPlanta carvalho;
  carvalho.id = 4;
  strcpy(carvalho.nome, "Carvalho");
  strcpy(carvalho.tipo, "Árvore");
  strcpy(carvalho.faseCrescimento, "Lenta");
  TpSolo soloCarvalho = lerSolo(0x01A0);
  carvalho.solo = soloCarvalho;
  escreverPlanta(0x07F0, carvalho);

  // Exemplo 5: Alface
  TpPlanta planta2;
  planta2.id = 5;
  strcpy(planta2.nome, "Alface");
  strcpy(planta2.tipo, "Hortaliça");
  strcpy(planta2.faseCrescimento, "Curta");
  TpSolo solo2 = lerSolo(0x0150);
  planta2.solo = solo2;
  escreverPlanta(0x0940, planta2);
  
}

// Potenciometro
int pinPot = A4;
int potValor = 0;

//Reservatorio
int pinLedReservatorio = 13;

Servo motor;
int contrast = 75;
int pos;

int isOpen = 0;
int isIrrigando = 0;
int SensChuva = A0;
int SensUmid1 = A1;
int umid1 = 0;
int SensUmid2 = A2;
int umid2 = 0;
int SensUmid3 = A3;
int umid3 = 0;
int enderecoEEPROM = 0x0400;
int enderecoMax = 0x0940;
int enderecoMin = 0x0400;

void setup () {
  Serial.begin (9600);
  pinMode(SensUmid1, INPUT);
  armazenarPlantas();
  analogWrite(6, contrast);
  LCD.begin(16, 2);
  digitalWrite(pinLedReservatorio, LOW);

  TpPlanta planta = lerPlanta(enderecoEEPROM);
  exibirLCD("Planta: ",planta.nome);
  
  motor.attach(23);
}

void moverMotor(){
  int chuva = analogRead(SensChuva);
  int range = floor(map(chuva, 0, 1024, 0, 2));
  
  switch (range) {
    case 0:       
      for (pos = 0; pos <= 180 && isOpen == 0; pos++) {
        if (pos == 180) {
          isOpen = 1;
        }
        motor.write(pos);
        delay(15);
      }
      break;
      
    case 1:
      for (pos = 180; pos >= 0 && isOpen == 1; pos--) {
        if (pos == 0) {
          isOpen = 0;
        }
        motor.write(pos);
        delay(15);
      }
      break;
   }
   delay(400);  
}

void exibirLCD(const char info[], const char info2[]) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(info);
    LCD.setCursor(0, 1);
    LCD.print(info2);

    delay(1000);
}

void irrigar(){
  TpPlanta planta = lerPlanta(enderecoEEPROM);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Irrigando: ");
  LCD.setCursor(0, 1);
  LCD.print(planta.nome);
}

void handleTecla(char key){
  if (key) {
    if(key == '1' && !isIrrigando){
      if(enderecoEEPROM > 0x0400) {
        enderecoEEPROM -= 0x0150;
      }
    } else if (key == '2' && !isIrrigando){
      if(enderecoEEPROM < 0x0940) {
          enderecoEEPROM += 0x0150;
      }
    } else if (key == '3') {
      isIrrigando = 1;
      irrigar();
    } else {
      Serial.println("Tecla Inválida");
    }
     TpPlanta planta = lerPlanta(enderecoEEPROM);
     exibirLCD("Planta: ",planta.nome);
     Serial.println(enderecoEEPROM);
     Serial.println(planta.nome);
  }
}

void ligarLedParaAbastecerReservatorio() {
  if(potValor <= 400) {
    digitalWrite(pinLedReservatorio, HIGH);
  }
  else {
    digitalWrite(pinLedReservatorio, LOW);
  }
  delay(500);
}

void loop () {  
  int umidPlanta2 = analogRead(SensUmid2);
  int umidPlanta3 = analogRead(SensUmid3);

  potValor = analogRead(pinPot);
  //Serial.println(potValor);

  umid1 = analogRead(SensUmid1);
  umid2 = analogRead(SensUmid2);
  umid3 = analogRead(SensUmid3);
  Serial.print("1: ");
  Serial.println(umid1);
  Serial.print("2: ");
  Serial.println(umid2);
  Serial.print("3: ");
  Serial.println(umid3);

  
  ligarLedParaAbastecerReservatorio();
  moverMotor();

//  char tecla = lerTecla();
//  handleTecla(tecla);
  
}