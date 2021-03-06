#include <MAX6675.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//ustawienie czujnika temperatury
const byte ONEWIRE_PIN = 2; // Numer pinu do którego podłaczasz czujnik
byte sensorAddress[8] = { 0x28, 0xFF, 0x93, 0x28, 0x34, 0x16, 0x4, 0xBA}; // Adres czujnika
OneWire onewire(ONEWIRE_PIN);
DS18B20 sensors(&onewire);

//ustawienie wyświetlacza
LiquidCrystal_I2C lcd(0x27, 20, 4);

//ustawienie termopary
int CS = 9;             // CS pin on MAX6675
int SO = 8;             // SO pin of MAX6675
int SCLK = 13;          // SCK pin of MAX6675
int units = 1;          // Units to readout temp (0 = raw, 1 = ˚C, 2 = ˚F)
float tempSpalin = 0.0;  // Temperature output variable
MAX6675 temp(CS,SO,SCLK,units);

//ustawienie czujnika odległości
#define trigPin 12
#define echoPin 11

//ustawienie pinu 4 jako "pompa"
#define pompa 4

//ustawienie pinu 5 jako "podajnik"
#define podajnik 5

//ustawienie pinu nr 3 jako pwm dla sterowania dmuchawą
#define PWM_PIN 3

//definicje funkcji
void wyswietl_tempCO();
void wyswietl_tempSpalin();
void wyswietl_paliwo();
void alarm_podajnik();
void alarm_CO();
void normalna_praca();
void pompaCO_ON();
void pompaCO_OFF();
void rozpalanie();
void dmuchawa_moc();
void podajnik_moc();


//ustawienia
const float tmin = 35;  //temperatura minimalna
const float tmax = 85;  //temperatura maksymalna
const float tempZadana = 55;  //temperatura zadana CO
const float tempDom_ust = 20; //ustawienie temperatury w domu
float tempDom = 0.0;
float tempCO = 0.0;
float tempPodajnik = 0.0;
const int moc1 = 5;  //moc1 dmuchawy w %
const int moc2 = 10;
const int moc3 = 50;
const int moc4 = 70;
const int moc5 = 100;
const int czas1 = 1750;  //czas pracy na mocy1
const int czas10 = 59000;  //czas postoju na mocy1
const int czas2 = 2000;
const int czas20 = 58000;
const int czas3 = 3000;
const int czas30 = 57000;
const int czas4 = 4000;
const int czas40 = 56000;
const int czas5 = 5000;
const int czas50 = 55000;
const int czas100 = 2400000;  //czas przerwy podajnika w trakcie podtrzymania 40 min
const int czas111 = 5000;  //czas pracy podajnika w trakcie podtrzymania


void setup()
{
  while(!Serial);
  Serial.begin(9600);
  sensors.begin();  //włączenie czujnika temeratury
  lcd.init();  //włączenie wyświetlacza
  lcd.backlight();  //włączenie podświetlenia wyświetlacza
  
  pinMode(PWM_PIN, OUTPUT); //ustawienie pinu jako wyjścia
  pinMode(trigPin, OUTPUT); //Pin, do którego podłączymy trig jako wyjście
  pinMode(echoPin, INPUT); //a echo, jako wejście
  pinMode(pompa, OUTPUT); //ustawienie pinu pompy jako wyjscia
  digitalWrite(pompa, HIGH); //ustawienie pinu pompy w stanie wysokim
  digitalWrite(podajnik, HIGH); //ustawienie pinu podajnika w stanie wysokim
}

void loop()
{
  sensors.request(sensorAddress);
  while(!sensors.available());
  float tempCO = sensors.readTemperature(sensorAddress);
  float tempDom = sensors.readTemperature(sensorAddress);
  float tempPodajnik = sensors.readTemperature(sensorAddress);
  float tempSpalin = temp.read_temp();
  
  
  wyswietl_tempCO();
  wyswietl_tempSpalin();
  wyswietl_paliwo();
  alarm_podajnik();
  alarm_CO();
}


//funkcja odczytująca i wyświetlająca temperaturę CO
void wyswietl_tempCO()
{
  lcd.setCursor(0, 0);
  lcd.print("Tco: ");
  lcd.print(tempCO);
  lcd.print((char)223);
  lcd.print("C");
}

