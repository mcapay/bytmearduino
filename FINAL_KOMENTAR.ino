#include <IRremote.h> // Knižnica pre IR
#include <LiquidCrystal_I2C.h>

#include <IRremote.h> // Knižnica pre IR
#include <LiquidCrystal_I2C.h>

/*
 * --- Nastavenie základných komponentov a premenných ---
 *
 * daj              - objekt pre ovládanie LCD displeja cez I2C (adresa 0x27, rozmery 16x2)
 * ledPins[8]       - pole pinov, na ktoré sú pripojené LED diódy (piny 3 až 10)
 * IR_RECEIVE_PIN   - pin (12) pre IR prijímač, ktorý prijíma signály z diaľkového ovládača
 * tlacidloPin      - pin (11) pre fyzické tlačidlo, ktorým hráč spustí hru
 * buzzerPin        - pin (2) pre piezo bzučiak na zvukovú spätnú väzbu
 * stav             - stavový indikátor hry (0 = čaká sa na stlačenie tlačidla, 1 = hráč háda)
 * receivedNumber   - reťazec, do ktorého sa ukladá číslo zadané cez IR ovládač
 * number           - náhodne vygenerované číslo, ktoré má hráč uhádnuť
 */
LiquidCrystal_I2C daj(0x27, 16, 2); 
int ledPins[8] = {3, 4, 5, 6, 7, 8, 9, 10}; 
int IR_RECEIVE_PIN = 12;  
int tlacidloPin = 11;     
int buzzerPin = 2;        
int stav = 0;             
String receivedNumber = "";
int number = 0;


void setup() {
  /*
   * Funkcia setup()
   * ----------------
   * Inicializačná funkcia, ktorá sa vykoná raz po zapnutí alebo resete mikrokontroléra.
   * V tejto fáze sa nastavia všetky vstupy/výstupy a spustia sa komponenty:
   *
   * - Spustí sa sériová komunikácia pre debug výstup (Serial monitor)
   * - Inicializuje sa IR prijímač na zadanom pine (IR_RECEIVE_PIN)
   * - Spustí sa LCD displej s podsvietením
   * - Nastaví sa tlačidlo ako vstup a bzučiak ako výstup
   * - Inicializuje sa generátor náhodných čísel na základe analógového vstupu A0
   * - Všetky LED diódy sa nastavia ako výstupy a vypnú sa
   * - Na displeji sa zobrazí úvodná hláška so správou pre používateľa
   */
  
  Serial.begin(9600);  // Spustenie sériovej komunikácie (rýchlosť 9600 baud)

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  // Inicializácia IR prijímača

  daj.init();       // Inicializácia LCD displeja
  daj.backlight();  // Zapnutie podsvietenia LCD displeja

  pinMode(tlacidloPin, INPUT);     // Nastavenie pinu pre tlačidlo ako vstup
  pinMode(buzzerPin, OUTPUT);      // Nastavenie pinu pre bzučiak ako výstup

  randomSeed(analogRead(A0));      // Inicializácia generátora náhodných čísel (na základe šumu z analógového pinu)

  nastavLED(); // Inicializácia LED pinov (všetky výstupy, nastavené na LOW)

  zobraz("Stlac tlacidlo", "", "a hra sa spusti", ""); // Úvodné zobrazenie textu na displeji
}


