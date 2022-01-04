//Spesifikasi Pinout Komponen :
/* Sensor dht11 
 *  => VCC (3.3V)
 *  => OUT 
 *  => GND
 
 * Sensor Soil Moisture 
 * => A0
 * => D0
 * => GND
 * => VCC (3.3V to 5V)
 
 * Sensor Ultrasonik 
 * => VCC
 * => TRIG
 * => ECHO
 * => GND 
 */


//Library yang dibutuhkan
#include <ESP8266WiFi.h>    //Library untuk Board NodeMCU V3 Lolin (ESP8266)
#include <Blynk.h>          //Library untuk App Blynk (legacy)
#include <DHT.h>            //Library untuk Sensor dht11
#include <LiquidCrystal.h>  //Library untuk Sensor Soil Moisture
#include <BlynkSimpleEsp8266.h>


//Penempatan setiap Pin pada nodeMCU

// Variable untuk Sensor Soil moisture
#define sensorTanah A0


// Variable untuk Sensor Ultrasonik
#define echoPin D3
#define trigPin D4

long durasi;   //waktu/Time (t)
int jarak;     //jarak/Distance (s)

// Variable untuk Sensor dht11
#define dhtPin D5
#define DHTTYPE DHT11
  
//Parameter DHT
DHT dht(dhtPin, DHTTYPE);

// Varible unutuk Relay 
#define relay D2
//inisiasi App Blynk 
#define BLYNK_PRINT Serial

//Variable Indikator Sensor Ultrasonik
int LED_HIJAU = D6;
int LED_MERAH = D7;

//Parameter waktu Blynk
BlynkTimer timer;


//Konfigurasi koneksi ke App Blynk
const char token[] = "ZfHTZVFhvts9T8w-wDZ26wyjIkyyQmWM";         //token Blynk
const char ssid[] = "WiFi.id";                                   //Nama username jaringan
const char pass[] = "sopokowe";                                  //Password jaringan

void sendUptime()              //Fungsi untuk pengiriman Pin virtual
{
   Blynk.virtualWrite(V3, millis() / 1000);
   Blynk.virtualWrite(V4, millis() / 1000);
   Blynk.virtualWrite(V5, millis() / 1000);
   Blynk.virtualWrite(V6, millis() / 1000);
}
void suhu()                   //Fungsi untuk membaca sensor dht11
{
  float h = dht.readHumidity();           //Baca suhu sebagai Fahrenheit
  float t = dht.readTemperature();        //Baca suhu sebagai Celcius

  //Kondisi jika suhu dan kelembaban tidak terdeteksi
  if ( isnan(h) || isnan(t) ) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  //Kondisi jika suhu dan kelembaban terdeteksi
  }else {
    //mengirim pesan pada serial monitor 
    Serial.println("\n Kelembaban dan suhu \n");
    Serial.print("Saat ini \n Suhu = ");
    Serial.print(h);
    Serial.print("%  ");
    Serial.print("\n kelembaban = ");
    Serial.print(t);
  }
  //Pin Virtual di App Blynk (Legacy)
  Blynk.virtualWrite(V5, t);
  Blynk.virtualWrite(V6, h);
}


void setup() { //Fungsi awal untuk menjalankan Program
  
  Serial.begin(9600);         //memberikan baud sebesar 9600
  dht.begin();                //untuk menginisialisasi sensor dht11
  //Blynk.begin(token, ssid, pass,);                                   //Langkah 1
  Blynk.begin(token, ssid, pass, "blynk-cloud.com", 8442);             //Langkah 2
  //Blynk.begin(token, ssid, pass, IPAddress(192,168,1,100), 8442);    //Langkah 3
  
  while (Blynk.connect() == false) {
    
  }
  
  //inisiasi Pin
  pinMode(trigPin, OUTPUT);   //menjadikan pin trigger ultrasonik sebagai keluaran
  pinMode(echoPin, INPUT);    //menjadikan pin echo ultrasonik sebagai masukan
  pinMode(relay, OUTPUT);     //menjadikan pin relay sebagai keluaran


}

void loop() {  //Fungsi Kerja untuk menjalankan Program

  timer.setInterval(1000L, suhu);             //untuk memanggil fungsi suhu
  timer.setInterval(1000L, sendUptime);       //untuk mengirim data ke pin virtual
  
  //Kondisi ultrasonik mati
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  //Kondisi ultrasonik nyala
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  //Perhitungan Matematis
  durasi = pulseIn(echoPin, HIGH);

  /* Rumus 
   *  v = 340 m/s == 0.034 m/s
   *  time = distance(s)/speed(v)
   *  distance(s) = time(t) x 0.034(s) / 2
   */
  jarak = durasi * 0.034 / 2;           

float dataTanah = analogRead(sensorTanah);    //mengambil data di pin sensorTanah => ke Variable dataTanah

  //Kondisi Tanah jika < 50%    (Kondisi Kering)
  if(dataTanah < 50){                         
    digitalWrite(relay, HIGH);
  }
  //Kondisi Tanah jika >50%/80% (Kondisi Normal)
  else if((dataTanah > 50) || (dataTanah = 80)){
    digitalWrite(relay, HIGH);
  }
  //Kondisi Tanah jika >80%/100% (Kondisi Basah)             
  else if((dataTanah > 80) || (dataTanah = 100)){
    digitalWrite(relay, LOW);
    delay(1000);

    //Kondisi sensor Ultrasonik (Penampung Air)
    int i = jarak;

    //Logika For ketika i=jarak kurang dari 5 cm
    for(i=5; i<5; i++){     
      digitalWrite(relay, LOW);
      digitalWrite(LED_HIJAU, HIGH);
    }
    //Logika For ketika i=jarak lebih dari 5 cm
    for(i=5; i>5; i++){
      digitalWrite(relay, HIGH);
      digitalWrite(LED_MERAH, HIGH);
    }
    //Pin Virtual di App Blynk (Legacy)
    Blynk.virtualWrite(V4, dataTanah);        //Pin Virtual untuk Sensor Soil Moisture 
    Blynk.virtualWrite(V3, i);                //Pin Virtual untuk Sensor Soil Ultrasonik
  }

Blynk.run();            //untuk menjalankan Blynk
timer.run();            //untuk memulai timer 
}
