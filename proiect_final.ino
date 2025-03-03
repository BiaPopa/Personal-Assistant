// modulul BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// e-paper display
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"
// bluetooth connection
#include <BluetoothSerial.h>
// buzzer
#include "pitches.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define echoPin 14 
#define trigPin 0
#define photoresistor 15
#define buzzer 2

BluetoothSerial serialBT;
Adafruit_BME280 bme; // I2C

volatile bool nextState = false;
volatile bool prevState = false;
volatile bool powerState = false;
volatile bool darkmode = false;
volatile bool activeHC = false;

// buzzer
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// quotes
char* quotes[] = {
  "I can stand it then. Tomorrow, I'll think of some way to get him back. After all, tomorrow is another day.",
  "There is some good in this world, and it’s worth fighting for.",
  "It is only with the heart that one can see rightly; what is essential is invisible to the eye.",
  "The only way out of the labyrinth of suffering is to forgive.",
  "And so we beat on, boats against the current, borne back ceaselessly into the past.",
  "Whatever our souls are made of, his and mine are the same.",
  "I am not afraid of storms, for I am learning how to sail my ship.",
  "All happy families are alike; each unhappy family is unhappy in its own way.",
  "Memories warm you up from the inside. But they also tear you apart.",
  "It is nothing to die; it is dreadful not to live."
};

char* authors[] = {
  "Margaret Mitchell",
  "J.R.R. Tolkien",
  "Antoine de Saint-Exupéry",
  "John Green",
  "F. Scott Fitzgerald",
  "Emily Brontë",
  "Louisa May Alcott",
  "Leo Tolstoy",
  "Haruki Murakami",
  "Victor Hugo"
};

char *books[] = {
  "Gone with the Wind",
  "The Two Towers",
  "The Little Prince",
  "Looking for Alaska",
  "The Great Gatsby",
  "Wuthering Heights",
  "Little Women",
  "Anna Karenina",
  "Kafka on the Shore",
  "Les Misérables"
};

void setup()
{
  Serial.begin(115200);
  serialBT.begin("Esp32-BT");

  // e-paper display
  display.init(115200, true, 2, false); 
  display.hibernate();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  // next button
  pinMode(25, INPUT_PULLUP); // configureaza pinul butonului ca intrerupere cu rezistenta pull-up
  attachInterrupt(digitalPinToInterrupt(25), ISR_changeNextPage, RISING); // configureaza o intrerupere pentru pinul butonului

  // prev button
  pinMode(26, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(26), ISR_changePrevPage, RISING);

  // power button
  pinMode(12, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(12), ISR_power, RISING);

  // modul bme
  bool status;

  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor!");
    while(1);
  }

  // senzor ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

unsigned long lastMeasurementTime = 2 * 60 * 1000;
// refresh date de la bme280 odata la 2 minute
const unsigned long measurementInterval = 2 * 60 * 1000;
// refresh quotes odata la 5 minute
const unsigned long measurementIntervalQuotes = 5 * 60 * 1000;

// numarul total de pagini
const int PAGE_COUNT = 3;
// pagina curenta
int currentPage = 1;
// pagina a fost deja afisata
bool alreadyDisplayed = false;

char chr;
int indexRow = 0;
// lista de task-uri
char cmd[10][30];
// numarul de task-uri
int indexList = 0;

long pulseWidth;
int distance;
long light;
// citatul curent
int quote = -1;

