#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


/* ================= WIFI ================= */
const char* ssid = "Akmal Motavala1";
const char* password = "Akmal@031205";

/* ================= PZEM ================= */
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
/* ============== LCD ================= */   // Change to 0x3F if needed
#define LCD_COLS 16
#define LCD_ROWS 2

/* ===== PF ALERT OUTPUT ===== */
#define PF_ALERT_PIN 25
#define PF_THRESHOLD 0.65


int counter = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);


PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

/* ======= HARD THRESHOLDS (CRITICAL) ======= */
#define MIN_VOLTAGE   50.0    // Below this → AC OFF
#define MIN_CURRENT   0.02    // Below this → Load OFF
#define MIN_POWER     1.0     // Ignore ghost watts

/* ============== WEB SERVER ============== */
AsyncWebServer server(80);

/* ============== GLOBAL DATA ============== */
float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float frequency = 0;
float pf = 0;

/* ================= HTML ================= */
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Power Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body{font-family:Arial;background:#f2f2f2;text-align:center}
    .box{background:#fff;padding:20px;margin:15px;border-radius:10px}
  </style>
  <script>
    function update(){
      fetch('/data').then(r=>r.json()).then(d=>{
        for (let k in d) document.getElementById(k).innerHTML = d[k];
      });
    }
    setInterval(update,2000);
  </script>
</head>
<body onload="update()">
  <h2>ESP32 Power Monitor</h2>
  <div class="box">Voltage: <span id="voltage"></span> V</div>
  <div class="box">Current: <span id="current"></span> A</div>
  <div class="box">Power: <span id="power"></span> W</div>
  <div class="box">Energy: <span id="energy"></span> kWh</div>
  <div class="box">Frequency: <span id="frequency"></span> Hz</div>
  <div class="box">Power Factor: <span id="pf"></span></div>
</body>
</html>
)rawliteral";

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, PZEM_RX_PIN, PZEM_TX_PIN);

  // ✅ I2C + LCD INIT (CORRECT PLACE)
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Power Monitor");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  delay(1500);
  lcd.clear();

  pinMode(PF_ALERT_PIN, OUTPUT);
  digitalWrite(PF_ALERT_PIN, LOW);   // Default OFF


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){
    r->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *r){
    String json = "{";
    json += "\"voltage\":\"" + String(voltage,1) + "\",";
    json += "\"current\":\"" + String(current,2) + "\",";
    json += "\"power\":\"" + String(power,1) + "\",";
    json += "\"energy\":\"" + String(energy,3) + "\",";
    json += "\"frequency\":\"" + String(frequency,1) + "\",";
    json += "\"pf\":\"" + String(pf,2) + "\"}";
    r->send(200,"application/json",json);
  });

  server.begin();
}


  

/* ================= LOOP ================= */
void loop() {

  float v  = pzem.voltage();
  float c  = pzem.current();
  float p  = pzem.power();
  float e  = pzem.energy();
  float f  = pzem.frequency();
  float pfv = pzem.pf();

  /* ===== HARD ZERO ENFORCEMENT ===== */
  if (
      isnan(v) || isnan(c) || isnan(p) ||
      v < MIN_VOLTAGE ||
      c < MIN_CURRENT ||
      p < MIN_POWER
     ) {

    voltage = (v > MIN_VOLTAGE) ? v : 0;
    current = 0;
    power = 0;
    pf = 0;
    frequency = 0;

  } else {
    voltage = v;
    current = c;
    power = p;
    energy = e;
    frequency = f;
    pf = pfv;
  }

  /* ===== SERIAL DEBUG ===== */
  Serial.println("------ PZEM DATA ------");
  Serial.print("V: "); Serial.println(voltage);
  Serial.print("I: "); Serial.println(current);
  Serial.print("P: "); Serial.println(power);
  Serial.print("PF: "); Serial.println(pf);

 static float lastV = -1, lastP = -1;

if (abs(voltage - lastV) > 0.5 || abs(power - lastP) > 1.0) {

  lcd.clear();

  if (voltage == 0 && power == 0) {
    lcd.setCursor(0,0);
    lcd.print("AC / Load OFF");
    lcd.setCursor(0,1);
    lcd.print("Waiting...");
  } else {
    lcd.setCursor(0,0);
    lcd.print("V:");
    lcd.print(voltage,1);
    lcd.print(" I:");
    lcd.print(current,2);

    lcd.setCursor(0,1);
    lcd.print("P:");
    lcd.print(power,1);
    lcd.print(" PF:");
    lcd.print(pf,2);
  }

  lastV = voltage;
  lastP = power;
}

/* ===== POWER FACTOR ALERT ===== */
if (pf > 0 && pf < PF_THRESHOLD) {
  digitalWrite(PF_ALERT_PIN, HIGH);   // PF LOW → ALERT
} else {
  digitalWrite(PF_ALERT_PIN, LOW);    // PF OK
}




  delay(2000);
}

