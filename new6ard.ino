#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <TimeLib.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
int otonomMod = 1;

unsigned long dahiliZamanBaslangic = 0;
bool dahiliZamanAyarlandi = false;
const int SAG_ON_TRIG_PIN = 28;
const int SAG_ON_ECHO_PIN = 29;
const int SOL_ON_TRIG_PIN = 22;
const int SOL_ON_ECHO_PIN = 23;
const int SAG_YAN_TRIG_PIN = 24;
const int SAG_YAN_ECHO_PIN = 25;
const int SOL_YAN_TRIG_PIN = 26;
const int SOL_YAN_ECHO_PIN = 27;
const int UST_TRIG_PIN = 32;
const int UST_ECHO_PIN = 33;
long ustMesafe = 0;
const int UST_GUVENLI_MESAFE = 45;

long sagOnMesafe = 0;
long solOnMesafe = 0;
long sagYanMesafe = 0;
long solYanMesafe = 0;
const int COKLU_SENSOR_GUVENLI_MESAFE = 25;

Adafruit_BME280 bme;
float bmeBasinc = 0;
float bmeSicaklik = 0;
float bmeNem = 0;
unsigned long sonBMEGostermeZamani = 0;
const unsigned long BME_GOSTERME_SURESI = 10000;
bool bmeEkranGoster = false;

const int ENA = 5;
const int ENB = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;

const int ON_TRIG_PIN = 30;
const int ON_ECHO_PIN = 31;
const int GUVENLI_MESAFE = 15;
const int BUZZER_PIN = 11;
const int VAKUM_ROLE = 4;
const int DHT_PIN = 2;
DHT dht(DHT_PIN, DHT11);
const int MQ2_PIN = A0;
const int PIR_PIN = 3;
const int SES_SENSOR_PIN = A1;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DFRobotDFPlayerMini dfPlayer;
const int DFPLAYER_BUSY_PIN = 12;
bool otomatikMod = false;
bool manuelMod = false;
bool vakumDurumu = false;
bool muzikCaliyor = false;
int currentSong = 1;
int totalSongs = 5;

char yon = 'S';

long onMesafe;
const int SUPURGE_GUVENLI_MESAFE = 10;

int motorHizi = 200;
int yavasMotorHizi = 140;
int donmeMotorHizi = 150;

float sicaklik = 0.0;
float nem = 0.0;
int gazSeviyesi = 0;
bool hareketAlgilandi = false;
int sesSeviyesi = 0;

unsigned long calismaBaslangicZamani = 0;
unsigned long toplamCalismaSuresi = 0;
float toplamGidilenMesafe = 0.0;
int otomatikModGecis = 0;
bool istatistikGuncellemesiGerekli = false;

unsigned long sonVeriGondermeZamani = 0;
unsigned long sonSensorOkumaZamani = 0;
unsigned long sonMesafeOlcmeZamani = 0;
unsigned long sonIstatistikKaydetmeZamani = 0;
unsigned long sonHareketZamani = 0;
bool hareketEdiyorMu = false;
unsigned long hareketBaslangicZamani = 0;

bool taramaModu = false;
unsigned long taramaBaslangic = 0;
int enIyiYon = 0;
int enIyiMesafe = 0;

bool taramaIcinDonusYapiliyor = false;
int taramaDereceleri[8] = {0};
int taramaMesafeleri[8] = {0};
int taramaAdimi = 0;
bool zamanliTemizlikAktif = false;
int planliSaat = 0;
int planliDakika = 0;
int planliSure = 30;

#include <EEPROM.h>
const int EEPROM_ADRES_TOPLAM_SURE = 0;
const int EEPROM_ADRES_MESAFE = 4;
const int EEPROM_ADRES_GECIS = 8;

const int OLCUM_GECMISI_SAYISI = 5;
long onMesafeGecmisi[OLCUM_GECMISI_SAYISI] = {0};
long ustMesafeGecmisi[OLCUM_GECMISI_SAYISI] = {0};
int olcumGecmisiIndeksi = 0;

