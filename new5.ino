#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "time.h"

// WiFi Bilgileri
const char* ssid = "Badirinphone";
const char* password = "Badir8186";

// NTP Sunucu Bilgileri
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800; // Türkiye için UTC+3 (saniye cinsinden): 3*60*60
const int daylightOffset_sec = 0;

// Kamera pinleri - ESP32-CAM AI-THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Global değişkenler
bool otomatikMod = false;
bool manuelMod = false;
bool vakumDurumu = false;
bool muzikCaliyor = false;
int currentTrack = 1;

// Mesafe sensör değerleri - Artık sadece on ve ust sensörü var
int onMesafe = 0;
int ustMesafe = 0;

// Eski arayüz uyumluluğu için saklanan değişkenler
int onSagMesafe = 0;
int onSolMesafe = 0;
int sagYanMesafe = 0;
int solYanMesafe = 0;

float sicaklik = 0;
float nem = 0;
int gazSeviyesi = 0;
bool hareketAlgilandi = false;
int sesSeviyesi = 0;
int motorHizi = 200;

// İstatistik değerleri
int toplamCalismaSuresi = 0; // dakika cinsinden
float toplamGidilenMesafe = 0.0; // metre cinsinden
int otomatikModGecis = 0;

// Temizlik planlama değişkenleri
bool planliTemizlikAktif = false;
int planliSaat = 0;
int planliDakika = 0;
int planliSure = 30; // dakika cinsinden

// Hareketsizlik tespit değişkenleri
bool robotHareketEdiyor = false;
unsigned long sonHareketZamani = 0;

// YENİ - Zaman senkronizasyonu değişkenleri
unsigned long sonZamanGondermeZamani = 0;
const unsigned long ZAMAN_GONDER_ARALIK = 43200000; // 12 saat (milisaniye)

// Debug değişkenleri
unsigned long sonDebugGondermeZamani = 0;
const unsigned long DEBUG_GONDER_ARALIK = 10000; // 10 saniye (milisaniye)

// Şarkı listesi
const int SONG_COUNT = 5;
String songNames[SONG_COUNT] = {
  "1. Sarki Adi", "2. Sarki Adi", "3. Sarki Adi", "4. Sarki Adi", "5. Sarki Adi"
};

// Asenkron Web Sunucusu
AsyncWebServer server(80);

