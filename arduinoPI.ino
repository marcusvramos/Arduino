#include <LiquidCrystal.h>
#include <Servo.h>
#include <math.h>
#include <EEPROM.h>
#include <SD.h>

struct Solo {
  char tipoDeSolo[20];
  char capacidadeDeRetencao[30];
};
typedef struct Solo TpSolo;

struct Planta {
  int id;
  char nome[52];
  char tipo[20];
  char faseCrescimento[20]; 
  int  umidadeIdeal;
  // germinação, crescimento vegetativo, floração, frutificação, Maturação
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
  margarida.umidadeIdeal = 2;  
  TpSolo soloMargarida = lerSolo(0x01A0);
  margarida.solo = soloMargarida;
  escreverPlanta(0x0400, margarida);

  // Exemplo 2: Girassol
  TpPlanta girassol;
  girassol.id = 2;
  strcpy(girassol.nome, "Girassol");
  strcpy(girassol.tipo, "Flor");
  strcpy(girassol.faseCrescimento, "Media");
  girassol.umidadeIdeal = 1; 
  TpSolo soloGirassol = lerSolo(0x01F0);
  girassol.solo = soloGirassol;
  escreverPlanta(0x0550, girassol);

  // Exemplo 3: Tulipa
  TpPlanta tulipa;
  tulipa.id = 3;
  strcpy(tulipa.nome, "Tulipa");
  strcpy(tulipa.tipo, "Flor");
  strcpy(tulipa.faseCrescimento, "Media");
  tulipa.umidadeIdeal = 2; 
  TpSolo soloTulipa = lerSolo(0x0240);
  tulipa.solo = soloTulipa;
  escreverPlanta(0x06A0, tulipa);

  // Exemplo 4: Carvalho
  TpPlanta carvalho;
  carvalho.id = 4;
  strcpy(carvalho.nome, "Carvalho");
  strcpy(carvalho.tipo, "Árvore");
  strcpy(carvalho.faseCrescimento, "Lenta");
  carvalho.umidadeIdeal = 4;  
  TpSolo soloCarvalho = lerSolo(0x01A0);
  carvalho.solo = soloCarvalho;
  escreverPlanta(0x07F0, carvalho);

  // Exemplo 5: Alface
  TpPlanta planta2;
  planta2.id = 5;
  strcpy(planta2.nome, "Alface");
  strcpy(planta2.tipo, "Hortaliça");
  strcpy(planta2.faseCrescimento, "Curta");
  planta2.umidadeIdeal = 3; 
  TpSolo solo2 = lerSolo(0x0150);
  planta2.solo = solo2;
  escreverPlanta(0x0940, planta2);
}

// Potenciometro
int pinPot = A4;
int potValor = 0;
TpPlanta plantaSelecionada;

Servo motor;
int contrast = 75;
int pos;

int isOpen = 1;
int change = 0;
int isIrrigando = 0;
int SensChuva = A0;
int SensUmid1 = A1;
int umidPlanta1 = 0;
int SensUmid2 = A2;
int umidPlanta2 = 0;
int SensUmid3 = A3;
int umidPlanta3 = 0;
int enderecoEEPROM = 0x0400;
int enderecoMax = 0x0940;
int enderecoMin = 0x0400;
const byte botaoAnt = 25;
const byte botaoProx = 27;
const byte botaoEnter = 29;

const int chipSelect = 53; // Pino do Arduino Mega ao qual o CS do módulo SD está conectado

File dataFile;