unsigned long sonDebugMesajZamani = 0;
const unsigned long DEBUG_MESAJ_ARALIK = 10000;
unsigned long sonTestZamani = 0;

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  pinMode(ON_TRIG_PIN, OUTPUT);
  pinMode(ON_ECHO_PIN, INPUT);
  
  pinMode(UST_TRIG_PIN, OUTPUT);
  pinMode(UST_ECHO_PIN, INPUT);
  
  pinMode(SAG_ON_TRIG_PIN, OUTPUT);
  pinMode(SAG_ON_ECHO_PIN, INPUT);
  pinMode(SOL_ON_TRIG_PIN, OUTPUT);
  pinMode(SOL_ON_ECHO_PIN, INPUT);
  pinMode(SAG_YAN_TRIG_PIN, OUTPUT);
  pinMode(SAG_YAN_ECHO_PIN, INPUT);
  pinMode(SOL_YAN_TRIG_PIN, OUTPUT);
  pinMode(SOL_YAN_ECHO_PIN, INPUT);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  pinMode(VAKUM_ROLE, OUTPUT);
  digitalWrite(VAKUM_ROLE, HIGH);
  pinMode(PIR_PIN, INPUT);
  
  pinMode(DFPLAYER_BUSY_PIN, INPUT);
  
  dht.begin();
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Akilli Supurge");
  lcd.setCursor(0, 1);
  lcd.print("Baslatiliyor...");
  
  Serial.begin(115200);
  
  Serial1.begin(9600);
  
  Wire.begin();
  
  if (!bme.begin(0x76)) {
    lcd.setCursor(0, 1);
    lcd.print("BME280 Hata!    ");
    delay(1000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("BME280 OK!      ");
    delay(1000);
  }
  
  delay(1000);
  if (dfPlayer.begin(Serial1)) {
    lcd.setCursor(0, 1);
    lcd.print("DFPlayer OK     ");
    Serial.println("DFPlayer başarıyla başlatıldı");
    
    dfPlayer.reset();
    delay(1000);
    dfPlayer.setTimeOut(500);
    dfPlayer.volume(15);
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    
    totalSongs = dfPlayer.readFileCounts();
    Serial.print("SD karttaki toplam şarkı sayısı: ");
    Serial.println(totalSongs);
    
    dfPlayer.play(1);
    Serial.println("Test sesi çalınıyor...");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("DFPlayer Hata!  ");
    Serial.println("DFPlayer başlatılamadı!");
  }
  
  dahiliZamanBaslangic = millis();
  setTime(0, 0, 0, 1, 1, 2023);
  
  EEPROM.get(EEPROM_ADRES_TOPLAM_SURE, toplamCalismaSuresi);
  EEPROM.get(EEPROM_ADRES_MESAFE, toplamGidilenMesafe);
  EEPROM.get(EEPROM_ADRES_GECIS, otomatikModGecis);
  
  durdur();
  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mod: Hazir");
  
  onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
  ustMesafe = mesafeOlc(UST_TRIG_PIN, UST_ECHO_PIN);
  
  sagOnMesafe = mesafeOlc(SAG_ON_TRIG_PIN, SAG_ON_ECHO_PIN);
  solOnMesafe = mesafeOlc(SOL_ON_TRIG_PIN, SOL_ON_ECHO_PIN);
  sagYanMesafe = mesafeOlc(SAG_YAN_TRIG_PIN, SAG_YAN_ECHO_PIN);
  solYanMesafe = mesafeOlc(SOL_YAN_TRIG_PIN, SOL_YAN_ECHO_PIN);
  
  Serial.println("Arduino Mega hazır, komut bekleniyor!");
}

