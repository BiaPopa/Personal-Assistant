# Personal-Assistant

Acest proiect constă în dezvoltarea unui **asistent personal** conectat prin Bluetooth la un smartphone, capabil să afișeze diverse informații și să includă un sistem de alarmă. Dispozitivul detectează prezența utilizatorului și afișează informații despre starea camerei pe un ecran E-Paper, permițând navigarea prin diferite pagini utilizând butoane fizice. De asemenea, asistentul trece automat în modul dark mode în condiții de iluminare scăzută.

## Caracteristici principale

- **Afișare informații ambientale:** Temperatură, umiditate, presiune și altitudine, actualizate la fiecare două minute.
- **Detectare prezență utilizator:** Utilizează un senzor ultrasonic pentru a detecta prezența și a afișa mesaje corespunzătoare.
- **Listă de sarcini (TODO list):** Permite gestionarea sarcinilor prin intermediul conexiunii Bluetooth, cu notificări audio la adăugarea sau ștergerea acestora.
- **Mod de afișare adaptiv:** Trece automat în modul dark mode în funcție de nivelul de iluminare ambientală detectat de un fotorezistor.

## Componente utilizate

- FireBeetle 2 ESP32-E
- 7.5 inch E-Paper display
- BME280 (senzor temperatură, presiune, umiditate)
- senzor ultrasonic HC-SR04
- fotorezistor
- mini difuzor
- rezistențe 1K, 10K, 270K
- tranzistor BD139
- condensator 1000uF
- 2 push buttons
- on/off switch
- 5000 mAh LiPo battery
- alimentator 5V

## Design hardware

Dispozitivul este construit în jurul microcontrollerului **ESP32**, la care sunt conectate componentele menționate anterior. Un PCB personalizat a fost realizat pentru a facilita conexiunile și a asigura o structură compactă. Circuitul include linii principale pentru 3V3 și GND, la care sunt conectate componentele. Fotorezistorul și difuzorul sunt integrate în circuit, difuzorul fiind controlat printr-un amplificator realizat cu tranzistorul BD139.

## Design software

Dezvoltarea software a fost realizată utilizând **Arduino IDE**, cu următoarele biblioteci:

- Pentru senzorul BME280: `Adafruit_Sensor.h` și `Adafruit_BME280.h`
- Pentru display-ul E-Paper: `GxEPD2_BW.h`, `GxEPD2_display_selection_new_style.h` și `Fonts/FreeMonoBold9pt7b.h`
- Pentru conexiunea Bluetooth: `BluetoothSerial.h`
- Pentru difuzor: `pitches.h`

Funcționalitatea dispozitivului este organizată pe trei pagini de afișare:

1. **Pagina 1:** Afișează informațiile de la senzorul BME280 (temperatură, umiditate, presiune, altitudine), actualizate la fiecare două minute.
2. **Pagina 2:** Detectează prezența utilizatorului prin senzorul ultrasonic și afișează citate actualizate la fiecare cinci minute.
3. **Pagina 3:** Listă de sarcini (TODO) gestionabilă prin conexiunea Bluetooth, cu notificări audio la adăugarea sau ștergerea sarcinilor.

Navigarea între pagini se realizează prin butoanele "prev" și "next", iar trecerea în modul dark mode este controlată automat de fotorezistor în funcție de luminozitatea ambientală.

## Instrucțiuni de utilizare

1. **Pornirea dispozitivului:** Asigurați-vă că bateria este încărcată și utilizați comutatorul on/off pentru a porni asistentul personal.
2. **Navigare între pagini:** Utilizați butoanele "prev" și "next" pentru a naviga între paginile de afișare.
3. **Conexiune Bluetooth:** Asociați dispozitivul cu smartphone-ul utilizând o aplicație de tip Serial Bluetooth pentru a gestiona lista de sarcini.
4. **Modul dark mode:** Dispozitivul va trece automat în modul dark mode în condiții de iluminare scăzută, fără intervenția utilizatorului.
