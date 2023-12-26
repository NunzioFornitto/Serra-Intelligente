#include <DHT.h>
#include <Servo.h>
#include <LiquidCrystal.h>

enum StatoSerra {
  Riposo,
  AperturaFalde,
  ChiusuraFalde,
  Irrigazione
};

StatoSerra statoAttuale = Riposo;
StatoSerra statoPrecedente = Riposo;

const int dhtPin = 5;                 // Pin per il sensore DHT11 (con dati)
const int soilHumidityPin = A0;       // Pin per il sensore di umidità del terreno
const int servoAperturaFaldaPin = 2;  // Pin per il servo motore apertura falda
const int servoChiusuraFaldaPin = 3; // Pin per il servo motore chiusura falda
const int pompaPin = 4;               // Pin per il controllo della pompa di irrigazione
const int rsPin = 12;                 // Pin RS del display
const int enablePin = 11;             // Pin Enable del display
const int d4Pin = 6;                  // Pin D4 del display
const int d5Pin = 7;                  // Pin D5 del display
const int d6Pin = 9;                  // Pin D6 del display
const int d7Pin = 8;                  // Pin D7 del display

int Contrast = 100;                    // Contrasto lcd

// Definizione degli oggetti
DHT dht(dhtPin, DHT11);                 // Inizializzazione del sensore DHT11
float temperatura;
Servo servoAperturaFalda;
Servo servoChiusuraFalda;
LiquidCrystal lcd(rsPin, enablePin, d4Pin, d5Pin, d6Pin, d7Pin);

const int sogliaTemperaturaApertura = 25;          // Soglia di temperatura per l'apertura delle falde
const int sogliaUmiditaMinima = 50;               // Soglia di umidità nel terreno per l'attivazione dell'irrigazione
const int incrementoUmiditaNecessario = 10;       // Incremento minimo richiesto durante l'irrigazione
int percentualeUmidita;

void setup() {
  // Inizializzazione dei pin
  servoAperturaFalda.attach(servoAperturaFaldaPin);
  servoChiusuraFalda.attach(servoChiusuraFaldaPin);
  pinMode(pompaPin, OUTPUT);

  // Inizializzazione del sensore DHT11
  dht.begin();

  analogWrite(15, Contrast);
  lcd.begin(16, 2);

  //Porta seriale per debug
  Serial.begin(9600);

  // Inizializzazione del display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Inizializzazione...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Lettura dei valori dai sensori DHT11
  temperatura = dht.readTemperature();
  int umiditaTerreno = analogRead(soilHumidityPin);

  // Mappo valore letto dal sensore di umidità nel terreno (0-1023) a un intervallo percentuale (0-100)
  percentualeUmidita = map(umiditaTerreno, 0, 1023, 0, 100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tem:" + String(temperatura) + "C ");

  lcd.setCursor(0, 1);
  lcd.print("Um" + String(percentualeUmidita) + "%");

  // Controllo delle condizioni per l'apertura e chiusura delle falde
  switch (statoAttuale) {
    case Riposo:
      if (temperatura > sogliaTemperaturaApertura && statoPrecedente != AperturaFalde) {
        apriFalde();
        statoPrecedente = statoAttuale;
        statoAttuale = AperturaFalde;
      } else if (temperatura <= sogliaTemperaturaApertura && statoPrecedente != ChiusuraFalde) {
        chiudiFalde();
        statoPrecedente = statoAttuale;
        statoAttuale = ChiusuraFalde;
      }
      break;

    case AperturaFalde:
      // Mantengo le falde aperte finché la temperatura è al di sopra della soglia
      if (temperatura <= sogliaTemperaturaApertura) {
        statoPrecedente = statoAttuale;
        statoAttuale = Riposo;
      }
      lcd.clear();
      lcd.print("Serra Aperta");
      lcd.setCursor(0, 1);
      lcd.print("Tem:" + String(temperatura) + "C Um" + String(percentualeUmidita) + "%");
      Serial.println("Sistema in stato di riposo");
      break;

    case ChiusuraFalde:
      // Mantengo le falde chiuse finché la temperatura è al di sotto della soglia
      if (temperatura > sogliaTemperaturaApertura) {
        statoPrecedente = statoAttuale;
        statoAttuale = Riposo;
      }
      lcd.clear();
      lcd.print("Serra Chiusa");
      lcd.setCursor(0, 1);
      lcd.print("Tem:" + String(temperatura) + "C Um" + String(percentualeUmidita) + "%");
      Serial.println("Sistema in stato di riposo");
      break;
  }

  // Controllo dell'irrigazione
  if (percentualeUmidita < sogliaUmiditaMinima) {
    irrigaTerreno();
  }
  delay(1000);
}

//Attiva disattiva irrigazione
void irrigaTerreno() {
  digitalWrite(pompaPin, HIGH);
  delay(2000);
  digitalWrite(pompaPin, LOW);
}

//Apertura falde 100°
void apriFalde() {
  int angoloApertura = 100;
  servoAperturaFalda.write(angoloApertura);
  servoChiusuraFalda.write(angoloApertura);
}

//Chiudi Falde
void chiudiFalde() {
  int angoloChiusura = 0;
  servoAperturaFalda.write(angoloChiusura);
  servoChiusuraFalda.write(angoloChiusura);
}