void loop() {
  if (millis() - dahiliZamanBaslangic >= 1000) {
    adjustTime(1);
    dahiliZamanBaslangic = millis();
  }

  if (Serial.available() > 0) {
    String komut = Serial.readStringUntil('\n');
    komut.trim();
    
    if (komut == "TEST_MUZIK") {
      Serial.println("Müzik testi başlıyor...");
      dfPlayerDebug();
      dfPlayer.play(1);
      sonTestZamani = millis();
    } 
    else if (komut == "TEST_SENSORLER") {
      Serial.println("Sensör testi başlıyor...");
      sensorKontrol();
      sonTestZamani = millis();
    }
    else if (komut == "OTONOM_MOD_1") {
      otonomMod = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Otonom Mod: 1");
      lcd.setCursor(0, 1);
      lcd.print("Coklu Sensor");
      Serial.println("Otonom Mod 1 seçildi (Çoklu Sensör)");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      if (otomatikMod) {
        lcd.print("Mod: Otomatik");
      } else if (manuelMod) {
        lcd.print("Mod: Manuel");
      } else {
        lcd.print("Mod: Hazir");
      }
    }
    else if (komut == "OTONOM_MOD_2") {
      otonomMod = 2;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Otonom Mod: 2");
      lcd.setCursor(0, 1);
      lcd.print("Ust-On Sensor");
      Serial.println("Otonom Mod 2 seçildi (Üst-Ön Sensör)");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      if (otomatikMod) {
        lcd.print("Mod: Otomatik");
      } else if (manuelMod) {
        lcd.print("Mod: Manuel");
      } else {
        lcd.print("Mod: Hazir");
      }
    }
    else {
      komutIsle(komut);
    }
  }
  
  if (zamanliTemizlikAktif) {
    if (hour() == planliSaat && minute() == planliDakika && !otomatikMod && !manuelMod && second() < 60) {
      otomatikMod = true;
      manuelMod = false;
      vakumAc();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Planli");
      lcd.setCursor(0, 1);
      lcd.print("Temizlik");
      Serial.println("PLANLI_TEMIZLIK_BASLADI");
      
      calismaBaslangicZamani = millis();
      
      otomatikModGecis++;
      istatistikGuncellemesiGerekli = true;
    }
    
    if (otomatikMod && millis() - calismaBaslangicZamani > planliSure * 60000) {
      otomatikMod = false;
      vakumKapat();
      durdur();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
      lcd.setCursor(0, 1);
      lcd.print("Plan tamamlandi");
      Serial.println("PLANLI_TEMIZLIK_TAMAMLANDI");
      
      long calismaSuresi = (millis() - calismaBaslangicZamani);
      toplamCalismaSuresi += calismaSuresi;
      istatistikGuncellemesiGerekli = true;
    }
  }
  
  if (millis() - sonSensorOkumaZamani > 2000) {
    sensorleriOku();
    sonSensorOkumaZamani = millis();
  }
  
  if (millis() - sonMesafeOlcmeZamani > 200) {
    onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
    ustMesafe = mesafeOlc(UST_TRIG_PIN, UST_ECHO_PIN);
    
    sagOnMesafe = mesafeOlc(SAG_ON_TRIG_PIN, SAG_ON_ECHO_PIN);
    solOnMesafe = mesafeOlc(SOL_ON_TRIG_PIN, SOL_ON_ECHO_PIN);
    sagYanMesafe = mesafeOlc(SAG_YAN_TRIG_PIN, SAG_YAN_ECHO_PIN);
    solYanMesafe = mesafeOlc(SOL_YAN_TRIG_PIN, SOL_YAN_ECHO_PIN);
    
    if (!bmeEkranGoster) {
      lcd.setCursor(0, 1);
      if (otonomMod == 1) {
        lcd.print("SO:");
        lcd.print(sagOnMesafe);
        lcd.print(" LO:");
        lcd.print(solOnMesafe);
        lcd.print("   ");
      } else {
        lcd.print("On:");
        lcd.print(onMesafe);
        lcd.print(" Ust:");
        lcd.print(ustMesafe);
        lcd.print("   ");
      }
    }
    
    sonMesafeOlcmeZamani = millis();
  }
  
  if (istatistikGuncellemesiGerekli && millis() - sonIstatistikKaydetmeZamani > 60000) {
    istatistikleriKaydet();
    istatistikGuncellemesiGerekli = false;
    sonIstatistikKaydetmeZamani = millis();
  }
  
  if (millis() - sonVeriGondermeZamani > 500) {
    sensorVerileriniGonder();
    sonVeriGondermeZamani = millis();
  }
  
  if (millis() - sonBMEGostermeZamani > BME_GOSTERME_SURESI) {
    bmeSicaklik = bme.readTemperature();
    bmeNem = bme.readHumidity();
    bmeBasinc = bme.readPressure() / 100.0F;
    
    if (!otomatikMod && !manuelMod) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("S:");
      lcd.print(bmeSicaklik, 1);
      lcd.print("C B:");
      lcd.print(bmeBasinc, 0);
      lcd.print("hPa");
      
      lcd.setCursor(0, 1);
      lcd.print("Nem: %");
      lcd.print(bmeNem, 0);
      
      bmeEkranGoster = true;
      
      delay(3000);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
      bmeEkranGoster = false;
    }
    
    sonBMEGostermeZamani = millis();
  }
  
  if (millis() - sonDebugMesajZamani > DEBUG_MESAJ_ARALIK) {
    Serial.print("--- DEBUG: Ön Mesafe: " + String(onMesafe) + " cm, ");
    if (otonomMod == 1) {
      Serial.print("Sağ Ön: " + String(sagOnMesafe) + " Sol Ön: " + String(solOnMesafe) + ", ");
    } else {
      Serial.print("Üst: " + String(ustMesafe) + " cm, ");
    }
    Serial.println("Tarama: " + String(taramaModu ? "AÇIK" : "KAPALI") + ", Mod: " + String(otonomMod) + " ---");
    sonDebugMesajZamani = millis();
  }
  
  if (digitalRead(DFPLAYER_BUSY_PIN) == LOW) {
    muzikCaliyor = true;
  } else {
    static unsigned long sonMuzikKontrolu = 0;
    static bool tahminCalisiyor = false;
    
    if (muzikCaliyor && millis() - sonMuzikKontrolu > 3000) {
      sonMuzikKontrolu = millis();
      tahminCalisiyor = true;
    }
  }
  
  if (otomatikMod) {
    if (taramaModu) {
      otonomTaramaYap();
    } else {
      if (hareketEdiyorMu) {
        if (millis() - sonHareketZamani > 3000) {
          durdur();
          hareketEdiyorMu = false;
          lcd.setCursor(0, 1);
          lcd.print("Hareket yok!    ");
        } else {
          otomatikModCalistir();
        }
      } else {
        otomatikModCalistir();
        
        if (yon != 'S') {
          hareketEdiyorMu = true;
          sonHareketZamani = millis();
        }
      }
    }
  } else if (manuelMod) {
    manuelModCalistir();
  } else {
    durdur();
  }
}