//funkcja odczytująca i wyświetlająca temperaturę spalin
void wyswietl_tempSpalin()
{
  lcd.setCursor(0, 1);
  lcd.print("Tspalin: ");
  lcd.print( tempSpalin );
  lcd.print((char)223);
  lcd.print("C");
}

//funkcja odczytująca i wyswietlająca poziom paliwa
void wyswietl_paliwo()
  {
  long czas, dystans;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  czas = pulseIn(echoPin, HIGH);
  dystans = czas / 58;
  lcd.setCursor(0, 2);
  lcd.print(dystans);
  lcd.print(" cm");
  delay(500);
  }
  
//funkcja przeciw zapaleniu się podajnika
void alarm_podajnik()
  {
  
  }

//funkcja przeciw zagotowaniu wody
void alarm_CO()
  {
  if((tempCO<=tmax) && (tempCO>=tmin))
  {
  //normalna praca
  lcd.setCursor(0, 3);
  lcd.print("normalna praca");
  normalna_praca();
  }
  if(tempCO<tmin)
  {
  //rozpalanie
  lcd.setCursor(0, 3);
  lcd.print("rozpalanie");
  rozpalanie();
  }
  else
  {
  //wyłączanie podajnika i dmuchawy
  lcd.clear();
  lcd.setCursor(0, 3);
  lcd.print("za wysoka temp!!!");
  podtrzymanie();
  delay(600000); //czekaj 10 minut
  }
  }

//funkcja odpowiedzialna za normalną pracę pieca
void normalna_praca()
{
  while(tempDom<tempDom_ust)
  {
  pompaCO_ON();
  dmuchawa_moc();
  podajnik_moc();
  }
  while(tempDom>=tempDom_ust)
  {
  podtrzymanie();
  }
}

//funkcja odpowiedzialna za rozpalanie
void rozpalanie()
{
  dmuchawa_moc();
}

//funkcja odpowiedzialna za włączenie pompy CO
void pompaCO_ON()
{
  digitalWrite(pompa, LOW);
}

//funkcja odpowiedzialna za wyłączenie pompy CO
void pompaCO_OFF()
{
  digitalWrite(pompa, HIGH);
}

//funkcja odpowiedzialna za moc dmuchawy
void dmuchawa_moc()
{
  while((tempCO<tempZadana) && (tempSpalin<250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc5);
  }
  while((tempCO<tempZadana-5) && (tempSpalin<250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc4);
  }
  while((tempCO<tempZadana-2) && (tempSpalin<250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc3);
  }
  while((tempCO==tempZadana) && (tempSpalin<250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc2);
  }
  while((tempCO>tempZadana+2) && (tempSpalin<250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc1);
  }
  //za wykosa temp spalin
  while((tempCO<tempZadana) && (tempSpalin>=250) && (tempCO>tmin))
  {
  analogWrite(PWM_PIN, moc4);
  }
}

//funkcja odpowiedzialna za moc podajnika
void podajnik_moc()
{
  while((tempCO<tempZadana) && (tempSpalin<250) && (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas5);
  digitalWrite(podajnik, HIGH);
  delay(czas50);
  }
  while((tempCO<tempZadana-5) && (tempSpalin<250) && (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas4);
  digitalWrite(podajnik, HIGH);
  delay(czas40);
  }
  while((tempCO<tempZadana-2) && (tempSpalin<250) && (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas3);
  digitalWrite(podajnik, HIGH);
  delay(czas30);
  }
  while((tempCO==tempZadana) && (tempSpalin<250) && (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas2);
  digitalWrite(podajnik, HIGH);
  delay(czas20);
  }
  while((tempCO>tempZadana+2) && (tempSpalin<250) & (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas1);
  digitalWrite(podajnik, HIGH);
  delay(czas10);
  }
  //za wysoka temp spalin
  while((tempCO<tempZadana) && (tempSpalin>=250) && (tempCO>tmin))
  {
  digitalWrite(podajnik, LOW);
  delay(czas4);
  digitalWrite(podajnik, HIGH);
  delay(czas40);
  }
}


//funkcja odpowiedzialna za podtrzymanie żaru w piecu
void podtrzymanie()
{
  pompaCO_OFF();
  digitalWrite(podajnik, HIGH);
  delay(czas100);
  digitalWrite(podajnik, LOW);
  delay(czas111);
  digitalWrite(podajnik, HIGH);
}