// HTML sayfası - açıklama ve kodda okunurluk için daha kısa tutuldu
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Akıllı Süpürge Kontrol Paneli</title>
    <style>
        :root {--primary-color: #4C84FF;--secondary-color: #3366CC;--success-color: #4CAF50;--danger-color: #F44336;--warning-color: #FFC107;--info-color: #00BCD4;--dark-color: #333;--light-color: #f4f4f4;--text-color: #333;}
        * {box-sizing: border-box;margin: 0;padding: 0;font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;}
        body {background-color: #f9f9f9;color: var(--text-color);line-height: 1.6;}
        .container {max-width: 1200px;margin: 0 auto;padding: 1rem;}
        header {background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));color: white;padding: 1rem;text-align: center;border-radius: 10px 10px 0 0;box-shadow: 0 2px 10px rgba(0,0,0,0.1);margin-bottom: 1rem;}
        header h1 {margin: 0;font-size: 1.8rem;}
        .status-panel {background-color: white;border-radius: 10px;padding: 1rem;margin: 1rem 0;box-shadow: 0 2px 10px rgba(0,0,0,0.05);}
        .status-item {display: flex;justify-content: space-between;align-items: center;margin-bottom: 0.5rem;padding: 0.5rem;border-bottom: 1px solid #eee;}
        .status-label {font-weight: bold;}
        .status-value {padding: 0.25rem 0.5rem;border-radius: 4px;color: white;font-weight: bold;min-width: 80px;text-align: center;}
        .active {background-color: var(--success-color);}
        .inactive {background-color: var(--danger-color);}
        .main-content {display: flex;flex-wrap: wrap;gap: 1rem;}
        .control-panel, .camera-panel, .music-panel, .schedule-panel {flex: 1;min-width: 300px;background-color: white;border-radius: 10px;padding: 1rem;box-shadow: 0 2px 10px rgba(0,0,0,0.05);margin-bottom: 1rem;}
        .control-section {margin-bottom: 2rem;}
        .control-section h3 {margin-bottom: 1rem;color: var(--primary-color);border-bottom: 2px solid var(--primary-color);padding-bottom: 0.5rem;}
        .btn {padding: 0.5rem 1rem;border: none;border-radius: 4px;cursor: pointer;font-weight: bold;transition: all 0.3s ease;margin: 0.25rem;min-width: 100px;box-shadow: 0 2px 5px rgba(0,0,0,0.1);}
        .btn:hover {transform: translateY(-2px);box-shadow: 0 4px 8px rgba(0,0,0,0.15);}
        .btn:active {transform: translateY(0);}
        .btn-primary {background-color: var(--primary-color);color: white;}
        .btn-secondary {background-color: var(--secondary-color);color: white;}
        .btn-danger {background-color: var(--danger-color);color: white;}
        .btn-success {background-color: var(--success-color);color: white;}
        .btn-warning {background-color: var(--warning-color);color: white;}
        .btn-info {background-color: var(--info-color);color: white;}
        .btn-dark {background-color: var(--dark-color);color: white;}
        .direction-controls {display: grid;grid-template-columns: repeat(3, 1fr);grid-template-rows: repeat(3, 1fr);gap: 0.5rem;max-width: 300px;margin: 0 auto;}
        .direction-btn {padding: 1rem;font-size: 1.2rem;border-radius: 8px;background-color: var(--secondary-color);color: white;cursor: pointer;border: none;transition: all 0.3s ease;box-shadow: 0 2px 5px rgba(0,0,0,0.1);}
        .direction-btn:hover {background-color: var(--primary-color);transform: scale(1.05);}
        .direction-btn:active {transform: scale(0.98);}
        #stopBtn {grid-column: 2;grid-row: 2;background-color: var(--danger-color);}
        #forwardBtn {grid-column: 2;grid-row: 1;}
        #backBtn {grid-column: 2;grid-row: 3;}
        #leftBtn {grid-column: 1;grid-row: 2;}
        #rightBtn {grid-column: 3;grid-row: 2;}
        .camera-view {text-align: center;margin-bottom: 1rem;}
        .camera-stream {width: 100%;max-height: 480px;border-radius: 10px;object-fit: contain;border: 1px solid #ddd;box-shadow: 0 2px 10px rgba(0,0,0,0.1);}
        .sensor-panel {margin-top: 1rem;display: grid;grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));gap: 1rem;padding: 1rem;background-color: white;border-radius: 10px;box-shadow: 0 2px 10px rgba(0,0,0,0.05);}
        .sensor-box {padding: 1rem;border-radius: 8px;background: #f9f9f9;text-align: center;box-shadow: 0 2px 5px rgba(0,0,0,0.05);transition: all 0.3s ease;}
        .sensor-box:hover {transform: translateY(-5px);box-shadow: 0 5px 15px rgba(0,0,0,0.1);}
        .sensor-value {font-size: 1.5rem;font-weight: bold;color: var(--primary-color);margin-top: 0.5rem;}
        .slider-container {margin: 2rem 0;padding: 1rem;background: #f9f9f9;border-radius: 8px;box-shadow: 0 2px 5px rgba(0,0,0,0.05);}
        .slider {-webkit-appearance: none;width: 100%;height: 15px;border-radius: 10px;background: #d3d3d3;outline: none;margin: 1rem 0;}
        .slider::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 25px;height: 25px;border-radius: 50%;background: var(--primary-color);cursor: pointer;box-shadow: 0 2px 5px rgba(0,0,0,0.2);}
        .slider::-moz-range-thumb {width: 25px;height: 25px;border-radius: 50%;background: var(--primary-color);cursor: pointer;box-shadow: 0 2px 5px rgba(0,0,0,0.2);}
        .slider-value {text-align: center;font-size: 1.2rem;font-weight: bold;color: var(--primary-color);}
        .music-player {display: flex;flex-direction: column;align-items: center;padding: 1rem;background: linear-gradient(135deg, #f4f9ff, #e6f0ff);border-radius: 10px;box-shadow: 0 4px 15px rgba(0,0,0,0.08);}
        .music-info {width: 100%;text-align: center;padding: 1rem;margin-bottom: 1rem;background: white;border-radius: 10px;box-shadow: inset 0 0 10px rgba(0,0,0,0.05);}
        .music-info h4 {color: var(--primary-color);margin-bottom: 0.5rem;}
        .now-playing {font-size: 1.2rem;font-weight: bold;margin-bottom: 0.5rem;color: var(--text-color);}
        .music-status {color: var(--success-color);font-weight: bold;}
        .music-controls {display: flex;justify-content: center;gap: 0.5rem;margin-bottom: 1rem;width: 100%;}
        .music-controls .btn {font-size: 1.2rem;padding: 0.8rem 1.2rem;border-radius: 50%;width: 50px;height: 50px;display: flex;justify-content: center;align-items: center;min-width: auto;}
        .song-list {width: 100%;margin-top: 1rem;}
        .song-item {padding: 0.8rem;background: white;border-radius: 5px;margin-bottom: 0.5rem;display: flex;justify-content: space-between;align-items: center;cursor: pointer;transition: all 0.2s ease;border-left: 4px solid transparent;}
        .song-item:hover {background: #f0f5ff;border-left-color: var(--primary-color);}
        .song-item.active {background: #e6f0ff;border-left-color: var(--success-color);font-weight: bold;}
        .play-song-btn {padding: 0.3rem 0.6rem;background: var(--primary-color);color: white;border: none;border-radius: 4px;cursor: pointer;transition: all 0.2s ease;}
        .play-song-btn:hover {background: var(--secondary-color);}

        /* Planlı Temizlik Stilleri */
        .schedule-form {display: flex;flex-direction: column;gap: 1rem;margin-top: 1rem;}
        .form-row {display: flex;justify-content: space-between;gap: 1rem;align-items: center;}
        .form-group {display: flex;flex-direction: column;gap: 0.5rem;flex: 1;}
        .form-group label {font-weight: bold;}
        .form-group input, .form-group select {padding: 0.5rem;border: 1px solid #ddd;border-radius: 4px;}
        .schedule-status {margin-top: 1rem;padding: 1rem;border-radius: 8px;background: #f9f9f9;text-align: center;}
        .schedule-active {color: var(--success-color);font-weight: bold;}
        .schedule-inactive {color: var(--danger-color);font-weight: bold;}
        
        /* İstatistik Paneli Stilleri */
        .stats-panel {margin-top: 1rem;background-color: white;border-radius: 10px;padding: 1rem;box-shadow: 0 2px 10px rgba(0,0,0,0.05);}
        .stats-item {padding: 1rem;margin-bottom: 1rem;border-radius: 8px;background: linear-gradient(135deg, #f9f9f9, #f4f4f4);text-align: center;}
        .stats-value {font-size: 2rem;font-weight: bold;color: var(--primary-color);margin: 0.5rem 0;}
        .stats-label {color: var(--dark-color);font-weight: bold;}
        .stats-grid {display: grid;grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));gap: 1rem;}

        /* Robot durum indikatörü */
        .robot-status {padding: 0.5rem 1rem;margin: 0.5rem 0;border-radius: 4px;text-align: center;font-weight: bold;}
        .robot-moving {background-color: #e7f7e7;color: var(--success-color);}
        .robot-stationary {background-color: #fff0e7;color: var(--warning-color);}
        
        @media (max-width: 768px) {
            .main-content {flex-direction: column;}
            .direction-controls {max-width: 100%;}
            .camera-stream {max-height: 300px;}
            .sensor-panel {grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));}
            .music-controls .btn {width: 40px;height: 40px;font-size: 1rem;}
            .stats-grid {grid-template-columns: 1fr;}
            .form-row {flex-direction: column;}
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Akıllı Süpürge Robot Kontrol Paneli</h1>
        </header>
        
        <div class="status-panel">
            <div class="status-item">
                <span class="status-label">Otomatik Mod:</span>
                <span id="otomatikStatus" class="status-value inactive">Kapalı</span>
            </div>
            <div class="status-item">
                <span class="status-label">Manuel Mod:</span>
                <span id="manuelStatus" class="status-value inactive">Kapalı</span>
            </div>
            <div class="status-item">
                <span class="status-label">Vakum Motoru:</span>
                <span id="vakumStatus" class="status-value inactive">Kapalı</span>
            </div>
            <div class="status-item">
                <span class="status-label">Planlı Temizlik:</span>
                <span id="planStatus" class="status-value inactive">Kapalı</span>
            </div>
            <div class="robot-status" id="robotMovementStatus">
                Robot Durum: Hazır
            </div>
        </div>
        
        <div class="main-content">
            <div class="camera-panel">
                <h3>Kamera Goruntusu</h3>                
                <div class="camera-view">
                  <img src="/stream" alt="Kamera Yayını" class="camera-stream" id="cameraStream">
                </div>
            </div>
            
            <div class="control-panel">
                <div class="control-section">
                    <h3>Çalışma Modu</h3>
                    <button id="otomatikAcBtn" class="btn btn-primary">Otomatik Mod Aç</button>
                    <button id="otomatikKapatBtn" class="btn btn-danger">Otomatik Mod Kapat</button>
                    <button id="manuelAcBtn" class="btn btn-primary">Manuel Mod Aç</button>
                    <button id="manuelKapatBtn" class="btn btn-danger">Manuel Mod Kapat</button>
                </div>
                
                <div class="control-section">
                    <h3>Vakum Kontrolü</h3>
                    <button id="vakumAcBtn" class="btn btn-success">Vakum Aç</button>
                    <button id="vakumKapatBtn" class="btn btn-danger">Vakum Kapat</button>
                </div>
                
                <div class="slider-container">
                    <h3>Motor Hızı</h3>
                    <input type="range" min="50" max="255" value="200" class="slider" id="hizSlider">
                    <div class="slider-value">
                        <span id="hizDeger">200</span>
                    </div>
                </div>
                
                <div class="control-section">
                    <h3>Manuel Yön Kontrolü</h3>
                    <div class="direction-controls">
                        <button id="forwardBtn" class="direction-btn">↑</button>
                        <button id="leftBtn" class="direction-btn">←</button>
                        <button id="stopBtn" class="direction-btn">■</button>
                        <button id="rightBtn" class="direction-btn">→</button>
                        <button id="backBtn" class="direction-btn">↓</button>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- Planlı Temizlik Paneli -->
        <div class="schedule-panel">
            <h3>Planlı Temizlik</h3>
            <div class="schedule-form">
                <div class="form-row">
                    <div class="form-group">
                        <label for="scheduleHour">Saat:</label>
                        <select id="scheduleHour">
                            <!-- JavaScript ile doldurulacak -->
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="scheduleMinute">Dakika:</label>
                        <select id="scheduleMinute">
                            <!-- JavaScript ile doldurulacak -->
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="scheduleDuration">Süre (dakika):</label>
                        <select id="scheduleDuration">
                            <option value="15">15</option>
                            <option value="30" selected>30</option>
                            <option value="45">45</option>
                            <option value="60">60</option>
                            <option value="90">90</option>
                            <option value="120">120</option>
                        </select>
                    </div>
                </div>
                <div class="form-row">
                    <button id="planliTemizlikAyarlaBtn" class="btn btn-primary">Planlı Temizlik Ayarla</button>
                    <button id="planliTemizlikIptalBtn" class="btn btn-danger">Planlı Temizliği İptal Et</button>
                </div>
            </div>
            <div class="schedule-status" id="scheduleStatus">
                <span class="schedule-inactive">Planlı temizlik ayarlanmadı</span>
            </div>
        </div>
        
        <!-- İstatistik Paneli -->
        <div class="stats-panel">
            <h3>İstatistikler</h3>
            <div class="stats-grid">
                <div class="stats-item">
                    <div class="stats-label">Toplam Çalışma Süresi</div>
                    <div id="toplamSure" class="stats-value">0 dk</div>
                </div>
                <div class="stats-item">
                    <div class="stats-label">Toplam Gidilen Mesafe</div>
                    <div id="toplamMesafe" class="stats-value">0 m</div>
                </div>
                <div class="stats-item">
                    <div class="stats-label">Otomatik Moda Geçiş</div>
                    <div id="otomatikGecis" class="stats-value">0 kere</div>
                </div>
            </div>
            <div class="form-row" style="margin-top: 1rem; justify-content: center;">
                <button id="istatistikSifirlaBtn" class="btn btn-warning">İstatistikleri Sıfırla</button>
            </div>
        </div>
        
        <!-- Müzik Kontrol Paneli -->
        <div class="music-panel">
            <h3>Müzik Kontrolü</h3>
            <div class="music-player">
                <div class="music-info">
                    <h4>Şu An Çalan:</h4>
                    <div id="nowPlaying" class="now-playing">Şarkı seçilmedi</div>
                    <div id="musicStatus" class="music-status">Duraklatıldı</div>
                </div>
                
                <div class="music-controls">
                    <button id="prevBtn" class="btn btn-secondary">◄◄</button>
                    <button id="playBtn" class="btn btn-success">►</button>
                    <button id="pauseBtn" class="btn btn-warning">❚❚</button>
                    <button id="nextBtn" class="btn btn-secondary">►►</button>
                </div>
                
                <div class="slider-container">
                    <h4>Ses Seviyesi</h4>
                    <input type="range" min="0" max="30" value="15" class="slider" id="volumeSlider">
                    <div class="slider-value">
                        <span id="volumeDeger">15</span>
                    </div>
                </div>
                
                <div class="song-list" id="songList">
                    <!-- Şarkı listesi JavaScript ile doldurulacak -->
                </div>
            </div>
        </div>
        
        <div class="sensor-panel">
            <div class="sensor-box">
                <h4>Ön Sağ Mesafe</h4>
                <div id="onSagValue" class="sensor-value">0 cm</div>
            </div>
            <div class="sensor-box">
                <h4>Ön Sol Mesafe</h4>
                <div id="onSolValue" class="sensor-value">0 cm</div>
            </div>
            <div class="sensor-box">
                <h4>Sağ Yan Mesafe</h4>
                <div id="sagYanValue" class="sensor-value">0 cm</div>
            </div>
            <div class="sensor-box">
                <h4>Sol Yan Mesafe</h4>
                <div id="solYanValue" class="sensor-value">0 cm</div>
            </div>
            <div class="sensor-box">
                <h4>Üst Mesafe</h4>
                <div id="ustValue" class="sensor-value">0 cm</div>
            </div>
            <div class="sensor-box">
                <h4>Sıcaklık</h4>
                <div id="sicaklikValue" class="sensor-value">0 °C</div>
            </div>
            <div class="sensor-box">
                <h4>Nem</h4>
                <div id="nemValue" class="sensor-value">0 %</div>
            </div>
            <div class="sensor-box">
                <h4>Gaz Seviyesi</h4>
                <div id="gazValue" class="sensor-value">0</div>
            </div>
            <div class="sensor-box">
                <h4>Hareket</h4>
                <div id="hareketValue" class="sensor-value">Yok</div>
            </div>
            <div class="sensor-box">
                <h4>Ses Seviyesi</h4>
                <div id="sesValue" class="sensor-value">0</div>
            </div>
        </div>
    </div>
    
    <script>
        // Global değişkenler
        var motorHizi = 200;
        var volumeLevel = 15;
        var currentTrack = 1;
        var isPlaying = false;
        var yonTutucu = null;
        var planliTemizlikAktif = false;
        var planliSaat = 0;
        var planliDakika = 0;
        var planliSure = 30;
        var robotHareketEdiyor = false;
        var sonCommandZamani = 0;
        
        // Şarkı listesi
        var songs = [
            "1. Sarki Adi",
            "2. Sarki Adi",
            "3. Sarki Adi", 
            "4. Sarki Adi", 
            "5. Sarki Adi"
        ];
        
        // Sayfa yüklendiğinde çalışacak
        document.addEventListener('DOMContentLoaded', function() {
            // İlk durumu güncelle
            updateStatus();
            
            // Şarkı listesini oluştur
            createSongList();
            
            // Saat ve dakika seçeneklerini doldur
            populateTimeOptions();
            
            // Eventleri ayarla
            setupEventListeners();
        });
        
        // Zaman seçeneklerini doldurma
        function populateTimeOptions() {
            var hourSelect = document.getElementById('scheduleHour');
            var minuteSelect = document.getElementById('scheduleMinute');
            
            // Saatleri doldur (0-23)
            for (var i = 0; i < 24; i++) {
                var hourOption = document.createElement('option');
                hourOption.value = i;
                hourOption.textContent = i < 10 ? '0' + i : i;
                hourSelect.appendChild(hourOption);
            }
            
            // Dakikaları doldur (0-59, 5'er dakika)
            for (var j = 0; j < 60; j += 5) {
                var minuteOption = document.createElement('option');
                minuteOption.value = j;
                minuteOption.textContent = j < 10 ? '0' + j : j;
                minuteSelect.appendChild(minuteOption);
            }
        }
        
        // Şarkı listesi oluşturma
        function createSongList() {
            var songListElement = document.getElementById('songList');
            songListElement.innerHTML = '';
            
            for (var i = 0; i < songs.length; i++) {
                var songItem = document.createElement('div');
                songItem.className = 'song-item';
                songItem.dataset.id = i + 1;
                
                if (i + 1 === currentTrack) {
                    songItem.classList.add('active');
                }
                
                songItem.innerHTML = '<span>' + songs[i] + '</span><button class="play-song-btn">Çal</button>';
                
                songListElement.appendChild(songItem);
            }
            
            // Şarkı çalma butonlarına event ekle
            var buttons = document.querySelectorAll('.play-song-btn');
            for (var j = 0; j < buttons.length; j++) {
                buttons[j].addEventListener('click', function(e) {
                    var trackId = parseInt(e.target.parentElement.dataset.id);
                    sendCommand('muzik_track:' + trackId);
                    currentTrack = trackId;
                    updateMusicUI();
                });
            }
        }
        
        // Event listener kurulumu
        function setupEventListeners() {
            // Çalışma modu butonları
            document.getElementById('otomatikAcBtn').addEventListener('click', function() { sendCommand('otomatik_ac'); });
            document.getElementById('otomatikKapatBtn').addEventListener('click', function() { sendCommand('otomatik_kapat'); });
            document.getElementById('manuelAcBtn').addEventListener('click', function() { sendCommand('manuel_ac'); });
            document.getElementById('manuelKapatBtn').addEventListener('click', function() { sendCommand('manuel_kapat'); });
            
            // Vakum butonları
            document.getElementById('vakumAcBtn').addEventListener('click', function() { sendCommand('vakum_ac'); });
            document.getElementById('vakumKapatBtn').addEventListener('click', function() { sendCommand('vakum_kapat'); });
            
            // Hız slider
            var slider = document.getElementById('hizSlider');
            var sliderValue = document.getElementById('hizDeger');
            
            slider.oninput = function() {
                motorHizi = this.value;
                sliderValue.textContent = motorHizi;
                sendCommand('hiz:' + motorHizi);
            };
            
            // Ses slider
            var volumeSlider = document.getElementById('volumeSlider');
            var volumeValue = document.getElementById('volumeDeger');
            
            volumeSlider.oninput = function() {
                volumeLevel = this.value;
                volumeValue.textContent = volumeLevel;
                sendCommand('muzik_volume:' + volumeLevel);
            };
            
            // Yön tuşları - dokunmatik arayüz için güçlendirilmiş kontrol
            document.getElementById('forwardBtn').addEventListener('mousedown', function() {
                sendCommand('ileri');
                yonTutucu = setInterval(function() { sendCommand('ileri'); }, 200);
                sonCommandZamani = Date.now();
                updateMovementStatus(true);
            });
            document.getElementById('backBtn').addEventListener('mousedown', function() {
                sendCommand('geri');
                yonTutucu = setInterval(function() { sendCommand('geri'); }, 200);
                sonCommandZamani = Date.now();
                updateMovementStatus(true);
            });
            document.getElementById('leftBtn').addEventListener('mousedown', function() {
                sendCommand('sol');
                yonTutucu = setInterval(function() { sendCommand('sol'); }, 200);
                sonCommandZamani = Date.now();
                updateMovementStatus(true);
            });
            document.getElementById('rightBtn').addEventListener('mousedown', function() {
                sendCommand('sag');
                yonTutucu = setInterval(function() { sendCommand('sag'); }, 200);
                sonCommandZamani = Date.now();
                updateMovementStatus(true);
            });
            document.getElementById('stopBtn').addEventListener('click', function() { 
                sendCommand('dur');
                updateMovementStatus(false);
                if (yonTutucu) {
                    clearInterval(yonTutucu);
                    yonTutucu = null;
                }
            });
            
            // Yön tuşları bırakıldığında
            var dirButtons = document.querySelectorAll('.direction-btn');
            for (var i = 0; i < dirButtons.length; i++) {
                dirButtons[i].addEventListener('mouseup', function() {
                    if (yonTutucu) {
                        clearInterval(yonTutucu);
                        yonTutucu = null;
                        sendCommand('dur');
                        updateMovementStatus(false);
                    }
                });
                dirButtons[i].addEventListener('mouseleave', function() {
                    if (yonTutucu) {
                        clearInterval(yonTutucu);
                        yonTutucu = null;
                        sendCommand('dur');
                        updateMovementStatus(false);
                    }
                });
                
                // Dokunmatik ekran desteği
                dirButtons[i].addEventListener('touchstart', function(e) {
                    e.preventDefault();
                    var buttonId = this.id;
                    if (buttonId === 'forwardBtn') {
                        sendCommand('ileri');
                    } else if (buttonId === 'backBtn') {
                        sendCommand('geri');
                    } else if (buttonId === 'leftBtn') {
                        sendCommand('sol');
                    } else if (buttonId === 'rightBtn') {
                        sendCommand('sag');
                    } else if (buttonId === 'stopBtn') {
                        sendCommand('dur');
                    }
                    sonCommandZamani = Date.now();
                    updateMovementStatus(buttonId !== 'stopBtn');
                });
                
                dirButtons[i].addEventListener('touchend', function(e) {
                    e.preventDefault();
                    sendCommand('dur');
                    updateMovementStatus(false);
                });
            }
            
            // Müzik kontrol butonları
            document.getElementById('playBtn').addEventListener('click', function() {
                sendCommand('muzik_play');
                isPlaying = true;
                updateMusicUI();
            });
            
            document.getElementById('pauseBtn').addEventListener('click', function() {
                sendCommand('muzik_pause');
                isPlaying = false;
                updateMusicUI();
            });
            
            document.getElementById('prevBtn').addEventListener('click', function() {
                sendCommand('muzik_prev');
                currentTrack = currentTrack > 1 ? currentTrack - 1 : songs.length;
                updateMusicUI();
            });
            
            document.getElementById('nextBtn').addEventListener('click', function() {
                sendCommand('muzik_next');
                currentTrack = currentTrack < songs.length ? currentTrack + 1 : 1;
                updateMusicUI();
            });
            
            // Klavye kontrolü
            document.addEventListener('keydown', function(event) {
                if (event.repeat) return; // Tuş basılı tutulduğunda tekrar etmeyi engelle
                
                switch(event.key) {
                    case 'ArrowUp':
                        sendCommand('ileri');
                        sonCommandZamani = Date.now();
                        updateMovementStatus(true);
                        break;
                    case 'ArrowDown':
                        sendCommand('geri');
                        sonCommandZamani = Date.now();
                        updateMovementStatus(true);
                        break;
                    case 'ArrowLeft':
                        sendCommand('sol');
                        sonCommandZamani = Date.now();
                        updateMovementStatus(true);
                        break;
                    case 'ArrowRight':
                        sendCommand('sag');
                        sonCommandZamani = Date.now();
                        updateMovementStatus(true);
                        break;
                    case ' ':
                        sendCommand('dur');
                        updateMovementStatus(false);
                        break;
                }
            });
            
            document.addEventListener('keyup', function(event) {
                switch(event.key) {
                    case 'ArrowUp':
                    case 'ArrowDown':
                    case 'ArrowLeft':
                    case 'ArrowRight':
                        sendCommand('dur');
                        updateMovementStatus(false);
                        break;
                }
            });

            // Planlı temizlik ayarlama
            document.getElementById('planliTemizlikAyarlaBtn').addEventListener('click', function() {
                var saat = document.getElementById('scheduleHour').value;
                var dakika = document.getElementById('scheduleMinute').value;
                var sure = document.getElementById('scheduleDuration').value;
                
                planliSaat = parseInt(saat);
                planliDakika = parseInt(dakika);
                planliSure = parseInt(sure);
                
                // Komutu Arduino'ya gönder
                var planKomut = 'PLANLI_TEMIZLIK_AYARLA:' + 
                                (planliSaat < 10 ? '0' + planliSaat : planliSaat) + ':' + 
                                (planliDakika < 10 ? '0' + planliDakika : planliDakika) + ':' + 
                                planliSure;
                
                sendCommand(planKomut);
                
                planliTemizlikAktif = true;
                updateScheduleUI();
            });
            
            // Planlı temizlik iptal
            document.getElementById('planliTemizlikIptalBtn').addEventListener('click', function() {
                sendCommand('PLANLI_TEMIZLIK_IPTAL');
                planliTemizlikAktif = false;
                updateScheduleUI();
            });
            
            // İstatistikleri sıfırla
            document.getElementById('istatistikSifirlaBtn').addEventListener('click', function() {
                if (confirm("İstatistikleri sıfırlamak istediğinizden emin misiniz?")) {
                    sendCommand('ISTATISTIK_SIFIRLA');
                }
            });
        }
        
        // Müzik UI güncelleme
        function updateMusicUI() {
            // Şimdi çalan bilgisi
            document.getElementById('nowPlaying').textContent = songs[currentTrack - 1] || "Şarkı seçilmedi";
            document.getElementById('musicStatus').textContent = isPlaying ? "Çalıyor" : "Duraklatıldı";
            document.getElementById('musicStatus').style.color = isPlaying ? "var(--success-color)" : "var(--danger-color)";
            
            // Şarkı listesinde aktif şarkıyı işaretle
            var songItems = document.querySelectorAll('.song-item');
            for (var i = 0; i < songItems.length; i++) {
                var trackId = parseInt(songItems[i].dataset.id);
                if (trackId === currentTrack) {
                    songItems[i].classList.add('active');
                } else {
                    songItems[i].classList.remove('active');
                }
            }
        }

        // Robot hareket durumu güncelleme
        function updateMovementStatus(isMoving) {
            robotHareketEdiyor = isMoving;
            var statusElement = document.getElementById('robotMovementStatus');
            if (isMoving) {
                statusElement.textContent = "Robot Durum: Hareket Ediyor";
                statusElement.className = "robot-status robot-moving";
            } else {
                statusElement.textContent = "Robot Durum: Durgun";
                statusElement.className = "robot-status robot-stationary";
            }
        }

        // Plan durumu UI güncelleme
        function updateScheduleUI() {
            var scheduleStatus = document.getElementById('scheduleStatus');
            var planStatus = document.getElementById('planStatus');
            
            if (planliTemizlikAktif) {
                scheduleStatus.innerHTML = '<span class="schedule-active">Planlı temizlik: ' + 
                    (planliSaat < 10 ? '0' + planliSaat : planliSaat) + ':' + 
                    (planliDakika < 10 ? '0' + planliDakika : planliDakika) + 
                    ' (' + planliSure + ' dk)</span>';
                
                planStatus.textContent = 'Aktif';
                planStatus.className = 'status-value active';
            } else {
                scheduleStatus.innerHTML = '<span class="schedule-inactive">Planlı temizlik ayarlanmadı</span>';
                planStatus.textContent = 'Kapalı';
                planStatus.className = 'status-value inactive';
            }
        }
        
        // Durum güncelleme fonksiyonu
        function updateStatus() {
            fetch('/status')
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    // Mod durumları
                    document.getElementById('otomatikStatus').textContent = data.otomatik ? 'Açık' : 'Kapalı';
                    document.getElementById('otomatikStatus').className = 'status-value ' + (data.otomatik ? 'active' : 'inactive');
                    
                    document.getElementById('manuelStatus').textContent = data.manuel ? 'Açık' : 'Kapalı';
                    document.getElementById('manuelStatus').className = 'status-value ' + (data.manuel ? 'active' : 'inactive');
                    
                    document.getElementById('vakumStatus').textContent = data.vakum ? 'Açık' : 'Kapalı';
                    document.getElementById('vakumStatus').className = 'status-value ' + (data.vakum ? 'active' : 'inactive');
                    
                    // Hareket durumu
                    if (data.hareketEdiyor !== undefined) {
                        updateMovementStatus(data.hareketEdiyor);
                    }
                    
                    // Plan durumu
                    if (data.planliTemizlik !== undefined) {
                        planliTemizlikAktif = data.planliTemizlik;
                        updateScheduleUI();
                    }
                    
                    // Mesafe sensör değerleri
                    document.getElementById('onSagValue').textContent = data.onSag + ' cm';
                    document.getElementById('onSolValue').textContent = data.onSol + ' cm';
                    document.getElementById('sagYanValue').textContent = data.sagYan + ' cm';
                    document.getElementById('solYanValue').textContent = data.solYan + ' cm';
                    document.getElementById('ustValue').textContent = data.ust + ' cm'; // YENİ - Üst sensör değeri
                    
                    // Diğer sensör değerleri
                    document.getElementById('sicaklikValue').textContent = data.sicaklik + ' °C';
                    document.getElementById('nemValue').textContent = data.nem + ' %';
                    document.getElementById('gazValue').textContent = data.gaz;
                    document.getElementById('hareketValue').textContent = data.hareket ? 'Var' : 'Yok';
                    document.getElementById('sesValue').textContent = data.ses;
                    
                    // İstatistik verileri
                    if (data.toplamSureDk !== undefined) {
                        document.getElementById('toplamSure').textContent = data.toplamSureDk + ' dk';
                    }
                    if (data.toplamMesafe !== undefined) {
                        document.getElementById('toplamMesafe').textContent = data.toplamMesafe.toFixed(2) + ' m';
                    }
                    if (data.otomatikGecis !== undefined) {
                        document.getElementById('otomatikGecis').textContent = data.otomatikGecis + ' kere';
                    }
                    
                    // Müzik durumu
                    if (data.muzik !== undefined) {
                        isPlaying = data.muzik;
                    }
                    if (data.track !== undefined && data.track !== currentTrack) {
                        currentTrack = data.track;
                        updateMusicUI();
                    }
                })
                .catch(function(error) {
                    console.error('Durum güncellenirken hata:', error);
                });
        }
        
        // Düzenli durum güncellemesi
        setInterval(updateStatus, 500);
        
        // Komut gönderme fonksiyonu
        function sendCommand(cmd) {
            fetch('/control?cmd=' + cmd)
                .then(function(response) { return response.json(); })
                .then(function(data) {
                    console.log('Komut gönderildi:', cmd, 'Yanıt:', data);
                })
                .catch(function(error) {
                    console.error('Komut gönderilirken hata:', error);
                });
        }
    </script>
</body>
</html>
)rawliteral";

// MJPEG Streaming Handler için
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Sensor verilerini ayrıştırma - YENİ SÜRÜM
void parseSensorData(String data) {
  int startPos = data.indexOf('{');
  int endPos = data.indexOf('}');
  
  if (startPos >= 0 && endPos > startPos) {
    String jsonStr = data.substring(startPos, endPos + 1);
    
    // Debug amaçlı yazdır
    Serial.print("Ayrıştırılan JSON: ");
    Serial.println(jsonStr);
    
    // Arduino Mega artık tek bir ön sensör kullanıyor
    // "on" anahtar kelimesini kullanarak mesafeyi oku
    int onPos = jsonStr.indexOf("\"on\":");
    if (onPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', onPos);
      if (kommaPos > onPos) {
        onMesafe = jsonStr.substring(onPos + 5, kommaPos).toInt();
        // Eski arayüz uyumluluğu - tüm ön sensörler aynı değeri alıyor
        onSagMesafe = onMesafe;
        onSolMesafe = onMesafe;
        sagYanMesafe = onMesafe;
        solYanMesafe = onMesafe;
      }
    } 
    // Eski format kontrol - "onSag", "onSol" gibi alanları hala destekle
    else {
      int onSagPos = jsonStr.indexOf("\"onSag\":");
      if (onSagPos >= 0) {
        int kommaPos = jsonStr.indexOf(',', onSagPos);
        if (kommaPos > onSagPos) {
          onSagMesafe = jsonStr.substring(onSagPos + 8, kommaPos).toInt();
          onMesafe = onSagMesafe; // Ana değişkene aktar
        }
      }
    }
    
    // Üst mesafe sensörü
    int ustPos = jsonStr.indexOf("\"ust\":");
    if (ustPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', ustPos);
      if (kommaPos > ustPos) {
        ustMesafe = jsonStr.substring(ustPos + 6, kommaPos).toInt();
      }
    }
    
    // Diğer sensör değerleri
    int sicaklikPos = jsonStr.indexOf("\"sicaklik\":");
    if (sicaklikPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', sicaklikPos);
      if (kommaPos > sicaklikPos) {
        sicaklik = jsonStr.substring(sicaklikPos + 11, kommaPos).toFloat();
      }
    }
    
    int nemPos = jsonStr.indexOf("\"nem\":");
    if (nemPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', nemPos);
      if (kommaPos > nemPos) {
        nem = jsonStr.substring(nemPos + 6, kommaPos).toFloat();
      }
    }
    
    int gazPos = jsonStr.indexOf("\"gaz\":");
    if (gazPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', gazPos);
      if (kommaPos > gazPos) {
        gazSeviyesi = jsonStr.substring(gazPos + 6, kommaPos).toInt();
      }
    }
    
    int hareketPos = jsonStr.indexOf("\"hareket\":");
    if (hareketPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', hareketPos);
      if (kommaPos > hareketPos) {
        hareketAlgilandi = (jsonStr.substring(hareketPos + 10, kommaPos) == "1");
      }
    }
    
    int sesPos = jsonStr.indexOf("\"ses\":");
    if (sesPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', sesPos);
      if (kommaPos > sesPos) {
        sesSeviyesi = jsonStr.substring(sesPos + 6, kommaPos).toInt();
      }
    }
    
    // Durum değerleri
    int vakumPos = jsonStr.indexOf("\"vakum\":");
    if (vakumPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', vakumPos);
      String vakumStr = "";
      if (kommaPos > vakumPos) {
        vakumStr = jsonStr.substring(vakumPos + 8, kommaPos);
      } else {
        vakumStr = jsonStr.substring(vakumPos + 8);
      }
      vakumDurumu = (vakumStr == "true");
    }
    
    int otomatikPos = jsonStr.indexOf("\"otomatik\":");
    if (otomatikPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', otomatikPos);
      String otomatikStr = "";
      if (kommaPos > otomatikPos) {
        otomatikStr = jsonStr.substring(otomatikPos + 11, kommaPos);
      } else {
        otomatikStr = jsonStr.substring(otomatikPos + 11);
      }
      otomatikMod = (otomatikStr == "true");
    }
    
    int manuelPos = jsonStr.indexOf("\"manuel\":");
    if (manuelPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', manuelPos);
      String manuelStr = "";
      if (kommaPos > manuelPos) {
        manuelStr = jsonStr.substring(manuelPos + 9, kommaPos);
      } else {
        manuelStr = jsonStr.substring(manuelPos + 9);
      }
      manuelMod = (manuelStr == "true");
    }
    
    int muzikPos = jsonStr.indexOf("\"muzik\":");
    if (muzikPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', muzikPos);
      String muzikStr = "";
      if (kommaPos > muzikPos) {
        muzikStr = jsonStr.substring(muzikPos + 8, kommaPos);
      } else {
        muzikStr = jsonStr.substring(muzikPos + 8);
      }
      muzikCaliyor = (muzikStr == "true");
    }
    
    int trackPos = jsonStr.indexOf("\"track\":");
    if (trackPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', trackPos);
      if (kommaPos > trackPos) {
        currentTrack = jsonStr.substring(trackPos + 8, kommaPos).toInt();
      } else {
        currentTrack = jsonStr.substring(trackPos + 8).toInt();
      }
    }
    
    // Planlı temizlik durumu
    int planPos = jsonStr.indexOf("\"planliTemizlik\":");
    if (planPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', planPos);
      String planStr = "";
      if (kommaPos > planPos) {
        planStr = jsonStr.substring(planPos + 17, kommaPos);
      } else {
        planStr = jsonStr.substring(planPos + 17);
      }
      planliTemizlikAktif = (planStr == "true");
    }
    
    // İstatistik değerleri
    int toplamSurePos = jsonStr.indexOf("\"toplamSureDk\":");
    if (toplamSurePos >= 0) {
      int kommaPos = jsonStr.indexOf(',', toplamSurePos);
      if (kommaPos > toplamSurePos) {
        toplamCalismaSuresi = jsonStr.substring(toplamSurePos + 15, kommaPos).toInt();
      } else {
        toplamCalismaSuresi = jsonStr.substring(toplamSurePos + 15).toInt();
      }
    }
    
    int toplamMesafePos = jsonStr.indexOf("\"toplamMesafe\":");
    if (toplamMesafePos >= 0) {
      int kommaPos = jsonStr.indexOf(',', toplamMesafePos);
      if (kommaPos > toplamMesafePos) {
        toplamGidilenMesafe = jsonStr.substring(toplamMesafePos + 15, kommaPos).toFloat();
      } else {
        toplamGidilenMesafe = jsonStr.substring(toplamMesafePos + 15).toFloat();
      }
    }
    
    int otomatikGecisPos = jsonStr.indexOf("\"otomatikGecis\":");
    if (otomatikGecisPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', otomatikGecisPos);
      if (kommaPos > otomatikGecisPos) {
        otomatikModGecis = jsonStr.substring(otomatikGecisPos + 16, kommaPos).toInt();
      } else {
        otomatikModGecis = jsonStr.substring(otomatikGecisPos + 16).toInt();
      }
    }
    
    // Robot hareket durumu kontrolü (yeni)
    robotHareketEdiyor = !(otomatikMod && millis() - sonHareketZamani > 3000);
  }
}

// Durum verilerini JSON formatında döndüren fonksiyon
String getStatusJson() {
  String json = "{";
  
  // Mod durumları
  json += "\"otomatik\":" + String(otomatikMod ? "true" : "false") + ",";
  json += "\"manuel\":" + String(manuelMod ? "true" : "false") + ",";
  json += "\"vakum\":" + String(vakumDurumu ? "true" : "false") + ",";
  json += "\"planliTemizlik\":" + String(planliTemizlikAktif ? "true" : "false") + ",";
  json += "\"hareketEdiyor\":" + String(robotHareketEdiyor ? "true" : "false") + ",";
  
  // Mesafe sensörleri - Eski arayüz uyumluluğu için
  json += "\"onSag\":" + String(onSagMesafe) + ",";
  json += "\"onSol\":" + String(onSolMesafe) + ",";
  json += "\"sagYan\":" + String(sagYanMesafe) + ",";
  json += "\"solYan\":" + String(solYanMesafe) + ",";
  json += "\"ust\":" + String(ustMesafe) + ",";
  
  // Diğer sensörler
  json += "\"sicaklik\":" + String(sicaklik) + ",";
  json += "\"nem\":" + String(nem) + ",";
  json += "\"gaz\":" + String(gazSeviyesi) + ",";
  json += "\"hareket\":" + String(hareketAlgilandi ? "true" : "false") + ",";
  json += "\"ses\":" + String(sesSeviyesi) + ",";
  json += "\"hiz\":" + String(motorHizi) + ",";
  
  // Müzik durumu
  json += "\"muzik\":" + String(muzikCaliyor ? "true" : "false") + ",";
  json += "\"track\":" + String(currentTrack) + ",";
  
  // İstatistikler
  json += "\"toplamSureDk\":" + String(toplamCalismaSuresi) + ",";
  json += "\"toplamMesafe\":" + String(toplamGidilenMesafe) + ",";
  json += "\"otomatikGecis\":" + String(otomatikModGecis);
  
  json += "}";
  
  return json;
}

// Kamera yayını için gerekli handler
void streamHandler(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginChunkedResponse(STREAM_CONTENT_TYPE, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    static size_t last_frame_size = 0;
    static uint8_t *last_frame_buffer = nullptr;
    static bool is_streaming = false;
    
    if (!is_streaming) {
      if (last_frame_buffer) free(last_frame_buffer);
      last_frame_buffer = nullptr;
      last_frame_size = 0;
      
      // MJPEG stream başlığını yaz
      const char *boundary = STREAM_BOUNDARY;
      size_t boundary_len = strlen(boundary);
      
      if (boundary_len > maxLen) return 0;
      memcpy(buffer, boundary, boundary_len);
      is_streaming = true;
      return boundary_len;
    }
    
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      is_streaming = false;
      return 0;
    }
    
    // Önce frame header bilgisini yaz
    static char part_buf[128];
    size_t part_len = snprintf(part_buf, sizeof(part_buf), STREAM_PART, fb->len);
    
    if (part_len + fb->len > maxLen) {
      esp_camera_fb_return(fb);
      is_streaming = false;
      return 0;
    }
    
    memcpy(buffer, part_buf, part_len);
    memcpy(buffer + part_len, fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    
    // Sonraki frame için stream başlığını sıfırla
    is_streaming = false;
    
    return part_len + fb->len;
  });
  
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void setup() {
  // Seri port başlatma
  Serial.begin(115200);
  delay(1000);
  Serial.println("Başlatılıyor...");
  
  // Kamera yapılandırması
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // PSRAM varsa yüksek kalite, yoksa düşük kalite
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 16;
    config.fb_count = 1;
  }
  
  // Kamerayı başlat
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı, hata: 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  
  // Kamera parametrelerini ayarla
  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_quality(s, 15);
    s->set_brightness(s, 1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 1);
    s->set_hmirror(s, 0); // Yatay ayna
    s->set_vflip(s, 0);   // Dikey çevirme
  }
  
  // WiFi bağlantısı
  WiFi.begin(ssid, password);
  Serial.print("WiFi bağlanıyor");
  
  // WiFi bağlantı durumunu göster
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("WiFi Bağlandı - IP Adresi: ");
  Serial.println(WiFi.localIP());
  
  // NTP sunucusuna bağlan ve zamanı senkronize et
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Web sunucu route tanımlamaları
  
  // Ana sayfa
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  // Durum bilgisi API'si
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getStatusJson());
  });
  
  // Kontrol API'si
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request) {
    String cmd;
    if (request->hasParam("cmd")) {
      cmd = request->getParam("cmd")->value();
      Serial.print("Gelen komut: ");
      Serial.println(cmd);  // Debug için komut yazdırma
      
      if (cmd == "otomatik_ac") {
        otomatikMod = true;
        manuelMod = false;
        Serial.println("OTOMATIK_AC");
      } 
      else if (cmd == "otomatik_kapat") {
        otomatikMod = false;
        Serial.println("OTOMATIK_KAPAT");
      } 
      else if (cmd == "manuel_ac") {
        manuelMod = true;
        otomatikMod = false;
        Serial.println("MANUEL_AC");
      } 
      else if (cmd == "manuel_kapat") {
        manuelMod = false;
        Serial.println("MANUEL_KAPAT");
      } 
      else if (cmd == "vakum_ac") {
        vakumDurumu = true;
        Serial.println("VAKUM_AC");
      } 
      else if (cmd == "vakum_kapat") {
        vakumDurumu = false;
        Serial.println("VAKUM_KAPAT");
      } 
      else if (cmd == "ileri") {
        Serial.println("YON:F");
        sonHareketZamani = millis();
        robotHareketEdiyor = true;
      } 
      else if (cmd == "geri") {
        Serial.println("YON:B");
        sonHareketZamani = millis();
        robotHareketEdiyor = true;
      } 
      else if (cmd == "sol") {
        Serial.println("YON:L");
        sonHareketZamani = millis();
        robotHareketEdiyor = true;
      } 
      else if (cmd == "sag") {
        Serial.println("YON:R");
        sonHareketZamani = millis();
        robotHareketEdiyor = true;
      } 
      else if (cmd == "dur") {
        Serial.println("YON:S");
        robotHareketEdiyor = false;
      }
      else if (cmd.startsWith("hiz:")) {
        motorHizi = cmd.substring(4).toInt();
        Serial.println("HIZ:" + String(motorHizi));
      }
      // Müzik komutları
      else if (cmd == "muzik_play") {
        Serial.println("MUZIK_PLAY");
        muzikCaliyor = true;
      }
      else if (cmd == "muzik_pause") {
        Serial.println("MUZIK_PAUSE");
        muzikCaliyor = false;
      }
      else if (cmd == "muzik_next") {
        Serial.println("MUZIK_NEXT");
        currentTrack = (currentTrack % SONG_COUNT) + 1;
      }
      else if (cmd == "muzik_prev") {
        Serial.println("MUZIK_PREV");
        currentTrack = (currentTrack > 1) ? currentTrack - 1 : SONG_COUNT;
      }
      else if (cmd.startsWith("muzik_volume:")) {
        String volume = cmd.substring(13);
        Serial.println("MUZIK_VOLUME:" + volume);
      }
      else if (cmd.startsWith("muzik_track:")) {
        currentTrack = cmd.substring(12).toInt();
        Serial.println("MUZIK_TRACK:" + String(currentTrack));
      }
      // Planlı temizlik komutları
      else if (cmd.startsWith("PLANLI_TEMIZLIK_AYARLA:")) {
        Serial.println(cmd); // Komutu Arduino Mega'ya gönder
        planliTemizlikAktif = true;
      }
      else if (cmd == "PLANLI_TEMIZLIK_IPTAL") {
        Serial.println(cmd); // Komutu Arduino Mega'ya gönder
        planliTemizlikAktif = false;
      }
      // İstatistikleri sıfırlama
      else if (cmd == "ISTATISTIK_SIFIRLA") {
        Serial.println(cmd); // Komutu Arduino Mega'ya gönder
      }
    }
    
    request->send(200, "application/json", "{\"success\":true,\"command\":\""+cmd+"\"}");
  });
  
  // Kamera yayın API'si
  server.on("/stream", HTTP_GET, streamHandler);
  
  // Sunucuyu başlat
  server.begin();
  
  Serial.println("HTTP Sunucusu başlatıldı");
  
  // Debug mesajı
  Serial.println("ESP32 CAM hazır, sensör verilerini dinliyor...");
}

