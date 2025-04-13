#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>
#include <TimeLib.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// YENİ - Dahili zaman tutma değişkenleri
unsigned long dahiliZamanBaslangic = 0;
bool dahiliZamanAyarlandi = false;

// YENİ - Üst mesafe sensörü tanımlamaları
const int UST_TRIG_PIN = 32;
const int UST_ECHO_PIN = 33;
long ustMesafe = 0;
const int UST_GUVENLI_MESAFE = 15; // 15 cm güvenli mesafe (tavan/masa için)

// BME280 Sensör Tanımlaması
Adafruit_BME280 bme;
float bmeBasinc = 0;
float bmeSicaklik = 0;
float bmeNem = 0;
unsigned long sonBMEGostermeZamani = 0;
const unsigned long BME_GOSTERME_SURESI = 10000; // 10 saniye
bool bmeEkranGoster = false;

// Motor Sürücü Pinleri (L298N)
const int ENA = 5;
const int ENB = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;

// Mesafe Sensörü Pinleri - SADECE ÖN MESAFE SENSÖRÜ KULLANILACAK
const int ON_TRIG_PIN = 30; // Öndeki sensör için TRIG pini
const int ON_ECHO_PIN = 31; // Öndeki sensör için ECHO pini
const int GUVENLI_MESAFE = 15; // 15 cm güvenli mesafe (kullanıcı isteği)

// Buzzer Pin
const int BUZZER_PIN = 11;

// Vakum Motoru Röle Pini
const int VAKUM_ROLE = 4;

// DHT11 Sıcaklık ve Nem Sensörü
const int DHT_PIN = 2;
DHT dht(DHT_PIN, DHT11);

// MQ2 Gaz Sensörü
const int MQ2_PIN = A0;

// PIR Hareket Sensörü
const int PIR_PIN = 3;

// Ses Sensörü
const int SES_SENSOR_PIN = A1;

// LCD I2C Ekran
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C adresi 0x27, 16 sütun, 2 satır

// DFPlayer Mini için Hardware Serial kullanımı
DFRobotDFPlayerMini dfPlayer;

// YENİ - DFPlayer BUSY pini
const int DFPLAYER_BUSY_PIN = 12;

// Mod değişkenleri
bool otomatikMod = false;
bool manuelMod = false;
bool vakumDurumu = false;
bool muzikCaliyor = false;
int currentSong = 1;
int totalSongs = 5; // SD karttaki toplam şarkı sayısı

// Manuel mod için yön değişkenleri
char yon = 'S'; // Durma

// Mesafe ölçüm değişkenleri
long onMesafe; // Tek ön mesafe sensörü
const int SUPURGE_GUVENLI_MESAFE = 10; // Süpürge için 10 cm güvenli mesafe

// Motor hızı
int motorHizi = 200;            // Normal hız
int yavasMotorHizi = 140;       // Otonom modda yavaş hız
int donmeMotorHizi = 150;       // Otonom modda dönüş hızı (daha yüksek hız)

// Sensör değerleri
float sicaklik = 0.0;
float nem = 0.0;
int gazSeviyesi = 0;
bool hareketAlgilandi = false;
int sesSeviyesi = 0;

// İstatistik değişkenleri
unsigned long calismaBaslangicZamani = 0;
unsigned long toplamCalismaSuresi = 0; // Milisaniye cinsinden
float toplamGidilenMesafe = 0.0;       // Metre cinsinden
int otomatikModGecis = 0;
bool istatistikGuncellemesiGerekli = false;

// Zamanlama değişkenleri
unsigned long sonVeriGondermeZamani = 0;
unsigned long sonSensorOkumaZamani = 0;
unsigned long sonMesafeOlcmeZamani = 0;
unsigned long sonIstatistikKaydetmeZamani = 0;
unsigned long sonHareketZamani = 0;        // Son hareket zamanı
bool hareketEdiyorMu = false;              // Hareket ediyor mu?
unsigned long hareketBaslangicZamani = 0;  // Hareket başlangıç zamanı

// Otonom tarama değişkenleri
bool taramaModu = false;
unsigned long taramaBaslangic = 0;
int enIyiYon = 0; // Derece cinsinden en iyi yön (0-360)
int enIyiMesafe = 0; // En iyi yöndeki mesafe