void loop() {
  long light = analogRead(photoresistor); //ADC

  if (light < 500 && darkmode == false) {
    // trecerea in dark mode
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    
    if (currentPage == 1) {
      printValuesPage1();
    } else if (currentPage == 2) {
      printValuesPage2();
    } else if (currentPage == 3) {
      printValuesPage3();
    }

    darkmode = true;
  } else if (light >= 500 && darkmode == true) {
    // trecerea in light mode
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    if (currentPage == 1) {
      printValuesPage1();
    } else if (currentPage == 2) {
      printValuesPage2();
    } else if (currentPage == 3) {
      printValuesPage3();
    }

    darkmode = false;
  }

  delay(1000);

  if (nextState) {
    // trecerea la urmatoarea pagina
    delay(100);

    if (digitalRead(25)) {
      currentPage++;

      if (currentPage > PAGE_COUNT) {
        currentPage = 1; // daca este la ultima pagina, revine la prima
      }

      // reseteaza starea de afisare pentru noua pagina
      alreadyDisplayed = false; 

      if (currentPage == 1) {
        lastMeasurementTime = 2 * 60 * 1000;
      } else if (currentPage == 2) {
        lastMeasurementTime = 5 * 60 * 1000;
      } else if (currentPage == 3) {
        lastMeasurementTime = 0;
      }

      nextState = false;
    }
  } 
  
  if (prevState) {
    // trecerea la pagina anterioara
    delay(100);

    if (digitalRead(26)) {
      currentPage--;

      if (currentPage < 1) {
        currentPage = PAGE_COUNT; // daca este la prima pagina, revine la ultima
      }

      // reseteaza starea de afisare pentru noua pagina
      alreadyDisplayed = false;
      
      if (currentPage == 1) {
        lastMeasurementTime = 2 * 60 * 1000;
      } else if (currentPage == 2) {
        lastMeasurementTime = 5 * 60 * 1000;
      } else if (currentPage == 3) {
        lastMeasurementTime = 0;
      }

      prevState = false;
    }
  }

  if (powerState) {
    // oprirea/pornirea display-ului
    delay(100);
    
    if (digitalRead(12)) {
      if (currentPage > 0) {
        currentPage = 0;
      } else {
        currentPage = 1;
      }
      
      // reseteaza starea de afisare pentru noua pagina
      alreadyDisplayed = false;
      
      if (currentPage == 1) {
        lastMeasurementTime = 2 * 60 * 1000;
      } else if (currentPage == 2) {
        lastMeasurementTime = 5 * 60 * 1000;
      } else if (currentPage == 3) {
        lastMeasurementTime = 0;
      }

      powerState = false;
    }
  } 

  if (currentPage == 1) {
    // afisarea paginii 1
    unsigned long currentTime = millis();

    // se verifica daca a trecut intervalul de timp pentru a face o noua masurare
    if (currentTime - lastMeasurementTime >= measurementInterval) {
      lastMeasurementTime = currentTime;

      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }

      printValuesPage1();
    }
  } else if (currentPage == 2) {
    // afisarea paginii 2
    unsigned long currentTime = millis();
    
    // citirea datelor de la ultrasonic
    pinMode(trigPin, OUTPUT);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    pinMode(echoPin, INPUT);
    pulseWidth = pulseIn(echoPin, HIGH);
    distance = pulseWidth / 29 / 2;

    // Serial.print("Pulse Width: ");
    // Serial.println(pulseWidth);
    // Serial.print("Distance: ");
    // Serial.println(distance);

    if (distance < 20 && activeHC == false) {
      // afisarea mesajului de salut pe display
      activeHC = true;

      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }

      printValuesPage2();
    } else if (distance >= 20 && activeHC == true) {
      // stergerea mesajului de salut de pe display
      activeHC = false;

      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }

      printValuesPage2();
    }

    delay(1000);

    // se verifica daca a trecut intervalul de timp 
    if (currentTime - lastMeasurementTime >= measurementIntervalQuotes) {
      lastMeasurementTime = currentTime;
      // schimbarea citatului
      quote++;

      if (quote > 9) {
        quote = 0;
      }

      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }

      printValuesPage2();
    }
  } else if (currentPage == 0) {
    // afisarea paginii de oprire
    if (!alreadyDisplayed) {
      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }
      
      turnOffDisplay();
      alreadyDisplayed = true;
    }
  } else if (currentPage == 3) {
    // afisarea paginii 3
    if (!alreadyDisplayed) {
      if (darkmode == false) {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);
      } else {
        display.fillScreen(GxEPD_BLACK);
        display.setTextColor(GxEPD_WHITE);
      }
      
      printValuesPage3();
      alreadyDisplayed = true;
    }

    if (serialBT.available()) {
      // citirea datelor primite de la telefon
      chr = serialBT.read();

      if (chr == '\n') {
        // s-a primit un task complet, se afiseaza pe pagina
        indexList++;
        indexRow = 0;

        if (darkmode == false) {
          display.fillScreen(GxEPD_WHITE);
          display.setTextColor(GxEPD_BLACK);
        } else {
          display.fillScreen(GxEPD_BLACK);
          display.setTextColor(GxEPD_WHITE);
        }

        printValuesPage3();

        // buzzer activ
        for (int thisNote = 0; thisNote < 8; thisNote++) {
          int noteDuration = 1000 / noteDurations[thisNote];
          tone(buzzer, melody[thisNote], noteDuration);

          int pauseBetweenNotes = noteDuration * 1.30;
          delay(pauseBetweenNotes);
          noTone(buzzer);
        }

        delay(1000);
      } else if (chr == '#') {
        // se sterge un task de pe pagina
        chr = serialBT.read();
        int indexRm = chr - '0' - 1;
        chr = serialBT.read();
        chr = serialBT.read();

        // stergerea task-ului din lista
        for (int i = indexRm; i < indexList - 1; i++) {
          strcpy(cmd[i], cmd[i + 1]);
        }
        indexList--;
        memset(cmd[indexList], 0, sizeof(cmd[indexList]));

        // reafisarea paginii
        if (darkmode == false) {
          display.fillScreen(GxEPD_WHITE);
          display.setTextColor(GxEPD_BLACK);
        } else {
          display.fillScreen(GxEPD_BLACK);
          display.setTextColor(GxEPD_WHITE);
        }

        printValuesPage3();

        // buzzer activ
        for (int thisNote = 0; thisNote < 8; thisNote++) {
          int noteDuration = 1000 / noteDurations[thisNote];
          tone(buzzer, melody[thisNote], noteDuration);

          int pauseBetweenNotes = noteDuration * 1.30;
          delay(pauseBetweenNotes);
          noTone(buzzer);
        }

        delay(1000);
      } else {
        cmd[indexList][indexRow] = chr;
        indexRow++;
      }
    }
  }
};