void komutIsle(String komut) {
  Serial.print("Alinan komut: ");
  Serial.println(komut);
  
  if (komut == "OTOMATIK_AC") {
    otomatikMod = true;
    manuelMod = false;
    vakumAc();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mod: Otomatik");
    Serial.println("Otomatik mod acildi");
    
    calismaBaslangicZamani = millis();
    otomatikModGecis++;
    istatistikGuncellemesiGerekli = true;
    
    hareketEdiyorMu = true;
    sonHareketZamani = millis();
    
    taramaModu = false;
  } 
  else if (komut == "OTOMATIK_KAPAT") {
    if (otomatikMod) {
      long calismaSuresi = (millis() - calismaBaslangicZamani);
      toplamCalismaSuresi += calismaSuresi;
      istatistikGuncellemesiGerekli = true;
    }
    
    otomatikMod = false;
    vakumKapat();
    durdur();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mod: Hazir");
    Serial.println("Otomatik mod kapatildi");
    
    taramaModu = false;
  } 
  else if (komut == "MANUEL_AC") {
    manuelMod = true;
    otomatikMod = false;
    vakumAc();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mod: Manuel");
    Serial.println("Manuel mod acildi");
    
    calismaBaslangicZamani = millis();
  } 
  else if (komut == "MANUEL_KAPAT") {
    if (manuelMod) {
      long calismaSuresi = (millis() - calismaBaslangicZamani);
      toplamCalismaSuresi += calismaSuresi;
      istatistikGuncellemesiGerekli = true;
    }
    
    manuelMod = false;
    vakumKapat();
    durdur();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mod: Hazir");
    Serial.println("Manuel mod kapatildi");
  } 
  else if (komut == "VAKUM_AC") {
    vakumAc();
  } 
  else if (komut == "VAKUM_KAPAT") {
    vakumKapat();
  } 
  else if (komut.startsWith("YON:")) {
    yon = komut.charAt(4);
    
    if (manuelMod) {
      switch (yon) {
        case 'F':
          ileriGit(motorHizi);
          break;
        case 'B':
          geriGit(motorHizi);
          break;
        case 'L':
          solaDon(motorHizi);
          break;
        case 'R':
          sagaDon(motorHizi);
          break;
        case 'S':
        default:
          durdur();
          break;
      }
    }
  }
  else if (komut.startsWith("HIZ:")) {
    motorHizi = komut.substring(4).toInt();
  }
  else if (komut == "MUZIK_PLAY") {
    muzikCal();
  }
  else if (komut == "MUZIK_PAUSE") {
    muzikDuraklat();
  }
  else if (komut == "MUZIK_NEXT") {
    sonrakiSarki();
  }
  else if (komut == "MUZIK_PREV") {
    oncekiSarki();
  }
  else if (komut.startsWith("MUZIK_VOLUME:")) {
    int volume = komut.substring(13).toInt();
    muzikSes(volume);
  }
  else if (komut.startsWith("MUZIK_TRACK:")) {
    int track = komut.substring(12).toInt();
    sarkiSec(track);
  }
  else if (komut.startsWith("PLANLI_TEMIZLIK_AYARLA:")) {
    String planBilgisi = komut.substring(23);
    
    int ilkNoktali = planBilgisi.indexOf(':');
    int ikinciNoktali = planBilgisi.lastIndexOf(':');
    
    if (ilkNoktali > 0 && ikinciNoktali > ilkNoktali) {
      planliSaat = planBilgisi.substring(0, ilkNoktali).toInt();
      planliDakika = planBilgisi.substring(ilkNoktali + 1, ikinciNoktali).toInt();
      planliSure = planBilgisi.substring(ikinciNoktali + 1).toInt();
      
      zamanliTemizlikAktif = true;
      Serial.println("PLANLI_TEMIZLIK_AYARLANDI");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Plan: ");
      if (planliSaat < 10) lcd.print("0");
      lcd.print(planliSaat);
      lcd.print(":");
      if (planliDakika < 10) lcd.print("0");
      lcd.print(planliDakika);
      lcd.setCursor(0, 1);
      lcd.print("Sure: ");
      lcd.print(planliSure);
      lcd.print(" dk");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
      lcd.setCursor(0, 1);
      lcd.print("Plan aktif");
    }
  }
  else if (komut == "PLANLI_TEMIZLIK_IPTAL") {
    zamanliTemizlikAktif = false;
    Serial.println("PLANLI_TEMIZLIK_IPTAL_EDILDI");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mod: Hazir");
    lcd.setCursor(0, 1);
    lcd.print("Plan iptal");
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("               ");
  }
  else if (komut == "ISTATISTIK_SIFIRLA") {
    toplamCalismaSuresi = 0;
    toplamGidilenMesafe = 0;
    otomatikModGecis = 0;
    istatistikleriKaydet();
    Serial.println("ISTATISTIKLER_SIFIRLANDI");
  }
  else if (komut.startsWith("ZAMAN_AYARLA:")) {
    String zamanBilgisi = komut.substring(13);
    
    int ilkNoktali = zamanBilgisi.indexOf(':');
    int ikinciNoktali = zamanBilgisi.lastIndexOf(':');
    
    if (ilkNoktali > 0 && ikinciNoktali > ilkNoktali) {
      int saat = zamanBilgisi.substring(0, ilkNoktali).toInt();
      int dakika = zamanBilgisi.substring(ilkNoktali + 1, ikinciNoktali).toInt();
      int saniye = zamanBilgisi.substring(ikinciNoktali + 1).toInt();
      
      setTime(saat, dakika, saniye, day(), month(), year());
      dahiliZamanBaslangic = millis();
      dahiliZamanAyarlandi = true;
      
      Serial.println("ZAMAN_AYARLANDI");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Zaman ayarlandi");
      lcd.setCursor(0, 1);
      if (saat < 10) lcd.print("0");
      lcd.print(saat);
      lcd.print(":");
      if (dakika < 10) lcd.print("0");
      lcd.print(dakika);
      lcd.print(":");
      if (saniye < 10) lcd.print("0");
      lcd.print(saniye);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
    }
  }
  else if (komut.startsWith("TARIH_AYARLA:")) {
    String tarihBilgisi = komut.substring(13);
    
    int ilkNoktali = tarihBilgisi.indexOf(':');
    int ikinciNoktali = tarihBilgisi.lastIndexOf(':');
    
    if (ilkNoktali > 0 && ikinciNoktali > ilkNoktali) {
      int gun = tarihBilgisi.substring(0, ilkNoktali).toInt();
      int ay = tarihBilgisi.substring(ilkNoktali + 1, ikinciNoktali).toInt();
      int yil = tarihBilgisi.substring(ikinciNoktali + 1).toInt();
      
      setTime(hour(), minute(), second(), gun, ay, yil);
      
      Serial.println("TARIH_AYARLANDI");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tarih ayarlandi");
      lcd.setCursor(0, 1);
      if (gun < 10) lcd.print("0");
      lcd.print(gun);
      lcd.print("/");
      if (ay < 10) lcd.print("0");
      lcd.print(ay);
      lcd.print("/");
      lcd.print(yil);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
    }
  }
}