// YENİ - Tarama için değişkenler
bool taramaIcinDonusYapiliyor = false;
int taramaDereceleri[8] = {0}; // 8 farklı yön için mesafe ölçümü (0, 45, 90, 135, 180, 225, 270, 315 derece)
int taramaMesafeleri[8] = {0}; // Her yöndeki ölçülen mesafe
int taramaAdimi = 0; // Mevcut tarama adımı

// Temizlik planlama değişkenleri
bool zamanliTemizlikAktif = false;
int planliSaat = 0;
int planliDakika = 0;
int planliSure = 30; // dakika cinsinden

// EEPROM kullanımı
#include <EEPROM.h>
const int EEPROM_ADRES_TOPLAM_SURE = 0;
const int EEPROM_ADRES_MESAFE = 4;
const int EEPROM_ADRES_GECIS = 8;

// Mesafe sensörü ölçüm geçmişi
const int OLCUM_GECMISI_SAYISI = 5;
long onMesafeGecmisi[OLCUM_GECMISI_SAYISI] = {0};
long ustMesafeGecmisi[OLCUM_GECMISI_SAYISI] = {0};
int olcumGecmisiIndeksi = 0;

// Debug değişkenleri
unsigned long sonDebugMesajZamani = 0;
const unsigned long DEBUG_MESAJ_ARALIK = 10000; // Debug mesajlarını 10 saniyede bir göster (azaltıldı)

// YENİ - DFPlayer ve sensör test değişkenleri
unsigned long sonTestZamani = 0;

void setup() {
  // Motor pinlerini çıkış olarak ayarla
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Ultrasonik sensör pinlerini ayarla - SADECE ÖN SENSÖR KULLANILACAK
  pinMode(ON_TRIG_PIN, OUTPUT);
  pinMode(ON_ECHO_PIN, INPUT);
  
  // YENİ - Üst mesafe sensörü pinlerini ayarla
  pinMode(UST_TRIG_PIN, OUTPUT);
  pinMode(UST_ECHO_PIN, INPUT);
  
  // Buzzer pinini ayarla
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Başlangıçta kapalı
  
  // Vakum motoru röle pinini ayarla
  pinMode(VAKUM_ROLE, OUTPUT);
  digitalWrite(VAKUM_ROLE, LOW); // Başlangıçta kapalı
  
  // PIR sensör pini
  pinMode(PIR_PIN, INPUT);
  
  // YENİ - DFPlayer BUSY pini
  pinMode(DFPLAYER_BUSY_PIN, INPUT);
  
  // DHT11 sensörünü başlat
  dht.begin();
  
  // LCD ekranı başlat
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Akilli Supurge");
  lcd.setCursor(0, 1);
  lcd.print("Baslatiliyor...");
  
  // Ana seri port - ESP32 iletişimi için
  Serial.begin(115200);
  
  // YENİ - DFPlayer Mini için Hardware Serial kullanımı
  Serial1.begin(9600);
  
  // BME280 sensörünü başlat
  Wire.begin();
  
  if (!bme.begin(0x76)) { // BME280'in I2C adresi genellikle 0x76 veya 0x77'dir
    lcd.setCursor(0, 1);
    lcd.print("BME280 Hata!    ");
    delay(1000);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("BME280 OK!      ");
    delay(1000);
  }
  
  // YENİ - DFPlayer başlatma
  delay(1000); // İletişim başlaması için biraz bekle
  if (dfPlayer.begin(Serial1, true)) { // true parametresi hata ayıklama modunu açar
    lcd.setCursor(0, 1);
    lcd.print("DFPlayer OK     ");
    Serial.println("DFPlayer başarıyla başlatıldı");
    
    // Başlangıç ayarları
    dfPlayer.reset(); // Cihazı resetle
    delay(1000);
    dfPlayer.setTimeOut(500);
    dfPlayer.volume(15);  // 0-30 arası
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    
    // SD karttaki toplam şarkı sayısını al
    totalSongs = dfPlayer.readFileCounts();
    Serial.print("SD karttaki toplam şarkı sayısı: ");
    Serial.println(totalSongs);
    
    // Test için bir ses çal
    dfPlayer.play(1); // 01.mp3 dosyasını çal
    Serial.println("Test sesi çalınıyor...");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("DFPlayer Hata!  ");
    Serial.println("DFPlayer başlatılamadı!");
  }
  
  // YENİ - Dahili zaman başlatma
  dahiliZamanBaslangic = millis();
  setTime(0, 0, 0, 1, 1, 2023); // 1 Ocak 2023 saat 00:00:00 olarak başlat
  
  // İstatistikleri EEPROM'dan yükle
  EEPROM.get(EEPROM_ADRES_TOPLAM_SURE, toplamCalismaSuresi);
  EEPROM.get(EEPROM_ADRES_MESAFE, toplamGidilenMesafe);
  EEPROM.get(EEPROM_ADRES_GECIS, otomatikModGecis);
  
  // Motorları durdur
  durdur();
  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mod: Hazir");
  
  // İlk sensör okumaları
  onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
  
  // Başlangıç mesajı
  Serial.println("Arduino Mega hazır, komut bekleniyor!");
}

