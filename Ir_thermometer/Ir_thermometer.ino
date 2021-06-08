/* Thermometre IR
   Réalisé par Xavier Morneau dans le cadre du cours de projet personnel
   Hiver 2021 - Technologie du génie physique
   Version 1.0.0

   ToDo:
   -AnalogRead pour Vbatt et pour T on.
   -Déterminer a quel tension lu de T on pour éteindre la diode laser.
   -Ajuster les seuils de bits pour chaque pourcentage de la batterie, ils sont approximatifs présentement
*/
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LowPower.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();


#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int EtatBat;
uint8_t rotation = 0;       // Flip the screen for left-handed or right-handed. Default = 0 / left-handed
uint32_t TempsPrecedent = 0;
uint32_t TempsPrecedent2 = 0;
uint32_t TempsPrecedent3 = 0;
double Tmin = 0;
double Tmax = 0;
double Tobjet;
const int buzzer = 4;
// '100', 20x10px
const unsigned char Batt100 [] PROGMEM = {
  0xff, 0xff, 0xc0, 0x80, 0x00, 0x70, 0xbf, 0xff, 0x10, 0xbf, 0xff, 0xd0, 0xbf, 0xff, 0xd0, 0xbf,
  0xff, 0xd0, 0xbf, 0xff, 0xd0, 0xbf, 0xff, 0x10, 0x80, 0x00, 0x70, 0xff, 0xff, 0xc0
};
// '75', 20x10px
const unsigned char Batt75 [] PROGMEM = {
  0xff, 0xff, 0xc0, 0x80, 0x00, 0x70, 0xbf, 0xfc, 0x10, 0xbf, 0xfc, 0x10, 0xbf, 0xfc, 0x10, 0xbf,
  0xfc, 0x10, 0xbf, 0xfc, 0x10, 0xbf, 0xfc, 0x10, 0x80, 0x00, 0x70, 0xff, 0xff, 0xc0
};
// '50', 20x10px
const unsigned char Batt50 [] PROGMEM = {
  0xff, 0xff, 0xc0, 0x80, 0x00, 0x70, 0xbf, 0xc0, 0x10, 0xbf, 0xc0, 0x10, 0xbf, 0xc0, 0x10, 0xbf,
  0xc0, 0x10, 0xbf, 0xc0, 0x10, 0xbf, 0xc0, 0x10, 0x80, 0x00, 0x70, 0xff, 0xff, 0xc0
};
// '25', 20x10px
const unsigned char Batt25 [] PROGMEM = {
  0xff, 0xff, 0xc0, 0x80, 0x00, 0x70, 0xbc, 0x00, 0x10, 0xbc, 0x00, 0x10, 0xbc, 0x00, 0x10, 0xbc,
  0x00, 0x10, 0xbc, 0x00, 0x10, 0xbc, 0x00, 0x10, 0x80, 0x00, 0x70, 0xff, 0xff, 0xc0
};
// '10 et moins', 20x10px
const unsigned char Batt10 [] PROGMEM = {
  0xff, 0xff, 0xc0, 0x80, 0x00, 0x70, 0xa0, 0x00, 0x10, 0xa0, 0x00, 0x10, 0xa0, 0x00, 0x10, 0xa0,
  0x00, 0x10, 0xa0, 0x00, 0x10, 0xa0, 0x00, 0x10, 0x80, 0x00, 0x70, 0xff, 0xff, 0xc0
};

void setup() {
  //startupsound
  delay(200);
  pinMode(buzzer, OUTPUT);
  tone(buzzer, 700);
  delay(40);
  tone(buzzer, 670);
  delay(40);
  tone(buzzer, 850);
  delay(40);
  noTone(buzzer);
  delay(150); //pour s'assurer que le i2c communique avec l'oled
  pinMode(2, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(rotation);
  display.clearDisplay();
  mlx.begin();
  //Serial.begin(9600);
  Tobjet = mlx.readObjectTempC();
  Tmin = Tobjet;
  Tmax = Tobjet;
  digitalWrite(2, HIGH);
  EtatBat = analogRead(A0);
  displayBattery();
}

void loop() {
  //if ((analogRead(A1) < 250)) { //Vo*2/3 dans le cas ou R up = 10k et Rdown = 20k  , ne fonctionne pas, reste tjrs dans le sleep moade après être activé
  //  digitalWrite(2, LOW);
  //  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  //}
  if (millis() > TempsPrecedent + 200) {
    TempsPrecedent = millis();
    Buzzer();
    Tobjet = mlx.readObjectTempC();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(3, 0);
    display.println("OBJECT:");

    display.setCursor(3, 25);
    display.println("M:");
    display.setCursor(15, 25);
    display.println(Tluemax(), 1);
    display.setCursor(57, 25);
    display.println("m:");
    display.setCursor(69, 25);
    display.println(Tluemin(), 1);

    display.setCursor(3, 9);    //Pour empêcher les caractères de prendre trop d'espace lorsque ça dépasse 100 degrés
    if (Tobjet >= 100) {
      display.setTextSize(1);
    } else {
      display.setTextSize(2);
    }

    display.println(Tobjet, 1);
    display.setCursor(57, 0);
    display.setTextSize(1);
    display.print("AMBIANT:");
    display.setCursor(57, 9);
    display.setTextSize(2);
    display.print(mlx.readAmbientTempC(), 1);

    displayBattery();

    display.setCursor(108, 12);
    display.setTextSize(1);
    display.print(EtatBat * 4.8 / 1023.00, 1); //Bit lues de la tension de la batterie
    display.setCursor(108, 20);
    display.print("V");
    //display.setCursor(108, 28);     //Bit lue de la tension dsuite au relachement du bouton
    //display.setTextSize(1);
    //display.print(analogRead(A1));

    display.display();
  }
  //if (millis() > TempsPrecedent3 + 180000) {             //Met le circuit en Sleepmode si le bouton reste appuyé accidentellement afin d'économiser la batterie
  // TempsPrecedent3 = millis();
  // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  //}
}

void  displayBattery() {
  if (EtatBat > 850) {
    display.drawBitmap(108, 0,  Batt100, 20, 10, WHITE);
  } else if (EtatBat > 775) {
    display.drawBitmap(108, 0,  Batt75, 20, 10, WHITE);
  } else if (EtatBat > 600) {
    display.drawBitmap(108, 0,  Batt50, 20, 10, WHITE);
  } else if (EtatBat > 500) {
    display.drawBitmap(108, 0,  Batt25, 20, 10, WHITE);
  } else if (EtatBat >= 0) {
    display.drawBitmap(108, 0,  Batt10, 20, 10, WHITE);
    if (millis() > TempsPrecedent2 + 1000) {
      TempsPrecedent2 = millis();
      display.drawBitmap(108, 0,  Batt10, 20, 10, BLACK);
    }

  }
}
void Buzzer() {    //Alerte Température max atteind
  if (Tobjet < 120) {
    noTone(buzzer);
  } else if (Tobjet >= 120) {
    tone(buzzer, 600);
  } else if (Tobjet >= 1000) {
    noTone(buzzer);
  }
}
double Tluemax() {
  double result;
  if (Tobjet > Tmax) {
    Tmax = Tobjet;
    result = Tmax;
  }
  return result;
}
double Tluemin() {
  double result;
  if (Tobjet < Tmin) {
    Tmin = Tobjet;
    result = Tmin;
  }
  return result;
}