void dfPlayerDebug() {
  Serial.println("DFPlayer durumu kontrol ediliyor...");
  
  Serial.print("BUSY pin durumu: ");
  Serial.println(digitalRead(DFPLAYER_BUSY_PIN) == LOW ? "Çalışıyor" : "Beklemede");
  
  Serial.print("SD kart kontrolü: ");
  bool sdOk = false;
  
  uint8_t tip = dfPlayer.readType();
  uint16_t deger = dfPlayer.read();
  
  if (dfPlayer.available()) {
    sdOk = true;
    Serial.println("Bağlı");
    
    int dosyaSayisi = dfPlayer.readFileCounts();
    Serial.print("Toplam dosya sayısı: ");
    Serial.println(dosyaSayisi);
    
    int sesSeviyesi = dfPlayer.readVolume();
    Serial.print("Ses seviyesi: ");
    Serial.println(sesSeviyesi);
    
    int suan = dfPlayer.readCurrentFileNumber();
    Serial.print("Çalan dosya: ");
    Serial.println(suan);
  } else {
    Serial.println("Yanıt vermiyor!");
    Serial.println("DFPlayer'ı yeniden başlatmayı deneyin...");
    
    dfPlayer.reset();
    delay(1000);
    
    if (dfPlayer.begin(Serial1)) {
      Serial.println("DFPlayer yeniden başlatıldı!");
      dfPlayer.volume(15);
    } else {
      Serial.println("DFPlayer başlatılamadı!");
    }
  }
}