void loop() {
  /*
   * Funkcia loop()
   * ----------------
   * Hlavná slučka programu, ktorá sa opakovane vykonáva.
   * Obsahuje dve hlavné časti:
   *
   * 1. Čakanie na stlačenie tlačidla:
   *    - Ak je hra v stave 0 (neprebieha), a hráč stlačí tlačidlo,
   *      vygeneruje sa náhodné číslo (0–255) a zobrazí sa na LED diódach.
   *    - Stav sa prepne na 1, čím sa umožní zadávanie cez IR ovládač.
   *
   * 2. Spracovanie IR signálov (ak je stav == 1):
   *    - Prijímajú sa signály z diaľkového ovládača.
   *    - Podľa prijatého kódu sa pridá príslušná číslica do `receivedNumber`.
   *    - Ak používateľ stlačí tlačidlo EQ (kód 0xF609FF00), číslo sa porovná s vygenerovaným:
   *         - Ak je správne, zaznie jeden tón a zobrazí sa smajlík.
   *         - Ak je nesprávne, zaznejú dva tóny a zobrazí sa smutný smajlík.
   *    - Po 5 sekundách sa všetko resetuje: LED sa vypnú, displej sa vyčistí a hra sa môže začať znova.
   */

  int stavTlacidla = digitalRead(tlacidloPin);
  
  // Čaká sa na stlačenie tlačidla
  if (stavTlacidla == HIGH && stav == 0) {                     
    zobraz("Cislo", "", "vygenerovane", "");    
    delay(1000);

    //number = 33;

    number = random(0, 256); // Vygeneruj náhodné číslo 0 - 255 (8 bitov, lebo 8 LED)

    zobrazNaLED(number); // Rozsvieti LED podľa binárnej reprezentácie čísla

    stav = 1; // Zamedzenie opakovanému stlačeniu, na rade je hádanie
  }

  if (stav == 1) {
    if (IrReceiver.decode()) { // Ak obdrží signál z ovládača
      unsigned long receivedCode = IrReceiver.decodedIRData.decodedRawData;
      Serial.print("Received HEX Code: ");
      Serial.println(receivedCode, HEX);
      Serial.println(receivedNumber);

      switch (receivedCode) {
        case 0xF30CFF00: receivedNumber += "1"; break;
        case 0xE718FF00: receivedNumber += "2"; break;
        case 0xA15EFF00: receivedNumber += "3"; break;
        case 0xF708FF00: receivedNumber += "4"; break;
        case 0xE31CFF00: receivedNumber += "5"; break;
        case 0xA55AFF00: receivedNumber += "6"; break;
        case 0xBD42FF00: receivedNumber += "7"; break;
        case 0xAD52FF00: receivedNumber += "8"; break;
        case 0xAB54AFF00: receivedNumber += "9"; break;
        case 0xE916FF00: receivedNumber += "0"; break;
        case 0xF609FF00: // Tlačidlo EQ ukončenie zadávania
            zobraz("Moje:", String(number), "Tvoje:", receivedNumber);

            // Porovnanie s vygenerovaným číslom
            if (receivedNumber.toInt() == number) {
              daj.print(" :)");
              tone(buzzerPin, 3000); 
              delay(1000);            
              noTone(buzzerPin);    
              } else {
              daj.print(" :(");
              tone(buzzerPin, 4000);  
              delay(1000);           
              tone(buzzerPin, 3000);  
              delay(1000);           
              noTone(buzzerPin);   
              }

              delay(5000); // Pauza na zobrazenie výsledku
              stav = 0; // Reset stavu, čaká sa na tlačidlo
              receivedNumber = "";
              nastavLED();
              zobraz("Stlac tlacidlo", "", "a hra sa spusti", "");
              break;

      }
      
      if (stav == 1) {
        zobraz("", receivedNumber, "", ""); // Priebežné zobrazovanie čísla
      }
      
      delay(250);  
      IrReceiver.resume(); // Reset prijímača pre ďalší kód
      IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  // Štart prijímača
    } 
  }
}


/*
 * Funkcia nastavLED()
 * --------------------
 * Inicializuje všetky LED piny v poli `ledPins` ako výstupy a nastaví ich do LOW stavu (vypnuté).
 * Nepoužíva žiadne vstupné parametre.
 */
void nastavLED() {
    for (int i = 0; i < 8; i++) {
      pinMode(ledPins[i], OUTPUT);
      digitalWrite(ledPins[i], LOW); //  vypnuté
    }
  }

/*
 * Funkcia zobrazNaLED(int cislo)
 * ------------------------------
 * Zobrazí zadané číslo `cislo` na LED diódy podľa jeho binárnej reprezentácie.
 * Najnižší bit je priradený prvej LED (na indexe 0).
 *
 * Parametre:
 *   cislo - celé číslo, ktoré sa má zobraziť binárne na LED diódach.
 */
void zobrazNaLED(int cislo) {
  Serial.print("Binarny kod: ");

  int temp = cislo;
  for (int i = 0; i < 8; i++) {
     int bitValue = temp % 2;        // Zvyšok po delení 2 = posledný bit
     digitalWrite(ledPins[i], bitValue);
     Serial.print(bitValue);
     temp = temp / 2;                // Posuň číslo doprava (zahoď spracovaný bit)
  }

  Serial.println(); // Nový riadok v Serial Monitori
}

/*
 * Funkcia zobraz(String text1, String hodnota1, String text2, String hodnota2)
 * -----------------------------------------------------------------------------
 * Zobrazí dva riadky textu na LCD displeji s možnosťou dynamického vkladania hodnôt.
 *
 * Parametre:
 *   text1 - text pre prvý riadok pred hodnotou (napr. "Moje:")
 *   hodnota1 - hodnota, ktorá sa zobrazí za text1 (napr. číslo)
 *   text2 - text pre druhý riadok pred hodnotou
 *   hodnota2 - hodnota, ktorá sa zobrazí za text2
 */
void zobraz(String text1, String hodnota1, String text2, String hodnota2) {
  daj.clear(); // Vymaž displej
  daj.setCursor(0, 0); 
  daj.print(text1 + " " + hodnota1); 
  daj.setCursor(0, 1);
  daj.print(text2 + " " + hodnota2); 
}