void printValuesPage1() {
  display.setFont(&FreeMonoBold9pt7b);
  display.setRotation(0);
  display.setTextSize(1);

  display.setCursor(10, 30);
  const char room_data[] = "Here are some informations about your room.";
  display.println(room_data);

  display.setCursor(10, 70);
  const char enjoy[] = "Have a nice day!";
  display.println(enjoy);

  display.setTextSize(2);

  display.setCursor(200, 180);
  display.print("Temperature: ");
  display.print(bme.readTemperature(), 2); // 2 zecimale
  display.println("*C");

  display.setCursor(200, 220);
  display.print("Humidity: ");
  display.print(bme.readHumidity(), 2);
  display.println("%");

  display.setCursor(200, 260);
  display.print("Pressure: ");
  display.print(bme.readPressure() / 100.0F, 2);
  display.println(" Pa");

  display.setCursor(200, 300);
  display.print("Altitude: ");
  display.print(bme.readAltitude(SEALEVELPRESSURE_HPA), 2);
  display.println("m");

  // false pentru a face actualizarea fara a astepta
  display.display(false); 
}

void printValuesPage2() {
  display.setFont(&FreeMonoBold9pt7b);
  display.setRotation(0);
  display.setTextSize(2);

  display.setCursor(10, 50);
  const char buna[] = "Welcome home, Bia! I've missed you.";
  if (activeHC) {
    display.print(buna);
  }

  display.setTextSize(1);
  display.setCursor(60, 200);
  display.print(quotes[quote]);

  display.setCursor(430, 300);
  display.print(authors[quote]);

  display.setCursor(430, 350);
  display.print(books[quote]);

  display.display(false);
}

void printValuesPage3() {
  display.setFont(&FreeMonoBold9pt7b);
  display.setRotation(0);
  display.setTextSize(2);

  display.setCursor(10, 50);
  const char list[] = "TODO list";
  display.print(list);

  int x = 40;
  int y = 100;
  Serial.println(indexList);

  for (int i = 0; i < indexList; i++) {
    display.setCursor(x, y);
    display.print("- ");
    display.print(cmd[i]);
    x = 40;
    y += 50;
  }

  display.display(false);
}

void turnOffDisplay() {
  display.setFont(&FreeMonoBold9pt7b);
  display.setRotation(0);
  display.setTextSize(2);

  int16_t tbx, tby; uint16_t tbw, tbh;
  const char buna[] = "Goodbye, Bianca!";

  display.getTextBounds(buna, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setCursor(x, y);
  
  display.print(buna);
  display.display(false);

  delay(3000);

  display.fillScreen(GxEPD_BLACK);
  display.setFullWindow();
  display.display(false);
}

void ISR_changeNextPage() {
  if (!nextState)
    nextState = true;
}

void ISR_changePrevPage() {
  if (!prevState)
    prevState = true;
}

void ISR_power() {
  if (!powerState)
    powerState = true;
}