void sensorKontrol() {
  Serial.println("\n----- SENSÖR DURUMU -----");
  
  Serial.print("DHT11: ");
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  if (isnan(t) || isnan(h)) {
    Serial.println("HATA - Veri okunamıyor!");
    Serial.println("  - Bağlantıları kontrol edin");
    Serial.println("  - DHT11 sensörü PIN: " + String(DHT_PIN));
  } else {
    Serial.print("Sıcaklık: ");
    Serial.print(t);
    Serial.print("°C, Nem: %");
    Serial.println(h);
  }
  
  int gazDegeri = analogRead(MQ2_PIN);
  Serial.print("MQ2 Gaz sensörü (A0): ");
  Serial.print(gazDegeri);
  if (gazDegeri < 50) {
    Serial.println(" - UYARI: Çok düşük değer, bağlantıyı kontrol edin");
  } else {
    Serial.println(" - Normal");
  }
  
  int pirDurum = digitalRead(PIR_PIN);
  Serial.print("PIR Hareket sensörü (PIN 3): ");
  Serial.println(pirDurum == HIGH ? "Hareket Algılandı" : "Hareket Yok");
  
  long onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
  Serial.print("Ön Mesafe sensörü: ");
  Serial.print(onMesafe);
  Serial.println(" cm");
  
  Serial.print("Üst Mesafe sensörü: ");
  Serial.print(mesafeOlc(UST_TRIG_PIN, UST_ECHO_PIN));
  Serial.println(" cm");
  
  Serial.print("Sağ Ön Mesafe sensörü: ");
  Serial.print(mesafeOlc(SAG_ON_TRIG_PIN, SAG_ON_ECHO_PIN));
  Serial.println(" cm");
  
  Serial.print("Sol Ön Mesafe sensörü: ");
  Serial.print(mesafeOlc(SOL_ON_TRIG_PIN, SOL_ON_ECHO_PIN));
  Serial.println(" cm");
  
  Serial.print("Sağ Yan Mesafe sensörü: ");
  Serial.print(mesafeOlc(SAG_YAN_TRIG_PIN, SAG_YAN_ECHO_PIN));
  Serial.println(" cm");
  
  Serial.print("Sol Yan Mesafe sensörü: ");
  Serial.print(mesafeOlc(SOL_YAN_TRIG_PIN, SOL_YAN_ECHO_PIN));
  Serial.println(" cm");
  
  int sesDegeri = analogRead(SES_SENSOR_PIN);
  Serial.print("Ses sensörü (A1): ");
  Serial.println(sesDegeri);
  
  Serial.print("BME280 sensörü (I2C): ");
  if (isnan(bme.readTemperature())) {
    Serial.println("HATA - Veri okunamıyor!");
    Serial.println("  - I2C bağlantısını kontrol edin (SDA, SCL)");
    Serial.println("  - I2C adresi: 0x76 veya 0x77 olmalı");
  } else {
    Serial.print("Sıcaklık: ");
    Serial.print(bme.readTemperature());
    Serial.print("°C, Nem: %");
    Serial.print(bme.readHumidity());
    Serial.print(", Basınç: ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
  }
  
  Serial.println("--------------------------\n");
}

void otomatikModCalistir() {
  int currentHiz = yavasMotorHizi;
  
  if (otonomMod == 1) {
    if (sagOnMesafe <= COKLU_SENSOR_GUVENLI_MESAFE || solOnMesafe <= COKLU_SENSOR_GUVENLI_MESAFE) {
      durdur();
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      
      geriGit(currentHiz);
      delay(1000);
      durdur();
      delay(300);
      
      taramaModu = true;
      taramaBaslangic = millis();
      
      return;
    }
    
    ileriGit(currentHiz);
    
  } else {
    if (onMesafe <= GUVENLI_MESAFE || ustMesafe <= UST_GUVENLI_MESAFE) {
      durdur();
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      
      geriGit(currentHiz);
      delay(1000);
      durdur();
      delay(300);
      
      taramaModu = true;
      taramaBaslangic = millis();
      
      return;
    }
    
    ileriGit(currentHiz);
  }
  
  toplamGidilenMesafe += 0.0002;
  istatistikGuncellemesiGerekli = true;
  
  sonHareketZamani = millis();
  hareketEdiyorMu = true;
}

void otonomTaramaYap() {
  static unsigned long sonDonusZamani = 0;
  static int donusSuresi = 0;
  
  if (millis() - taramaBaslangic < 100) {
    taramaAdimi = 0;
    enIyiYon = 0;
    enIyiMesafe = 0;
    
    sonDonusZamani = millis();
    
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    
    lcd.setCursor(0, 1);
    lcd.print("Tarama yapiliyor");
  }
  
  if (taramaAdimi < 8) {
    if (millis() - sonDonusZamani < 500) {
      sagaDon(donmeMotorHizi);
    } 
    else if (millis() - sonDonusZamani < 1000) {
      durdur();
      
      if (millis() - sonDonusZamani >= 700) {
        long olculenMesafe;
        
        if (otonomMod == 1) {
          long sagOn = mesafeOlc(SAG_ON_TRIG_PIN, SAG_ON_ECHO_PIN);
          long solOn = mesafeOlc(SOL_ON_TRIG_PIN, SOL_ON_ECHO_PIN);
          olculenMesafe = (sagOn + solOn) / 2;
        } else {
          olculenMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
        }
        
        taramaMesafeleri[taramaAdimi] = olculenMesafe;
        
        int guvenliMesafe = (otonomMod == 1) ? COKLU_SENSOR_GUVENLI_MESAFE : GUVENLI_MESAFE;
        if (olculenMesafe > enIyiMesafe && olculenMesafe > guvenliMesafe) {
          enIyiMesafe = olculenMesafe;
          enIyiYon = taramaAdimi;
        }
      }
    }
    else {
      taramaAdimi++;
      sonDonusZamani = millis();
    }
  }
  else if (taramaAdimi == 8) {
    int guvenliMesafe = (otonomMod == 1) ? COKLU_SENSOR_GUVENLI_MESAFE : GUVENLI_MESAFE;
    
    if (enIyiMesafe <= guvenliMesafe) {
      geriGit(yavasMotorHizi);
      delay(1000);
      durdur();
      delay(300);
      
      taramaAdimi = 0;
      enIyiYon = 0;
      enIyiMesafe = 0;
      sonDonusZamani = millis();
      
      return;
    }
    
    taramaAdimi = 9;
    sonDonusZamani = millis();
    donusSuresi = 500 * enIyiYon;
  }
  else if (taramaAdimi == 9) {
    if (millis() - sonDonusZamani < donusSuresi) {
      sagaDon(donmeMotorHizi);
    } else {
      durdur();
      delay(300);
      
      taramaModu = false;
      
      lcd.setCursor(0, 1);
      lcd.print("Tarama tamam!   ");
      
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      
      ileriGit(yavasMotorHizi);
    }
  }
}

void manuelModCalistir() {
  static unsigned long sonYonGuncelleme = 0;
  if (millis() - sonYonGuncelleme > 100) {
    switch (yon) {
      case 'F':
        ileriGit(motorHizi);
        
        float hizKatsayi = motorHizi / 120.0;
        toplamGidilenMesafe += 0.0002 * hizKatsayi;
        istatistikGuncellemesiGerekli = true;
        break;
      case 'B':
        geriGit(motorHizi);
        hizKatsayi = motorHizi / 120.0;
        toplamGidilenMesafe += 0.0002 * hizKatsayi;
        istatistikGuncellemesiGerekli = true;
        break;
      case 'L':
        solaDon(motorHizi);
        break;
      case 'R':
        sagaDon(motorHizi);
        break;
      case 'S':
      default:
        durdur();
        break;
    }
    sonYonGuncelleme = millis();
  }
}

long mesafeOlc(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long sure = pulseIn(echoPin, HIGH, 30000);
  if (sure == 0) return 100;
  
  long mesafe = sure * 0.034 / 2;
  
  if (mesafe > 400) mesafe = 400;
  if (mesafe <= 0) mesafe = 100;
  
  return mesafe;
}

void ileriGit(int hiz) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, hiz);
  analogWrite(ENB, hiz);
}

