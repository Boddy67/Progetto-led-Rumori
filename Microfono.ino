#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

//#define LOGARITMIC
const int micPin = A2;
const int ledPin = LED_BUILTIN;
const int sampleWindow = 50; 
// Parametri di calibrazione
int sogliaSilenzio = 50;        // Valore minimo per ignorare il rumore di fondo
int sensibilita = 200;          // Oltre questo valore il LED si accende al massimo
int reference = 100;          // Oltre questo valore il LED si accende al massimo
float logVal;

// Struttura per gestire i frame della matrice
uint8_t frame[8][12]; 

uint8_t immagineRumore[8][12] = {
  { 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1 }, 
  { 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1 },
  { 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1 }
};

uint8_t immagineMedio[8][12] = {
  { 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0 }, 
  { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0 }
};

uint8_t immagineSilenzio[8][12] = {
  { 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 }, 
  { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 }
};


void setup() {
  Serial.begin(115200);
  matrix.begin();
}

void loop() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // 1. Campionamento
  while (millis() - startMillis < sampleWindow) {
    int sample = analogRead(micPin);
    if (sample < 1024) {
      if (sample > signalMax) signalMax = sample;
      else if (sample < signalMin) signalMin = sample;
    }
  }

  unsigned int peakToPeak = signalMax - signalMin - reference;
  if(peakToPeak >5000)
    peakToPeak = 0;


  int intensitaLed = map(peakToPeak, sogliaSilenzio, sensibilita, 0, 255);
  intensitaLed = constrain(intensitaLed, 0, 255); // Evita valori fuori range

  analogWrite(ledPin, intensitaLed);




// immagineSilenzio , immagineMedio e immagineRumore sono due array 8×12 che rappresentano due immagini

if (peakToPeak < 80) {
    matrix.renderBitmap(immagineSilenzio, 8, 12);
}
else if (peakToPeak < 200) {
    matrix.renderBitmap(immagineMedio, 8, 12);
}
else {
    matrix.renderBitmap(immagineRumore, 8, 12);
}


 Serial.print("Segnale_Audio:");
  Serial.println(logVal);           //la fequenza in base a quanto veloce cambia immagine
  delay(100);



/* CODICE NON IN USO (CANCELLATO )

  // 2. Mappatura del valore su 96 LED
  // Regola il valore '400' in base alla sensibilità del microfono

#ifdef LOGARITMIC
// --- LOGICA LOGARITMICA ---                        Questa è la parte che trasforma il volume del microfono in un numero da 0 a 96 e a noi non ci serve
  int numLedsAccesi = 0;
  
  if (peakToPeak > 2) { // Evitiamo log10(0) che è matematicamente indefinito
    // Calcoliamo un valore logaritmico normalizzato
    // log10(peakToPeak) restituirà un valore tra circa 0.3 e 3.0 (se il max è 1024)
    logVal = log10((float)peakToPeak);
    
    // Mappiamo il logaritmo (da 0.5 a 2.8 ad esempio) sui 96 LED
    // Questi estremi vanno calibrati in base al microfono
    numLedsAccesi = map((logVal -2.2)* 3000, 0, 600, 0, 96); 
  }
#else
  int numLedsAccesi = map(peakToPeak, 20, 400, 0, 96);
  numLedsAccesi = constrain(numLedsAccesi, 0, 96);
#endif

/*                        ALTRA  PARTE DI CODICE CHE NON USIAMO
  // 3. Riempimento della matrice
  aggiornaMatrice(numLedsAccesi);
  
  // 4. Invio del frame alla matrice
  matrix.renderBitmap(frame, 8, 12);
  

}
/*                       ALTRA PARTE NON NECESSARIA questa è la funzione che costruisce la barra e non ci serve
void aggiornaMatrice(int ledTotali) {
  int contatore = 0;
  
  // Scansione per righe e colonne
  for (int r = 0; r < 12; r++) {
    for (int c = 0; c < 8; c++) {
      if (contatore < ledTotali) {
        frame[c][r] = 1; // Accendi LED
      } else {
        frame[c][r] = 0; // Spegni LED
      }
      contatore++;
    }
  }
}
*/