void moverMotor(int range){
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

void handleTampa(){
  int reservVazio = reservatorioVazio();
  int chovendo = isChovendo();
  if(chovendo && reservVazio){
    moverMotor(0);
  }
  else{
    moverMotor(1);
  }
}

int isChovendo(){
  int chuva = analogRead(SensChuva);
  int range = floor(map(chuva, 0, 1024, 0, 2));
  if(range > 0){
    return 0;
  } else {
    return 1;
  }
}


void clearLCD(){
    LCD.setCursor(0, 0);
    LCD.print("                ");
    LCD.setCursor(0, 1);
    LCD.print("                ");
    delay(500);
}

void exibirLCD(const char info[], const char info2[]) {
    LCD.setCursor(0, 0);
    LCD.print(info);
    LCD.setCursor(0, 1);
    LCD.print(info2);
    delay(500);
}


int necessitaIrrigar(int umidade) {
  Serial.print("Umidade atual: ");
  Serial.println(umidade);
  int nivelAtual = trunc(umidade / 310); // Dividindo a umidade por 310 para obter um valor entre 0 e 4
  nivelAtual = abs(nivelAtual - 4); // Tomando o valor absoluto da diferença entre nivelAtual e 4
  Serial.println(nivelAtual);
  Serial.println(plantaSelecionada.umidadeIdeal);
  if (nivelAtual < plantaSelecionada.umidadeIdeal) {
    return nivelAtual;
  }
  return 0;
}

int mudou(int info){
  int estado = change;
  change = info;
  if(estado == info){
    return 1;
  }
  else {
    return 0;
  }
}

void irrigar() {
  int valorEnter = LOW;
  isIrrigando = 1;
  int ultimo = -2;

  do{
    delay(500);
    Serial.println(enderecoEEPROM);
    Serial.println(plantaSelecionada.nome);
    umidPlanta1 = analogRead(SensUmid1);
    int nivel = necessitaIrrigar(umidPlanta1);
    char stringNivelAtual[15];
    char stringNivelIdeal[15];
    sprintf(stringNivelAtual, "Nivel atual: %d", nivel); // Formatando o valor de 'nivel' para string
    sprintf(stringNivelIdeal, "Nivel ideal: %d", plantaSelecionada.umidadeIdeal); // Formatando o valor de 'plantaSelecionada.umidadeIdeal' para string
    if (nivel) {
      if(ultimo != nivel){
        ultimo = nivel;
        clearLCD();
        exibirLCD(stringNivelAtual, stringNivelIdeal);
      }
    }
    else {
      if(ultimo != 0){
        clearLCD();
        exibirLCD("Monitorando: ", plantaSelecionada.nome);
        ultimo = 0;  
      }
    }
    valorEnter = digitalRead(botaoEnter);
  }while(valorEnter == LOW);
  Serial.println(enderecoEEPROM);
  isIrrigando = 0;
  // enderecoEEPROM = 0x0400;
  // TpPlanta plantaInicial = lerPlanta(enderecoEEPROM);
  // exibirLCD("Planta: ", plantaInicial.nome);
}

void handleTecla(char key){
  if (key) {
    if(key == '1'){
      if(enderecoEEPROM > 0x0400) {
        enderecoEEPROM -= 0x0150;
      }
    } else if (key == '2'){
      if(enderecoEEPROM < 0x0940) {
          enderecoEEPROM += 0x0150;
      }
    } else if (key == '3') {
      if(isIrrigando == 1) {
        Serial.println(enderecoEEPROM);
        Serial.println(plantaSelecionada.nome);
        isIrrigando = 0;
      } else {
        isIrrigando = 1;
        plantaSelecionada = lerPlanta(enderecoEEPROM);
        irrigar();
      }
    } else {
      Serial.println("Tecla Inválida");
    }
     TpPlanta planta = lerPlanta(enderecoEEPROM);
     clearLCD();
     exibirLCD("Planta: ",planta.nome);
  }
}

void tecla(){
  int valorAnt = digitalRead(botaoAnt);
  int valorProx = digitalRead(botaoProx);
  int valorEnter = digitalRead(botaoEnter);
  
  Serial.print("Valor ant: ");
  Serial.println(valorAnt);
  Serial.print("Valor prox: ");
  Serial.println(valorProx);
  Serial.print("Valor enter: ");
  Serial.println(valorEnter);
  
  if(valorAnt == HIGH){
    handleTecla('1');
  }
  
  if(valorProx == HIGH){
    handleTecla('2');
  }
  
  if(valorEnter == HIGH){
    handleTecla('3');
  }
  
  delay(100);
}

void setup () {
  Serial.begin(9600);
  pinMode(SensUmid1, INPUT);
  armazenarSolo();
  armazenarPlantas();
  SD.begin(chipSelect);
  
  analogWrite(6, contrast);
  LCD.begin(16, 2);
 
  dataFile = SD.open("dados.txt", FILE_WRITE);
  
  if (dataFile) {
    dataFile.println("Olá, isso é um arquivo de texto no cartão SD!");
    dataFile.close();
    Serial.println("Dados gravados com sucesso.");
  } else {
    Serial.println("Erro ao abrir o arquivo.");
  }
  
  TpPlanta planta = lerPlanta(enderecoEEPROM);
  Serial.println(planta.nome);
  exibirLCD("Planta: ",planta.nome);

  motor.attach(23);
  motor.write(180);
}

int reservatorioVazio(){
  if(potValor <= 800) {
    return 1;
  } 
  return 0;
}

void loop () {  
   umidPlanta1 = analogRead(SensUmid1);
   umidPlanta2 = analogRead(SensUmid2);
   umidPlanta3 = analogRead(SensUmid3);

   potValor = analogRead(pinPot);
  
   tecla();
   handleTampa();
  

  delay(500);
}