void loop() {
  // Arduino Mega'dan gelen verileri oku
  if (Serial.available()) {
    String veri = Serial.readStringUntil('\n');
    veri.trim();
    
    // Sensor verilerini işle
    if (veri.startsWith("SENSOR_DATA:")) {
      parseSensorData(veri);
      
      // Hareket durumu kontrolü - Arduino'dan gelen veriden
      if (otomatikMod) {
        // Eğer otomatik moddaysa ve hareket durumu değişirse, hareket zaman damgasını güncelle
        if (robotHareketEdiyor) {
          sonHareketZamani = millis();
        }
      }
    }
    // Mod değişikliklerini algıla
    else if (veri == "Otomatik mod acildi") {
      otomatikMod = true;
      manuelMod = false;
      sonHareketZamani = millis(); // Hareket zamanlayıcısını sıfırla
      robotHareketEdiyor = true;   // Başlangıçta hareket ediyor varsay
    }
    else if (veri == "Otomatik mod kapatildi") {
      otomatikMod = false;
      robotHareketEdiyor = false;
    }
    else if (veri == "Manuel mod acildi") {
      manuelMod = true;
      otomatikMod = false;
    }
    else if (veri == "Manuel mod kapatildi") {
      manuelMod = false;
    }
    // Vakum durumunu algıla
    else if (veri == "VAKUM:ACIK") {
      vakumDurumu = true;
    }
    else if (veri == "VAKUM:KAPALI") {
      vakumDurumu = false;
    }
    // Müzik durumunu algıla
    else if (veri == "MUZIK:CALIYOR") {
      muzikCaliyor = true;
    }
    else if (veri == "MUZIK:DURAKLATILDI") {
      muzikCaliyor = false;
    }
    else if (veri.startsWith("MUZIK:TRACK:")) {
      currentTrack = veri.substring(12).toInt();
    }
    // Planlı temizlik durumunu algıla
    else if (veri == "PLANLI_TEMIZLIK_AYARLANDI") {
      planliTemizlikAktif = true;
    }
    else if (veri == "PLANLI_TEMIZLIK_IPTAL_EDILDI") {
      planliTemizlikAktif = false;
    }
    else if (veri == "PLANLI_TEMIZLIK_BASLADI") {
      otomatikMod = true;
      manuelMod = false;
      sonHareketZamani = millis(); // Hareket zamanlayıcısını sıfırla
      robotHareketEdiyor = true;
    }
    else if (veri == "PLANLI_TEMIZLIK_TAMAMLANDI") {
      otomatikMod = false;
      robotHareketEdiyor = false;
    }
    // Tarama modu durumunu algıla
    else if (veri.startsWith("Tarama modu başlatılıyor")) {
      Serial.println("Web: Tarama modu algılandı");
    }
    
    // Debug için alınan veriyi yazdır
    Serial.print("Mega'dan alınan: ");
    Serial.println(veri);
  }
  
  // 3 saniye hareketsizlik kontrolü - otomatik modda
  if (otomatikMod && robotHareketEdiyor) {
    if (millis() - sonHareketZamani > 3000) { // 3 saniye
      robotHareketEdiyor = false;
      Serial.println("3 saniye hareketsiz kaldı, robot duruyor");
    }
  }
  
  // WiFi bağlantı kontrolü
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi bağlantısı koptu, yeniden bağlanılıyor...");
    WiFi.reconnect();
    
    // Yeniden bağlanma için bekle
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi yeniden bağlandı - IP Adresi: " + WiFi.localIP().toString());
    } else {
      Serial.println("WiFi bağlantısı kurulamadı, cihaz yeniden başlatılıyor...");
      ESP.restart();
    }
  }
  
  // NTP ile zamanı düzenli olarak güncelle (her 24 saatte bir)
  static unsigned long lastTimeSync = 0;
  if (millis() - lastTimeSync > 86400000) { // 24 saat
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    lastTimeSync = millis();
  }
  
  // Periyodik olarak zamanı Arduino'ya gönder
  if (millis() - sonZamanGondermeZamani > ZAMAN_GONDER_ARALIK) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[30];
      sprintf(timeStr, "ZAMAN_AYARLA:%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      Serial.println(timeStr);
      
      char dateStr[30];
      sprintf(dateStr, "TARIH_AYARLA:%02d:%02d:%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      Serial.println(dateStr);
      
      sonZamanGondermeZamani = millis();
    }
  }
  
  // Debug mesajları
  if (millis() - sonDebugGondermeZamani > DEBUG_GONDER_ARALIK) {
    Serial.println("\n--- DEBUG BILGISI ---");
    Serial.print("Ön Mesafe: ");
    Serial.println(onMesafe);
    Serial.print("Üst Mesafe: ");
    Serial.println(ustMesafe);
    Serial.print("Otonom Mod: ");
    Serial.println(otomatikMod ? "AÇIK" : "KAPALI");
    Serial.print("Tarama Modu: ");
    Serial.println("KAPALI"); // ESP32'de tarama modu takibi yok
    Serial.println("--- DEBUG SONU ---\n");
    
    sonDebugGondermeZamani = millis();
  }
}