void geriGit(int hiz) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, hiz);
  analogWrite(ENB, hiz);
}

void solaDon(int hiz) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, hiz);
  analogWrite(ENB, hiz);
}

void sagaDon(int hiz) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, hiz);
  analogWrite(ENB, hiz);
}

void durdur() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  yon = 'S';
}

void vakumAc() {
  digitalWrite(VAKUM_ROLE, LOW);
  vakumDurumu = true;
  Serial.println("VAKUM:ACIK");
}

void vakumKapat() {
  digitalWrite(VAKUM_ROLE, HIGH);
  vakumDurumu = false;
  Serial.println("VAKUM:KAPALI");
}

void muzikCal() {
  dfPlayer.start();
  muzikCaliyor = true;
  Serial.println("MUZIK:CALIYOR");
  lcd.setCursor(0, 1);
  lcd.print("Muzik: ");
  lcd.print(currentSong);
  lcd.print("       ");
}

void muzikDuraklat() {
  dfPlayer.pause();
  muzikCaliyor = false;
  Serial.println("MUZIK:DURAKLATILDI");
}

void sonrakiSarki() {
  currentSong++;
  if (currentSong > totalSongs) {
    currentSong = 1;
  }
  dfPlayer.play(currentSong);
  muzikCaliyor = true;
  Serial.print("MUZIK:TRACK:");
  Serial.println(currentSong);
  lcd.setCursor(0, 1);
  lcd.print("Muzik: ");
  lcd.print(currentSong);
  lcd.print("       ");
}

void oncekiSarki() {
  currentSong--;
  if (currentSong < 1) {
    currentSong = totalSongs;
  }
  dfPlayer.play(currentSong);
  muzikCaliyor = true;
  Serial.print("MUZIK:TRACK:");
  Serial.println(currentSong);
  lcd.setCursor(0, 1);
  lcd.print("Muzik: ");
  lcd.print(currentSong);
  lcd.print("       ");
}