void loop() {
  // YENİ - Dahili zamanı güncelle (millis() tabanlı)
  if (millis() - dahiliZamanBaslangic >= 1000) {
    adjustTime(1); // Her 1 saniyede bir zamanı 1 saniye artır
    dahiliZamanBaslangic = millis();
  }

  // YENİ - Test komutlarını işle
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
    else {
      komutIsle(komut);
    }
  }
  
  // Zamanı kontrol et ve gerekirse planlı temizliği başlat
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
      
      // Planlı süre kadar çalışacak
      calismaBaslangicZamani = millis();
      
      // İstatistik güncelleme
      otomatikModGecis++;
      istatistikGuncellemesiGerekli = true;
    }
    
    // Planlı süre tamamlandıysa otomatiği kapat
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
      
      // Çalışma süresini güncelle
      long calismaSuresi = (millis() - calismaBaslangicZamani);
      toplamCalismaSuresi += calismaSuresi;
      istatistikGuncellemesiGerekli = true;
    }
  }
  
  // Sensörleri periyodik olarak oku
  if (millis() - sonSensorOkumaZamani > 2000) {
    sensorleriOku();
    sonSensorOkumaZamani = millis();
  }
  
  // Mesafe ölçümü periyodik olarak yap - SADECE ÖN MESAFE SENSÖRÜ İÇİN
  if (millis() - sonMesafeOlcmeZamani > 200) {
    onMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
    
    // LCD ekranda mesafeyi göster
    if (!bmeEkranGoster) {
      lcd.setCursor(0, 1);
      lcd.print("On:");
      lcd.print(onMesafe);
      lcd.print("cm        ");
    }
    
    sonMesafeOlcmeZamani = millis();
  }
  
  // İstatistikleri periyodik olarak kaydet (her 1 dakikada bir)
  if (istatistikGuncellemesiGerekli && millis() - sonIstatistikKaydetmeZamani > 60000) {
    istatistikleriKaydet();
    istatistikGuncellemesiGerekli = false;
    sonIstatistikKaydetmeZamani = millis();
  }
  
  // Sensör verilerini periyodik olarak gönder
  if (millis() - sonVeriGondermeZamani > 500) {
    sensorVerileriniGonder();
    sonVeriGondermeZamani = millis();
  }
  
  // BME280 verilerini periyodik olarak LCD'de göster
  if (millis() - sonBMEGostermeZamani > BME_GOSTERME_SURESI) {
    // BME280 değerlerini oku
    bmeSicaklik = bme.readTemperature();
    bmeNem = bme.readHumidity();
    bmeBasinc = bme.readPressure() / 100.0F; // hPa cinsinden
    
    // Ekranda BME280 değerlerini göster
    if (!otomatikMod && !manuelMod) { // Robot beklemede iken
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
      
      // 3 saniye sonra normal ekrana dön
      delay(3000);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mod: Hazir");
      bmeEkranGoster = false;
    }
    
    sonBMEGostermeZamani = millis();
  }
  
  // Debug mesajları (azaltıldı)
  if (millis() - sonDebugMesajZamani > DEBUG_MESAJ_ARALIK) {
    Serial.println("--- DEBUG: Ön Mesafe: " + String(onMesafe) + " cm, Tarama: " + String(taramaModu ? "AÇIK" : "KAPALI") + " ---");
    sonDebugMesajZamani = millis();
  }
  
  // YENİ - DFPlayer BUSY pin kontrol - Müzik durumunu güncelle
  muzikCaliyor = (digitalRead(DFPLAYER_BUSY_PIN) == LOW);
  
  // Mod durumuna göre işlem yap
  if (otomatikMod) {
    // Tarama modunda ise tarama yap, değilse normal otonom modu çalıştır
    if (taramaModu) {
      otonomTaramaYap();
    } else {
      // Otomatik modda 3 saniye hareketsiz kalırsa durdur
      if (hareketEdiyorMu) {
        // Son hareketten bu yana 3 saniye geçtiyse motoru durdur
        if (millis() - sonHareketZamani > 3000) {
          durdur();
          hareketEdiyorMu = false;
          lcd.setCursor(0, 1);
          lcd.print("Hareket yok!    ");
        } else {
          otomatikModCalistir();
        }
      } else {
        // Yeniden hareket etmeyi dene
        otomatikModCalistir();
        
        // Hareket başladığını işaretle
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
    
    // İstatistik güncelleme
    calismaBaslangicZamani = millis();
    otomatikModGecis++;
    istatistikGuncellemesiGerekli = true;
    
    // Hareket durumunu sıfırla
    hareketEdiyorMu = true;
    sonHareketZamani = millis();
    
    // Tarama modunu başlangıçta kapalı yap
    taramaModu = false;
  } 
  else if (komut == "OTOMATIK_KAPAT") {
    // Çalışma süresini güncelle
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
    
    // Tarama modunu kapat
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
    
    // İstatistik için
    calismaBaslangicZamani = millis();
  } 
  else if (komut == "MANUEL_KAPAT") {
    // Çalışma süresini güncelle
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
    
    // Manuel modda yön değişikliğini hemen uygula
    if (manuelMod) {
      switch (yon) {
        case 'F': // İleri (Forward)
          ileriGit(motorHizi);
          break;
        case 'B': // Geri (Backward)
          geriGit(motorHizi);
          break;
        case 'L': // Sol (Left)
          solaDon(motorHizi);
          break;
        case 'R': // Sağ (Right)
          sagaDon(motorHizi);
          break;
        case 'S': // Dur (Stop)
        default:
          durdur();
          break;
      }
    }
  }
  else if (komut.startsWith("HIZ:")) {
    motorHizi = komut.substring(4).toInt();
  }
  // DFPlayer komutları
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
  // Temizlik planlama komutları
  else if (komut.startsWith("PLANLI_TEMIZLIK_AYARLA:")) {
    // Format: PLANLI_TEMIZLIK_AYARLA:HH:MM:DD
    // HH: Saat, MM: Dakika, DD: Süre (dakika)
    String planBilgisi = komut.substring(23);
    
    int ilkNoktali = planBilgisi.indexOf(':');
    int ikinciNoktali = planBilgisi.lastIndexOf(':');
    
    if (ilkNoktali > 0 && ikinciNoktali > ilkNoktali) {
      planliSaat = planBilgisi.substring(0, ilkNoktali).toInt();
      planliDakika = planBilgisi.substring(ilkNoktali + 1, ikinciNoktali).toInt();
      planliSure = planBilgisi.substring(ikinciNoktali + 1).toInt();
      
      zamanliTemizlikAktif = true;
      Serial.println("PLANLI_TEMIZLIK_AYARLANDI");
      
      // LCD'de göster
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
  // YENİ - Zaman ayarlama komutu
  else if (komut.startsWith("ZAMAN_AYARLA:")) {
    // Format: ZAMAN_AYARLA:HH:MM:SS
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
      
      // LCD'de göster
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
  // YENİ - Tarih ayarlama komutu
  else if (komut.startsWith("TARIH_AYARLA:")) {
    // Format: TARIH_AYARLA:GG:AA:YYYY
    String tarihBilgisi = komut.substring(13);
    
    int ilkNoktali = tarihBilgisi.indexOf(':');
    int ikinciNoktali = tarihBilgisi.lastIndexOf(':');
    
    if (ilkNoktali > 0 && ikinciNoktali > ilkNoktali) {
      int gun = tarihBilgisi.substring(0, ilkNoktali).toInt();
      int ay = tarihBilgisi.substring(ilkNoktali + 1, ikinciNoktali).toInt();
      int yil = tarihBilgisi.substring(ikinciNoktali + 1).toInt();
      
      setTime(hour(), minute(), second(), gun, ay, yil);
      
      Serial.println("TARIH_AYARLANDI");
      
      // LCD'de göster
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

// YENİ - DFPlayer Debug Fonksiyonu
void dfPlayerDebug() {
  Serial.println("DFPlayer durumu kontrol ediliyor...");
  
  // BUSY pin durumunu kontrol et
  Serial.print("BUSY pin durumu: ");
  Serial.println(digitalRead(DFPLAYER_BUSY_PIN) == LOW ? "Çalışıyor" : "Beklemede");
  
  // SD kart durumunu kontrol et
  Serial.print("SD kart kontrolü: ");
  bool sdOk = false;
  
  // DFPlayer SD kart kontrolü
  uint8_t tip = dfPlayer.readType();
  uint16_t deger = dfPlayer.read();
  
  if (dfPlayer.available()) {
    sdOk = true;
    Serial.println("Bağlı");
    
    // SD kart içeriğini kontrol et
    int dosyaSayisi = dfPlayer.readFileCounts();
    Serial.print("Toplam dosya sayısı: ");
    Serial.println(dosyaSayisi);
    
    // Ses seviyesini kontrol et
    int sesSeviyesi = dfPlayer.readVolume();
    Serial.print("Ses seviyesi: ");
    Serial.println(sesSeviyesi);
    
    // Mevcut şarkıyı kontrol et
    int suan = dfPlayer.readCurrentFileNumber();
    Serial.print("Çalan dosya: ");
    Serial.println(suan);
  } else {
    Serial.println("Yanıt vermiyor!");
    Serial.println("DFPlayer'ı yeniden başlatmayı deneyin...");
    
    // DFPlayer'ı yeniden başlat
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

// YENİ - Sensör Kontrol Fonksiyonu
void sensorKontrol() {
  Serial.println("\n----- SENSÖR DURUMU -----");
  
  // DHT11 Sıcaklık/Nem sensörü kontrolü
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
  
  // MQ2 Gaz sensörü kontrolü
  int gazDegeri = analogRead(MQ2_PIN);
  Serial.print("MQ2 Gaz sensörü (A0): ");
  Serial.print(gazDegeri);
  if (gazDegeri < 50) {
    Serial.println(" - UYARI: Çok düşük değer, bağlantıyı kontrol edin");
  } else {
    Serial.println(" - Normal");
  }
  
  // PIR Hareket sensörü kontrolü
  int pirDurum = digitalRead(PIR_PIN);
  Serial.print("PIR Hareket sensörü (PIN 3): ");
  Serial.println(pirDurum == HIGH ? "Hareket Algılandı" : "Hareket Yok");
  
  // Ultrasonik mesafe sensörü kontrolü
  long mesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
  Serial.print("Ön Mesafe sensörü (PIN 28,29): ");
  Serial.print(mesafe);
  Serial.println(" cm");
  
  // Ses sensörü kontrolü
  int sesDegeri = analogRead(SES_SENSOR_PIN);
  Serial.print("Ses sensörü (A1): ");
  Serial.println(sesDegeri);
  
  // BME280 sensörü kontrolü
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

// YENİ - İyileştirilmiş otonom mod fonksiyonu - ÜST SENSÖR KALDIRILDI
void otomatikModCalistir() {
  // Motor hızını ayarla - Yavaş motorla başla
  int currentHiz = yavasMotorHizi;
  
  // Sadece ön mesafe sensörünü kontrol et (15 cm'de engel algılanırsa tarama yap)
  if (onMesafe <= 15) {
    // Ön sensör engel algıladı
    
    // Durup sesli uyarı ver
    durdur();
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Biraz geri git
    geriGit(currentHiz);
    delay(1000); // 1 saniye geri git
    durdur();
    delay(300);
    
    // Tarama modunu başlat
    taramaModu = true;
    taramaBaslangic = millis();
    
    return; // Fonksiyondan çık, tarama modu loop'ta işlenecek
  }
  
  // Hiçbir engel yoksa ileri git
  ileriGit(currentHiz);
  
  // İstatistik için mesafe hesaplama (yaklaşık)
  toplamGidilenMesafe += 0.0002;
  istatistikGuncellemesiGerekli = true;
  
  // Hareket ettiğini belirt ve zaman damgasını güncelle
  sonHareketZamani = millis();
  hareketEdiyorMu = true;
}

// İyileştirilmiş tarama fonksiyonu - Dönüş problemi çözüldü
void otonomTaramaYap() {
  static unsigned long sonDonusZamani = 0;
  static int donusSuresi = 0;
  
  // Tarama başlangıcında değişkenleri sıfırla
  if (millis() - taramaBaslangic < 100) {
    // Tarama başlangıcında değerleri sıfırla
    
    // Tüm tarama değişkenlerini sıfırla
    taramaAdimi = 0;
    enIyiYon = 0;
    enIyiMesafe = 0;
    
    // İlk adım için dönüş başlat
    sonDonusZamani = millis();
    
    // Sesli uyarı
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    
    lcd.setCursor(0, 1);
    lcd.print("Tarama yapiliyor");
  }
  
  // Tüm 8 yöne sırayla bak (0-360 derece, 45'er derece aralıklarla)
  if (taramaAdimi < 8) {
    // Her yöne 45 derece dönüş yaparak bakar (8 x 45 = 360 derece)
    
    // Dönüş sürecindeyiz
    if (millis() - sonDonusZamani < 500) {
      // Her adımda sadece sağa dön (daha tutarlı davranış için)
      sagaDon(donmeMotorHizi);
    } 
    // Dönüş tamamlandı, durup ölçüm yap
    else if (millis() - sonDonusZamani < 1000) {
      durdur();
      
      // Durup ölçüm yap
      if (millis() - sonDonusZamani >= 700) {
        // Mesafeyi ölç ve kaydet
        long olculenMesafe = mesafeOlc(ON_TRIG_PIN, ON_ECHO_PIN);
        taramaMesafeleri[taramaAdimi] = olculenMesafe;
        
        // En iyi mesafeyi güncelle (15 cm'den büyük olmalı)
        if (olculenMesafe > enIyiMesafe && olculenMesafe > 15) {
          enIyiMesafe = olculenMesafe;
          enIyiYon = taramaAdimi;
        }
      }
    }
    // Bu adım tamamlandı, bir sonraki adıma geç
    else {
      taramaAdimi++;
      sonDonusZamani = millis();
    }
  }
  // Tüm yönlere baktıktan sonra en iyi yöne dön
  else if (taramaAdimi == 8) {
    // 8 adım tamamlandı, en iyi yöne dön
    
    // En iyi yön bulunamadıysa (tüm yönler 15 cm'den az) ileri yönü seç
    if (enIyiMesafe <= 15) {
      // Tüm yönler kapalıysa biraz daha geri git ve tekrar taramayı dene
      geriGit(yavasMotorHizi);
      delay(1000); // 1 saniye daha geri git
      durdur();
      delay(300);
      
      // Taramayı sıfırla
      taramaAdimi = 0;
      enIyiYon = 0;
      enIyiMesafe = 0;
      sonDonusZamani = millis();
      
      return;
    }
    
    // En iyi yön belirlendi, ona dön
    taramaAdimi = 9;
    sonDonusZamani = millis();
    // Her yön için 45 derece, enIyiYon adım sayısını belirtir
    donusSuresi = 500 * enIyiYon; // Her adım için 500ms dönüş süresi
  }
  // En iyi yöne dönüyoruz
  else if (taramaAdimi == 9) {
    if (millis() - sonDonusZamani < donusSuresi) {
      // En iyi yöne dönüş
      sagaDon(donmeMotorHizi);
    } else {
      // Dönüş tamamlandı, taramayı bitir
      durdur();
      delay(300);
      
      // Taramayı bitir ve normal moda geç
      taramaModu = false;
      
      lcd.setCursor(0, 1);
      lcd.print("Tarama tamam!   ");
      
      // Sesli uyarı (iki kısa bip)
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      delay(100);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      
      // Tarama sonrası ileri git
      ileriGit(yavasMotorHizi);
    }
  }
}

void manuelModCalistir() {
  // Manuel mod aktif olduğunda her zaman belirli bir frekansta yön kontrolü yap
  static unsigned long sonYonGuncelleme = 0;
  if (millis() - sonYonGuncelleme > 100) { // Her 100 ms'de bir yön güncelle
    switch (yon) {
      case 'F': // İleri (Forward)
        ileriGit(motorHizi);
        
        // Manuel modda da mesafe hesapla
        // Motorun hızına göre katsayı belirle (motorHizi / 120)
        float hizKatsayi = motorHizi / 120.0;
        toplamGidilenMesafe += 0.0002 * hizKatsayi;
        istatistikGuncellemesiGerekli = true;
        break;
      case 'B': // Geri (Backward)
        geriGit(motorHizi);
        // Geri giderken de mesafe hesapla
        hizKatsayi = motorHizi / 120.0;
        toplamGidilenMesafe += 0.0002 * hizKatsayi;
        istatistikGuncellemesiGerekli = true;
        break;
      case 'L': // Sol (Left)
        solaDon(motorHizi);
        break;
      case 'R': // Sağ (Right)
        sagaDon(motorHizi);
        break;
      case 'S': // Dur (Stop)
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
  
  long sure = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (sure == 0) return 100; // Timeout durumunda güvenli bir değer döndür
  
  long mesafe = sure * 0.034 / 2; // cm cinsinden mesafe
  
  // Ölçümün makul bir aralıkta olduğundan emin ol
  if (mesafe > 400) mesafe = 400; // HC-SR04'ün maksimum ölçüm aralığı
  if (mesafe <= 0) mesafe = 100;  // Hatalı ölçüm durumu
  
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
  digitalWrite(VAKUM_ROLE, HIGH);
  vakumDurumu = true;
  Serial.println("VAKUM:ACIK");
}

void vakumKapat() {
  digitalWrite(VAKUM_ROLE, LOW);
  vakumDurumu = false;
  Serial.println("VAKUM:KAPALI");
}

// YENİ - İyileştirilmiş DFPlayer Mini için müzik kontrol fonksiyonları
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
  // DHT11 sıcaklık ve nem
  sicaklik = dht.readTemperature();
  nem = dht.readHumidity();
  
  // MQ2 gaz sensörü
  gazSeviyesi = analogRead(MQ2_PIN);
  
  // PIR sensörü
  hareketAlgilandi = digitalRead(PIR_PIN);
  
  // Ses sensörü
  sesSeviyesi = analogRead(SES_SENSOR_PIN);
  
  // BME280 verilerini okuma
  bmeSicaklik = bme.readTemperature();
  bmeNem = bme.readHumidity();
  bmeBasinc = bme.readPressure() / 100.0F; // hPa cinsinden
}

void sensorVerileriniGonder() {
  // JSON formatında sensör verilerini gönder - Yeni uyumlu format
  Serial.print("SENSOR_DATA:{");
  
  // Mesafe sensörleri - Sadece ön sensör
  Serial.print("\"on\":");
  Serial.print(onMesafe);
  
  // Diğer sensörler
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
  
  // BME280 sensör verileri
  Serial.print(",\"bmeSicaklik\":");
  Serial.print(bmeSicaklik);
  Serial.print(",\"bmeNem\":");
  Serial.print(bmeNem);
  Serial.print(",\"bmeBasinc\":");
  Serial.print(bmeBasinc);
  
  // Durum bilgileri
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
  
  // Zaman bilgileri
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
  
  // Zamanlayıcı bilgileri
  Serial.print(",\"planliTemizlik\":");
  Serial.print(zamanliTemizlikAktif ? "true" : "false");
  
  // İstatistik bilgileri
  Serial.print(",\"toplamSureDk\":");
  Serial.print(toplamCalismaSuresi / 60000); // Dakika cinsinden
  Serial.print(",\"toplamMesafe\":");
  Serial.print(toplamGidilenMesafe);
  Serial.print(",\"otomatikGecis\":");
  Serial.print(otomatikModGecis);
  
  // JSON sonlandır
  Serial.println("}");
}

void istatistikleriKaydet() {
  // EEPROM'a istatistikleri kaydet
  EEPROM.put(EEPROM_ADRES_TOPLAM_SURE, toplamCalismaSuresi);
  EEPROM.put(EEPROM_ADRES_MESAFE, toplamGidilenMesafe);
  EEPROM.put(EEPROM_ADRES_GECIS, otomatikModGecis);
}