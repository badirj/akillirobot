#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include "time.h"
const char* ssid = "Badirinphone";
const char* password = "Badir8186";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800; 
const int daylightOffset_sec = 0;
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

bool otomatikMod = false;
bool manuelMod = false;
bool vakumDurumu = false;
bool muzikCaliyor = false;
int currentTrack = 1;
int onMesafe = 0;
int ustMesafe = 0;
int sagOnMesafe = 0;
int solOnMesafe = 0;
int sagYanMesafe = 0;
int solYanMesafe = 0;
int otonomMod = 1; 
float sicaklik = 0;
float nem = 0;
int gazSeviyesi = 0;
bool hareketAlgilandi = false;
int sesSeviyesi = 0;
int motorHizi = 200;
int toplamCalismaSuresi = 0; 
float toplamGidilenMesafe = 0.0; 
int otomatikModGecis = 0;
bool planliTemizlikAktif = false;
int planliSaat = 0;
int planliDakika = 0;
int planliSure = 30; 

bool robotHareketEdiyor = false;
unsigned long sonHareketZamani = 0;
unsigned long sonZamanGondermeZamani = 0;
const unsigned long ZAMAN_GONDER_ARALIK = 43200000; 
unsigned long sonDebugGondermeZamani = 0;
const unsigned long DEBUG_GONDER_ARALIK = 10000; 
const int SONG_COUNT = 5;
String songNames[SONG_COUNT] = {
  "1. Sarki Adi", "2. Sarki Adi", "3. Sarki Adi", "4. Sarki Adi", "5. Sarki Adi"
};
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Akıllı Süpürge Kontrol Paneli</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
    <style>
        :root {
            --primary-color: #4361EE;
            --secondary-color: #3A0CA3;
            --success-color: #4CAF50;
            --danger-color: #F72585;
            --warning-color: #F9C74F;
            --info-color: #4CC9F0;
            --dark-color: #242423;
            --light-color: #f8f9fa;
            --gray-color: #e9ecef;
            --text-color: #333;
            --card-shadow: 0 4px 15px rgba(0,0,0,0.08);
            --transition: all 0.3s ease;
        }
        
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: #f4f6fa;
            color: var(--text-color);
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 1rem;
        }
        
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: white;
            padding: 1.5rem;
            text-align: center;
            border-radius: 12px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.15);
            margin-bottom: 1.5rem;
            position: relative;
            overflow: hidden;
        }
        
        header::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, rgba(255,255,255,0) 70%);
            transform: rotate(30deg);
        }
        
        header h1 {
            margin: 0;
            font-size: 1.8rem;
            position: relative;
            text-shadow: 0 2px 4px rgba(0,0,0,0.2);
        }
        
        .card {
            background-color: white;
            border-radius: 12px;
            padding: 1.5rem;
            margin-bottom: 1.5rem;
            box-shadow: var(--card-shadow);
            transition: var(--transition);
            border: 1px solid rgba(0,0,0,0.03);
        }
        
        .card:hover {
            box-shadow: 0 8px 25px rgba(0,0,0,0.1);
            transform: translateY(-5px);
        }
        
        .card h3 {
            color: var(--primary-color);
            margin-bottom: 1.2rem;
            padding-bottom: 0.8rem;
            border-bottom: 2px solid #eaeaea;
            position: relative;
            font-weight: 600;
        }
        
        .card h3::after {
            content: '';
            position: absolute;
            left: 0;
            bottom: -2px;
            width: 60px;
            height: 2px;
            background-color: var(--primary-color);
        }
        
        .status-panel {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
            gap: 1rem;
        }
        
        .status-item {
            padding: 1rem;
            border-radius: 10px;
            background-color: var(--gray-color);
            display: flex;
            flex-direction: column;
            align-items: center;
            text-align: center;
            transition: var(--transition);
        }
        
        .status-item:hover {
            transform: translateY(-3px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.05);
        }
        
        .status-label {
            font-weight: 600;
            margin-bottom: 0.5rem;
            font-size: 0.9rem;
            color: var(--dark-color);
        }
        
        .status-value {
            padding: 0.4rem 1rem;
            border-radius: 30px;
            color: white;
            font-weight: bold;
            min-width: 100px;
            text-align: center;
            box-shadow: 0 3px 8px rgba(0,0,0,0.1);
        }
        
        .active {
            background-color: var(--success-color);
        }
        
        .inactive {
            background-color: var(--danger-color);
        }
        
        .main-content {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(350px, 1fr));
            gap: 1.5rem;
        }
        
        .btn {
            padding: 0.6rem 1.2rem;
            border: none;
            border-radius: 50px;
            cursor: pointer;
            font-weight: 600;
            transition: var(--transition);
            margin: 0.3rem;
            min-width: 120px;
            box-shadow: 0 3px 8px rgba(0,0,0,0.1);
            letter-spacing: 0.5px;
        }
        
        .btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }
        
        .btn:active {
            transform: translateY(-1px);
            box-shadow: 0 2px 5px rgba(0,0,0,0.15);
        }
        
        .btn-primary {
            background-color: var(--primary-color);
            color: white;
        }
        
        .btn-secondary {
            background-color: var(--secondary-color);
            color: white;
        }
        
        .btn-danger {
            background-color: var(--danger-color);
            color: white;
        }
        
        .btn-success {
            background-color: var(--success-color);
            color: white;
        }
        
        .btn-warning {
            background-color: var(--warning-color);
            color: var(--dark-color);
        }
        
        .btn-info {
            background-color: var(--info-color);
            color: white;
        }
        
        .btn-dark {
            background-color: var(--dark-color);
            color: white;
        }
        
        .direction-controls {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            grid-template-rows: repeat(3, 1fr);
            gap: 0.7rem;
            max-width: 300px;
            margin: 1.5rem auto;
        }
        
        .direction-btn {
            padding: 1.2rem;
            font-size: 1.3rem;
            border-radius: 12px;
            background-color: var(--primary-color);
            color: white;
            cursor: pointer;
            border: none;
            transition: var(--transition);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .direction-btn:hover {
            background-color: #3051d3;
            transform: scale(1.05) translateY(-3px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.2);
        }
        
        .direction-btn:active {
            transform: scale(0.98) translateY(0);
            box-shadow: 0 2px 5px rgba(0,0,0,0.15);
        }
        
        #stopBtn {
            grid-column: 2;
            grid-row: 2;
            background-color: var(--danger-color);
        }
        
        #stopBtn:hover {
            background-color: #e91a75;
        }
        
        #forwardBtn {
            grid-column: 2;
            grid-row: 1;
        }
        
        #backBtn {
            grid-column: 2;
            grid-row: 3;
        }
        
        #leftBtn {
            grid-column: 1;
            grid-row: 2;
        }
        
        #rightBtn {
            grid-column: 3;
            grid-row: 2;
        }
        
        .camera-view {
            text-align: center;
            margin-bottom: 1rem;
            overflow: hidden;
            border-radius: 12px;
        }
        
        .camera-stream {
            width: 100%;
            max-height: 480px;
            border-radius: 12px;
            object-fit: contain;
            border: none;
            box-shadow: var(--card-shadow);
            transition: var(--transition);
        }
        
        .camera-stream:hover {
            transform: scale(1.02);
            box-shadow: 0 8px 20px rgba(0,0,0,0.15);
        }
        
        .sensor-panel {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
            gap: 1rem;
        }
        
        .sensor-box {
            padding: 1.2rem;
            border-radius: 12px;
            background: white;
            text-align: center;
            border: 1px solid rgba(0,0,0,0.03);
            box-shadow: var(--card-shadow);
            transition: var(--transition);
        }
        
        .sensor-box:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 20px rgba(0,0,0,0.1);
        }
        
        .sensor-box h4 {
            color: var(--primary-color);
            margin-bottom: 0.8rem;
            font-weight: 600;
            font-size: 1rem;
        }
        
        .sensor-value {
            font-size: 1.6rem;
            font-weight: bold;
            color: var(--primary-color);
            margin-top: 0.8rem;
            text-shadow: 0 1px 2px rgba(0,0,0,0.05);
        }
        
        .slider-container {
            margin: 1.5rem 0;
            padding: 1.5rem;
            background: white;
            border-radius: 12px;
            box-shadow: var(--card-shadow);
        }
        
        .slider {
            -webkit-appearance: none;
            width: 100%;
            height: 10px;
            border-radius: 20px;
            background: #e0e0e0;
            outline: none;
            margin: 1.5rem 0;
            overflow: hidden;
        }
        
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: var(--primary-color);
            cursor: pointer;
            box-shadow: 0 0 0 4px var(--primary-color), 0 0 0 5px rgba(67, 97, 238, 0.3);
            transition: var(--transition);
        }
        
        .slider::-webkit-slider-runnable-track {
            height: 10px;
            border-radius: 20px;
            background: linear-gradient(90deg, var(--primary-color) 60%, #e0e0e0 60%);
        }
        
        .slider::-moz-range-thumb {
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: var(--primary-color);
            cursor: pointer;
            box-shadow: 0 0 0 4px var(--primary-color), 0 0 0 5px rgba(67, 97, 238, 0.3);
            transition: var(--transition);
        }
        
        .slider::-moz-range-track {
            height: 10px;
            border-radius: 20px;
        }
        
        .slider-value {
            text-align: center;
            font-size: 1.3rem;
            font-weight: bold;
            color: var(--primary-color);
            margin-top: 0.5rem;
        }
        
        .music-player {
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 1.5rem;
            background: linear-gradient(135deg, #f5f7ff, #eef2ff);
            border-radius: 12px;
            box-shadow: var(--card-shadow);
        }
        
        .music-info {
            width: 100%;
            text-align: center;
            padding: 1.2rem;
            margin-bottom: 1.2rem;
            background: white;
            border-radius: 12px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.03);
        }
        
        .music-info h4 {
            color: var(--primary-color);
            margin-bottom: 0.8rem;
            font-weight: 600;
        }
        
        .now-playing {
            font-size: 1.25rem;
            font-weight: bold;
            margin-bottom: 0.8rem;
            color: var(--text-color);
        }
        
        .music-status {
            color: var(--success-color);
            font-weight: bold;
            padding: 0.3rem 1rem;
            border-radius: 30px;
            display: inline-block;
            background-color: rgba(76, 175, 80, 0.1);
        }
        
        .music-controls {
            display: flex;
            justify-content: center;
            gap: 0.8rem;
            margin-bottom: 1.5rem;
            width: 100%;
        }
        
        .music-controls .btn {
            font-size: 1.2rem;
            padding: 0.8rem;
            border-radius: 50%;
            width: 50px;
            height: 50px;
            display: flex;
            justify-content: center;
            align-items: center;
            min-width: auto;
            box-shadow: 0 4px 10px rgba(0,0,0,0.1);
        }
        
        .song-list {
            width: 100%;
            margin-top: 1.2rem;
        }
        
        .song-item {
            padding: 1rem;
            background: white;
            border-radius: 8px;
            margin-bottom: 0.8rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: 0 2px 5px rgba(0,0,0,0.05);
            border-left: 3px solid transparent;
        }
        
        .song-item:hover {
            background: #f5f7ff;
            border-left-color: var(--primary-color);
            transform: translateX(5px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.08);
        }
        
        .song-item.active {
            background: #eef2ff;
            border-left-color: var(--success-color);
            font-weight: bold;
        }
        
        .play-song-btn {
            padding: 0.4rem 0.8rem;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: 30px;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            font-weight: 600;
        }
        
        .play-song-btn:hover {
            background: var(--secondary-color);
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }
        
        /* Planlı Temizlik Stilleri */
        .schedule-form {
            display: flex;
            flex-direction: column;
            gap: 1.2rem;
            margin-top: 1.2rem;
        }
        
        .form-row {
            display: flex;
            justify-content: space-between;
            gap: 1.2rem;
            align-items: center;
        }
        
        .form-group {
            display: flex;
            flex-direction: column;
            gap: 0.6rem;
            flex: 1;
        }
        
        .form-group label {
            font-weight: 600;
            color: var(--primary-color);
            font-size: 0.9rem;
        }
        
        .form-group input, .form-group select {
            padding: 0.8rem;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            background-color: #f9f9f9;
            transition: var(--transition);
            color: var(--text-color);
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
        }
        
        .form-group select {
            background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='16' height='16' fill='%234361EE' viewBox='0 0 16 16'%3E%3Cpath d='M7.247 11.14 2.451 5.658C1.885 5.013 2.345 4 3.204 4h9.592a1 1 0 0 1 .753 1.659l-4.796 5.48a1 1 0 0 1-1.506 0z'/%3E%3C/svg%3E");
            background-repeat: no-repeat;
            background-position: calc(100% - 15px) center;
            padding-right: 35px;
        }
        
        .form-group input:focus, .form-group select:focus {
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px rgba(67, 97, 238, 0.1);
            outline: none;
        }
        
        .schedule-status {
            margin-top: 1.5rem;
            padding: 1.2rem;
            border-radius: 8px;
            background: white;
            text-align: center;
            box-shadow: var(--card-shadow);
        }
        
        .schedule-active {
            color: var(--success-color);
            font-weight: bold;
            padding: 0.5rem 1rem;
            background-color: rgba(76, 175, 80, 0.1);
            border-radius: 30px;
            display: inline-block;
        }
        
        .schedule-inactive {
            color: var(--danger-color);
            font-weight: bold;
            padding: 0.5rem 1rem;
            background-color: rgba(247, 37, 133, 0.1);
            border-radius: 30px;
            display: inline-block;
        }
        
        /* İstatistik Paneli Stilleri */
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
            gap: 1.2rem;
            margin-top: 1.2rem;
        }
        
        .stats-item {
            padding: 1.5rem;
            border-radius: 12px;
            background: white;
            text-align: center;
            box-shadow: var(--card-shadow);
            transition: var(--transition);
            border: 1px solid rgba(0,0,0,0.03);
        }
        
        .stats-item:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 20px rgba(0,0,0,0.1);
        }
        
        .stats-value {
            font-size: 2.2rem;
            font-weight: bold;
            color: var(--primary-color);
            margin: 0.8rem 0;
        }
        
        .stats-label {
            color: var(--dark-color);
            font-weight: 600;
            font-size: 1rem;
        }
        
        /* Robot durum indikatörü */
        .robot-status {
            padding: 0.7rem 1.5rem;
            margin: 0.8rem 0;
            border-radius: 30px;
            text-align: center;
            font-weight: bold;
            transition: var(--transition);
            box-shadow: 0 3px 10px rgba(0,0,0,0.05);
            display: inline-block;
        }
        
        .robot-moving {
            background-color: rgba(76, 175, 80, 0.15);
            color: var(--success-color);
        }
        
        .robot-stationary {
            background-color: rgba(249, 199, 79, 0.15);
            color: var(--warning-color);
        }
        
        /* YENİ - Otonom Mod Seçim Butonları */
        .otonom-mod-panel {
            margin-top: 1.5rem;
            padding: 1.2rem;
            background-color: #f0f7ff;
            border-radius: 12px;
            border: 1px solid #d1e3ff;
            box-shadow: var(--card-shadow);
        }
        
        .otonom-mod-title {
            font-weight: 600;
            margin-bottom: 0.8rem;
            color: var(--primary-color);
        }
        
        .otonom-mod-buttons {
            display: flex;
            gap: 1rem;
            flex-wrap: wrap;
        }
        
        .otonom-mod-btn {
            flex: 1;
            min-width: 160px;
            text-align: center;
            border-radius: 8px;
            padding: 0.8rem 1rem;
            transition: var(--transition);
            box-shadow: 0 3px 10px rgba(0,0,0,0.05);
        }
        
        .active-mod {
            background-color: var(--success-color);
            color: white;
            position: relative;
            box-shadow: 0 5px 15px rgba(76, 175, 80, 0.3);
        }
        
        .active-mod::before {
            content: '✓';
            position: absolute;
            left: 10px;
            top: 50%;
            transform: translateY(-50%);
            font-weight: bold;
        }
        
        @media (max-width: 768px) {
            .main-content {
                grid-template-columns: 1fr;
            }
            
            .direction-controls {
                max-width: 100%;
            }
            
            .camera-stream {
                max-height: 300px;
            }
            
            .sensor-panel {
                grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));
            }
            
            .music-controls .btn {
                width: 45px;
                height: 45px;
                font-size: 1rem;
            }
            
            .stats-grid {
                grid-template-columns: 1fr;
            }
            
            .form-row {
                flex-direction: column;
            }
            
            .status-panel {
                grid-template-columns: repeat(auto-fill, minmax(150px, 1fr));
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1><i class="fas fa-robot"></i> Akıllı Süpürge Robot Kontrol Paneli</h1>
        </header>
        
        <div class="card">
            <h3><i class="fas fa-info-circle"></i> Durum Paneli</h3>
            <div class="status-panel">
                <div class="status-item">
                    <span class="status-label">Otomatik Mod</span>
                    <span id="otomatikStatus" class="status-value inactive">Kapalı</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Manuel Mod</span>
                    <span id="manuelStatus" class="status-value inactive">Kapalı</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Vakum Motoru</span>
                    <span id="vakumStatus" class="status-value inactive">Kapalı</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Planlı Temizlik</span>
                    <span id="planStatus" class="status-value inactive">Kapalı</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Otonom Mod</span>
                    <span id="otonomModStatus" class="status-value active">Mod 1</span>
                </div>
            </div>
            <div style="text-align: center; margin-top: 1rem;">
                <div class="robot-status" id="robotMovementStatus">
                    <i class="fas fa-circle-notch fa-spin" style="margin-right: 8px;"></i> Robot Durum: Hazır
                </div>
            </div>
        </div>
        
        <div class="main-content">
            <div class="card">
                <h3><i class="fas fa-camera"></i> Kamera Görüntüsü</h3>                
                <div class="camera-view">
                  <img src="/stream" alt="Kamera Yayını" class="camera-stream" id="cameraStream">
                </div>
            </div>
            
            <div class="card">
                <h3><i class="fas fa-cogs"></i> Kontrol Paneli</h3>
                <div class="control-section">
                    <h4 style="margin-bottom: 1rem; color: var(--primary-color);"><i class="fas fa-power-off"></i> Çalışma Modu</h4>
                    <button id="otomatikAcBtn" class="btn btn-primary"><i class="fas fa-robot"></i> Otomatik Mod Aç</button>
                    <button id="otomatikKapatBtn" class="btn btn-danger"><i class="fas fa-power-off"></i> Otomatik Kapat</button>
                    <button id="manuelAcBtn" class="btn btn-primary"><i class="fas fa-hand-pointer"></i> Manuel Mod Aç</button>
                    <button id="manuelKapatBtn" class="btn btn-danger"><i class="fas fa-power-off"></i> Manuel Kapat</button>
                </div>
                
                <!-- Otonom Mod Seçim Butonları -->
                <div class="otonom-mod-panel">
                    <div class="otonom-mod-title"><i class="fas fa-sliders-h"></i> Otonom Mod Seçimi:</div>
                    <div class="otonom-mod-buttons">
                        <button id="otonomMod1Btn" class="btn btn-primary otonom-mod-btn active-mod">Mod 1: Çoklu Sensör</button>
                        <button id="otonomMod2Btn" class="btn btn-primary otonom-mod-btn">Mod 2: Üst-Ön Sensör</button>
                    </div>
                </div>
                
                <div class="control-section">
                    <h4 style="margin: 1.5rem 0 1rem; color: var(--primary-color);"><i class="fas fa-vacuum-robot"></i> Vakum Kontrolü</h4>
                    <button id="vakumAcBtn" class="btn btn-success"><i class="fas fa-play"></i> Vakum Aç</button>
                    <button id="vakumKapatBtn" class="btn btn-danger"><i class="fas fa-stop"></i> Vakum Kapat</button>
                </div>
                
                <div class="slider-container">
                    <h4 style="color: var(--primary-color);"><i class="fas fa-tachometer-alt"></i> Motor Hızı</h4>
                    <input type="range" min="50" max="255" value="200" class="slider" id="hizSlider">
                    <div class="slider-value">
                        <span id="hizDeger">200</span>
                    </div>
                </div>
                
                <div class="control-section">
                    <h4 style="margin: 1.5rem 0 1rem; color: var(--primary-color);"><i class="fas fa-compass"></i> Manuel Yön Kontrolü</h4>
                    <div class="direction-controls">
                        <button id="forwardBtn" class="direction-btn"><i class="fas fa-chevron-up"></i></button>
                        <button id="leftBtn" class="direction-btn"><i class="fas fa-chevron-left"></i></button>
                        <button id="stopBtn" class="direction-btn"><i class="fas fa-stop"></i></button>
                        <button id="rightBtn" class="direction-btn"><i class="fas fa-chevron-right"></i></button>
                        <button id="backBtn" class="direction-btn"><i class="fas fa-chevron-down"></i></button>
                    </div>
                </div>
            </div>
        </div>
        
        <!-- Planlı Temizlik Paneli -->
        <div class="card">
            <h3><i class="fas fa-calendar-alt"></i> Planlı Temizlik</h3>
            <div class="schedule-form">
                <div class="form-row">
                    <div class="form-group">
                        <label for="scheduleHour"><i class="fas fa-clock"></i> Saat:</label>
                        <select id="scheduleHour">
                            <!-- JavaScript ile doldurulacak -->
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="scheduleMinute"><i class="fas fa-stopwatch"></i> Dakika:</label>
                        <select id="scheduleMinute">
                            <!-- JavaScript ile doldurulacak -->
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="scheduleDuration"><i class="fas fa-hourglass-half"></i> Süre (dakika):</label>
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
                    <button id="planliTemizlikAyarlaBtn" class="btn btn-primary"><i class="fas fa-save"></i> Planlı Temizlik Ayarla</button>
                    <button id="planliTemizlikIptalBtn" class="btn btn-danger"><i class="fas fa-times-circle"></i> Planlı Temizliği İptal Et</button>
                </div>
            </div>
            <div class="schedule-status" id="scheduleStatus">
                <span class="schedule-inactive"><i class="fas fa-calendar-times"></i> Planlı temizlik ayarlanmadı</span>
            </div>
        </div>
        
        <!-- İstatistik Paneli -->
        <div class="card">
            <h3><i class="fas fa-chart-line"></i> İstatistikler</h3>
            <div class="stats-grid">
                <div class="stats-item">
                    <div class="stats-label"><i class="fas fa-clock"></i> Toplam Çalışma Süresi</div>
                    <div id="toplamSure" class="stats-value">0 dk</div>
                </div>
                <div class="stats-item">
                    <div class="stats-label"><i class="fas fa-road"></i> Toplam Gidilen Mesafe</div>
                    <div id="toplamMesafe" class="stats-value">0 m</div>
                </div>
                <div class="stats-item">
                    <div class="stats-label"><i class="fas fa-sync-alt"></i> Otomatik Moda Geçiş</div>
                    <div id="otomatikGecis" class="stats-value">0 kere</div>
                </div>
            </div>
            <div class="form-row" style="margin-top: 1.5rem; justify-content: center;">
                <button id="istatistikSifirlaBtn" class="btn btn-warning"><i class="fas fa-redo"></i> İstatistikleri Sıfırla</button>
            </div>
        </div>
        
        <!-- Müzik Kontrol Paneli -->
        <div class="card">
            <h3><i class="fas fa-music"></i> Müzik Kontrolü</h3>
            <div class="music-player">
                <div class="music-info">
                    <h4><i class="fas fa-headphones"></i> Şu An Çalan:</h4>
                    <div id="nowPlaying" class="now-playing">Şarkı seçilmedi</div>
                    <div id="musicStatus" class="music-status"><i class="fas fa-pause"></i> Duraklatıldı</div>
                </div>
                
                <div class="music-controls">
                    <button id="prevBtn" class="btn btn-secondary"><i class="fas fa-step-backward"></i></button>
                    <button id="playBtn" class="btn btn-success"><i class="fas fa-play"></i></button>
                    <button id="pauseBtn" class="btn btn-warning"><i class="fas fa-pause"></i></button>
                    <button id="nextBtn" class="btn btn-secondary"><i class="fas fa-step-forward"></i></button>
                </div>
                
                <div class="slider-container" style="box-shadow: none; padding: 0.5rem;">
                    <h4 style="color: var(--primary-color);"><i class="fas fa-volume-up"></i> Ses Seviyesi</h4>
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
        
        <div class="card">
            <h3><i class="fas fa-digital-tachograph"></i> Sensör Verileri</h3>
            <div class="sensor-panel">
                <div class="sensor-box">
                    <h4><i class="fas fa-ruler-horizontal"></i> Ön Sağ Mesafe</h4>
                    <div id="onSagValue" class="sensor-value">0 cm</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-ruler-horizontal"></i> Ön Sol Mesafe</h4>
                    <div id="onSolValue" class="sensor-value">0 cm</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-ruler-vertical"></i> Sağ Yan Mesafe</h4>
                    <div id="sagYanValue" class="sensor-value">0 cm</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-ruler-vertical"></i> Sol Yan Mesafe</h4>
                    <div id="solYanValue" class="sensor-value">0 cm</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-ruler"></i> Üst Mesafe</h4>
                    <div id="ustValue" class="sensor-value">0 cm</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-thermometer-half"></i> Sıcaklık</h4>
                    <div id="sicaklikValue" class="sensor-value">0 °C</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-tint"></i> Nem</h4>
                    <div id="nemValue" class="sensor-value">0 %</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-wind"></i> Gaz Seviyesi</h4>
                    <div id="gazValue" class="sensor-value">0</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-running"></i> Hareket</h4>
                    <div id="hareketValue" class="sensor-value">Yok</div>
                </div>
                <div class="sensor-box">
                    <h4><i class="fas fa-volume-up"></i> Ses Seviyesi</h4>
                    <div id="sesValue" class="sensor-value">0</div>
                </div>
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
        var otonomMod = 1; // Başlangıçta Mod 1
        
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
                
                songItem.innerHTML = '<span><i class="fas fa-music" style="margin-right: 8px;"></i>' + songs[i] + '</span><button class="play-song-btn"><i class="fas fa-play"></i> Çal</button>';
                
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
            
            // YENİ - Otonom mod butonları
            document.getElementById('otonomMod1Btn').addEventListener('click', function() {
                sendCommand('otonom_mod_1');
                otonomMod = 1;
                updateOtonomModUI();
            });
            document.getElementById('otonomMod2Btn').addEventListener('click', function() {
                sendCommand('otonom_mod_2');
                otonomMod = 2;
                updateOtonomModUI();
            });
            
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
        
        // YENİ - Otonom Mod UI güncelleme
        function updateOtonomModUI() {
            var mod1Btn = document.getElementById('otonomMod1Btn');
            var mod2Btn = document.getElementById('otonomMod2Btn');
            var modStatus = document.getElementById('otonomModStatus');
            
            // Butonların aktiflik durumunu güncelle
            if (otonomMod === 1) {
                mod1Btn.classList.add('active-mod');
                mod2Btn.classList.remove('active-mod');
                modStatus.textContent = 'Mod 1';
            } else {
                mod1Btn.classList.remove('active-mod');
                mod2Btn.classList.add('active-mod');
                modStatus.textContent = 'Mod 2';
            }
        }
        
        // Müzik UI güncelleme
        function updateMusicUI() {
            // Şimdi çalan bilgisi
            document.getElementById('nowPlaying').textContent = songs[currentTrack - 1] || "Şarkı seçilmedi";
            document.getElementById('musicStatus').innerHTML = isPlaying ? 
                '<i class="fas fa-play"></i> Çalıyor' : 
                '<i class="fas fa-pause"></i> Duraklatıldı';
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
                statusElement.innerHTML = '<i class="fas fa-running"></i> Robot Durum: Hareket Ediyor';
                statusElement.className = "robot-status robot-moving";
            } else {
                statusElement.innerHTML = '<i class="fas fa-stop-circle"></i> Robot Durum: Durgun';
                statusElement.className = "robot-status robot-stationary";
            }
        }

        // Plan durumu UI güncelleme
        function updateScheduleUI() {
            var scheduleStatus = document.getElementById('scheduleStatus');
            var planStatus = document.getElementById('planStatus');
            
            if (planliTemizlikAktif) {
                scheduleStatus.innerHTML = '<span class="schedule-active"><i class="fas fa-calendar-check"></i> Planlı temizlik: ' + 
                    (planliSaat < 10 ? '0' + planliSaat : planliSaat) + ':' + 
                    (planliDakika < 10 ? '0' + planliDakika : planliDakika) + 
                    ' (' + planliSure + ' dk)</span>';
                
                planStatus.textContent = 'Aktif';
                planStatus.className = 'status-value active';
            } else {
                scheduleStatus.innerHTML = '<span class="schedule-inactive"><i class="fas fa-calendar-times"></i> Planlı temizlik ayarlanmadı</span>';
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
                    
                    // Otonom mod durumu
                    if (data.otonomMod !== undefined && data.otonomMod !== otonomMod) {
                        otonomMod = data.otonomMod;
                        updateOtonomModUI();
                    }
                    
                    // Plan durumu
                    if (data.planliTemizlik !== undefined) {
                        planliTemizlikAktif = data.planliTemizlik;
                        updateScheduleUI();
                    }
                    
                    // Mesafe sensör değerleri
                    document.getElementById('onSagValue').textContent = data.sagOn + ' cm';
                    document.getElementById('onSolValue').textContent = data.solOn + ' cm';
                    document.getElementById('sagYanValue').textContent = data.sagYan + ' cm';
                    document.getElementById('solYanValue').textContent = data.solYan + ' cm';
                    document.getElementById('ustValue').textContent = data.ust + ' cm';
                    
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

// MJPEG stream handlet s için
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Sensor verilerini ayrıştırma - Tüm sensörleri destekleyecek şekilde güncellendi
void parseSensorData(String data) {
  int startPos = data.indexOf('{');
  int endPos = data.indexOf('}');
  
  if (startPos >= 0 && endPos > startPos) {
    String jsonStr = data.substring(startPos, endPos + 1);
    
    Serial.print("Ayrıştırılan JSON: ");
    Serial.println(jsonStr);
    
    int onPos = jsonStr.indexOf("\"on\":");
    if (onPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', onPos);
      if (kommaPos > onPos) {
        onMesafe = jsonStr.substring(onPos + 5, kommaPos).toInt();
      }
    }
    
    int ustPos = jsonStr.indexOf("\"ust\":");
    if (ustPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', ustPos);
      if (kommaPos > ustPos) {
        ustMesafe = jsonStr.substring(ustPos + 6, kommaPos).toInt();
      }
    }
    
    int sagOnPos = jsonStr.indexOf("\"sagOn\":");
    if (sagOnPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', sagOnPos);
      if (kommaPos > sagOnPos) {
        sagOnMesafe = jsonStr.substring(sagOnPos + 8, kommaPos).toInt();
      }
    }
    
    int solOnPos = jsonStr.indexOf("\"solOn\":");
    if (solOnPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', solOnPos);
      if (kommaPos > solOnPos) {
        solOnMesafe = jsonStr.substring(solOnPos + 8, kommaPos).toInt();
      }
    }
    
    int sagYanPos = jsonStr.indexOf("\"sagYan\":");
    if (sagYanPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', sagYanPos);
      if (kommaPos > sagYanPos) {
        sagYanMesafe = jsonStr.substring(sagYanPos + 9, kommaPos).toInt();
      }
    }
    
    int solYanPos = jsonStr.indexOf("\"solYan\":");
    if (solYanPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', solYanPos);
      if (kommaPos > solYanPos) {
        solYanMesafe = jsonStr.substring(solYanPos + 9, kommaPos).toInt();
      }
    }
    int otonomModPos = jsonStr.indexOf("\"otonomMod\":");
    if (otonomModPos >= 0) {
      int kommaPos = jsonStr.indexOf(',', otonomModPos);
      if (kommaPos > otonomModPos) {
        otonomMod = jsonStr.substring(otonomModPos + 12, kommaPos).toInt();
      } else {
        otonomMod = jsonStr.substring(otonomModPos + 12).toInt();
      }
    }
    
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
    
    robotHareketEdiyor = !(otomatikMod && millis() - sonHareketZamani > 3000);
  }
}

String getStatusJson() {
  String json = "{";
  
  json += "\"otomatik\":" + String(otomatikMod ? "true" : "false") + ",";
  json += "\"manuel\":" + String(manuelMod ? "true" : "false") + ",";
  json += "\"vakum\":" + String(vakumDurumu ? "true" : "false") + ",";
  json += "\"planliTemizlik\":" + String(planliTemizlikAktif ? "true" : "false") + ",";
  json += "\"hareketEdiyor\":" + String(robotHareketEdiyor ? "true" : "false") + ",";
  json += "\"otonomMod\":" + String(otonomMod) + ",";
  json += "\"sagOn\":" + String(sagOnMesafe) + ",";
  json += "\"solOn\":" + String(solOnMesafe) + ",";
  json += "\"sagYan\":" + String(sagYanMesafe) + ",";
  json += "\"solYan\":" + String(solYanMesafe) + ",";
  json += "\"ust\":" + String(ustMesafe) + ",";
  json += "\"sicaklik\":" + String(sicaklik) + ",";
  json += "\"nem\":" + String(nem) + ",";
  json += "\"gaz\":" + String(gazSeviyesi) + ",";
  json += "\"hareket\":" + String(hareketAlgilandi ? "true" : "false") + ",";
  json += "\"ses\":" + String(sesSeviyesi) + ",";
  json += "\"hiz\":" + String(motorHizi) + ",";
  json += "\"muzik\":" + String(muzikCaliyor ? "true" : "false") + ",";
  json += "\"track\":" + String(currentTrack) + ",";
  json += "\"toplamSureDk\":" + String(toplamCalismaSuresi) + ",";
  json += "\"toplamMesafe\":" + String(toplamGidilenMesafe) + ",";
  json += "\"otomatikGecis\":" + String(otomatikModGecis);
  
  json += "}";
  
  return json;
}

void streamHandler(AsyncWebServerRequest *request) { //boundary
  AsyncWebServerResponse *response = request->beginChunkedResponse(STREAM_CONTENT_TYPE, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    static size_t last_frame_size = 0;
    static uint8_t *last_frame_buffer = nullptr;
    static bool is_streaming = false;
    
    if (!is_streaming) {
      if (last_frame_buffer) free(last_frame_buffer);
      last_frame_buffer = nullptr;
      last_frame_size = 0;
      
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
    
    is_streaming = false;
    
    return part_len + fb->len;
  });
  
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Başlatılıyor...");
  
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
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 16;
    config.fb_count = 1;
  }
    esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı, hata: 0x%x", err);
    delay(1000);
    ESP.restart();
  }
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
  
  // ntpye bağlanmac
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getStatusJson());
  });
  
  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request) {
    String cmd;
    if (request->hasParam("cmd")) {
      cmd = request->getParam("cmd")->value();
      Serial.print("Gelen komut: ");
      Serial.println(cmd);  
      
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
      // YENİ - Otonom mod komutları
      else if (cmd == "otonom_mod_1") {
        otonomMod = 1;
        Serial.println("OTONOM_MOD_1");
      }
      else if (cmd == "otonom_mod_2") {
        otonomMod = 2;
        Serial.println("OTONOM_MOD_2");
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
  
  server.on("/stream", HTTP_GET, streamHandler);
  server.begin();
  Serial.println("HTTP Sunucusu başlatıldı");
  Serial.println("ESP32 CAM hazır, sensör verilerini dinliyor...");
}

void loop() {
  if (Serial.available()) {
    String veri = Serial.readStringUntil('\n');
    veri.trim();
        if (veri.startsWith("SENSOR_DATA:")) {
      parseSensorData(veri);
      
      if (otomatikMod) {
        if (robotHareketEdiyor) {
          sonHareketZamani = millis();
        }
      }
    }
    else if (veri == "Otomatik mod acildi") {
      otomatikMod = true;
      manuelMod = false;
      sonHareketZamani = millis(); 
      robotHareketEdiyor = true;   
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
      sonHareketZamani = millis(); 
      robotHareketEdiyor = true;
    }
    else if (veri == "PLANLI_TEMIZLIK_TAMAMLANDI") {
      otomatikMod = false;
      robotHareketEdiyor = false;
    }
    else if (veri.startsWith("Tarama modu başlatılıyor")) {
      Serial.println("Web: Tarama modu algılandı");
    }
    else if (veri == "Otonom Mod 1 seçildi (Çoklu Sensör)") {
      otonomMod = 1;
    }
    else if (veri == "Otonom Mod 2 seçildi (Üst-Ön Sensör)") {
      otonomMod = 2;
    }
    Serial.print("Mega'dan alınan: ");
    Serial.println(veri);
  }
  
  if (otomatikMod && robotHareketEdiyor) {
    if (millis() - sonHareketZamani > 3000) { // 3 saniye
      robotHareketEdiyor = false;
      Serial.println("3 saniye hareketsiz kaldı, robot duruyor");
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi bağlantısı koptu, yeniden bağlanılıyor...");
    WiFi.reconnect();
    
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
  
  // ntp loop yine
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
  
  if (millis() - sonDebugGondermeZamani > DEBUG_GONDER_ARALIK) {
    Serial.println("\n--- DEBUG BILGISI ---");
    Serial.print("Ön Mesafe: ");
    Serial.print(onMesafe);
    Serial.print(" cm, Sağ Ön: ");
    Serial.print(sagOnMesafe);
    Serial.print(" cm, Sol Ön: ");
    Serial.print(solOnMesafe);
    Serial.println(" cm");
    
    Serial.print("Üst Mesafe: ");
    Serial.print(ustMesafe);
    Serial.print(" cm, Sağ Yan: ");
    Serial.print(sagYanMesafe);
    Serial.print(" cm, Sol Yan: ");
    Serial.print(solYanMesafe);
    Serial.println(" cm");
    
    Serial.print("Otonom Mod: ");
    Serial.print(otonomMod);
    Serial.print(", Otomatik Mod: ");
    Serial.println(otomatikMod ? "AÇIK" : "KAPALI");
    
    Serial.println("--- DEBUG SONU ---\n");
    
    sonDebugGondermeZamani = millis();
  }
}