void muzikSes(int volume) {
  if (volume >= 0 && volume <= 30) {
    dfPlayer.volume(volume);
    Serial.print("MUZIK:SES:");
    Serial.println(volume);
  }
}

void sarkiSec(int track) {
  if (track >= 1 && track <= totalSongs) {
    currentSong = track;
    dfPlayer.play(track);
    muzikCaliyor = true;
    Serial.print("MUZIK:TRACK:");
    Serial.println(track);
    lcd.setCursor(0, 1);
    lcd.print("Muzik: ");
    lcd.print(currentSong);
    lcd.print("       ");
  }
}

void sensorleriOku() {
  sicaklik = dht.readTemperature();
  nem = dht.readHumidity();
  
  gazSeviyesi = analogRead(MQ2_PIN);
  
  hareketAlgilandi = digitalRead(PIR_PIN);
  
  sesSeviyesi = analogRead(SES_SENSOR_PIN);
  
  onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
  ustMesafe = mesafeOlc(UST_TRIG_PIN, UST_ECHO_PIN);
  sagOnMesafe = mesafeOlc(SAG_ON_TRIG_PIN, SAG_ON_ECHO_PIN);
  solOnMesafe = mesafeOlc(SOL_ON_TRIG_PIN, SOL_ON_ECHO_PIN);
  sagYanMesafe = mesafeOlc(SAG_YAN_TRIG_PIN, SAG_YAN_ECHO_PIN);
  solYanMesafe = mesafeOlc(SOL_YAN_TRIG_PIN, SOL_YAN_ECHO_PIN);
  
  bmeSicaklik = bme.readTemperature();
  bmeNem = bme.readHumidity();
  bmeBasinc = bme.readPressure() / 100.0F;
}

void sensorVerileriniGonder() {
  Serial.print("SENSOR_DATA:{");
  
  Serial.print("\"on\":");
  Serial.print(onMesafe);
  Serial.print(",\"ust\":");
  Serial.print(ustMesafe);
  
  Serial.print(",\"sagOn\":");
  Serial.print(sagOnMesafe);
  Serial.print(",\"solOn\":");
  Serial.print(solOnMesafe);
  Serial.print(",\"sagYan\":");
  Serial.print(sagYanMesafe);
  Serial.print(",\"solYan\":");
  Serial.print(solYanMesafe);
  
  Serial.print(",\"sicaklik\":");
  Serial.print(sicaklik);
  Serial.print(",\"nem\":");
  Serial.print(nem);
  Serial.print(",\"gaz\":");
  Serial.print(gazSeviyesi);
  Serial.print(",\"hareket\":");
  Serial.print(hareketAlgilandi ? 1 : 0);
  Serial.print(",\"ses\":");
  Serial.print(sesSeviyesi);
  
  Serial.print(",\"bmeSicaklik\":");
  Serial.print(bmeSicaklik);
  Serial.print(",\"bmeNem\":");
  Serial.print(bmeNem);
  Serial.print(",\"bmeBasinc\":");
  Serial.print(bmeBasinc);
  
  Serial.print(",\"vakum\":");
  Serial.print(vakumDurumu ? "true" : "false");
  Serial.print(",\"otomatik\":");
  Serial.print(otomatikMod ? "true" : "false");
  Serial.print(",\"manuel\":");
  Serial.print(manuelMod ? "true" : "false");
  Serial.print(",\"muzik\":");
  Serial.print(muzikCaliyor ? "true" : "false");
  Serial.print(",\"track\":");
  Serial.print(currentSong);
  Serial.print(",\"tarama\":");
  Serial.print(taramaModu ? "true" : "false");
  
  Serial.print(",\"otonomMod\":");
  Serial.print(otonomMod);
  
  Serial.print(",\"saat\":");
  Serial.print(hour());
  Serial.print(",\"dakika\":");
  Serial.print(minute());
  Serial.print(",\"saniye\":");
  Serial.print(second());
  Serial.print(",\"gun\":");
  Serial.print(day());
  Serial.print(",\"ay\":");
  Serial.print(month());
  Serial.print(",\"yil\":");
  Serial.print(year());
  
  Serial.print(",\"planliTemizlik\":");
  Serial.print(zamanliTemizlikAktif ? "true" : "false");
  
  Serial.print(",\"toplamSureDk\":");
  Serial.print(toplamCalismaSuresi / 60000);
  Serial.print(",\"toplamMesafe\":");
  Serial.print(toplamGidilenMesafe);
  Serial.print(",\"otomatikGecis\":");
  Serial.print(otomatikModGecis);
  
  Serial.println("}");
}

void istatistikleriKaydet() {
  EEPROM.put(EEPROM_ADRES_TOPLAM_SURE, toplamCalismaSuresi);
  EEPROM.put(EEPROM_ADRES_MESAFE, toplamGidilenMesafe);
  EEPROM.put(EEPROM_ADRES_GECIS, otomatikModGecis);
}
