/**
 * HTIT-WB32 Stock Ticker Display (V2)
 * 
 * Displays real-time stock prices on the Heltec WiFi Kit 32 (HTIT-WB32)
 * Uses Finnhub.io API for stock data (free tier available)
 * 
 * Hardware: HTIT-WB32 / Heltec WiFi Kit 32 V2
 * Display: 0.96" SSD1306 OLED (128x64)
 * 
 * ==============================================
 *  CONFIGURATION: Edit config.h to customize!
 * ==============================================
 * 
 * Required Libraries (install via Library Manager):
 * 1. "ESP8266 and ESP32 OLED driver for SSD1306 displays" by ThingPulse
 * 2. "Arduino_JSON" by Arduino
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "SSD1306Wire.h"
#include "config.h"  // <-- User configuration file

// Initialize display
SSD1306Wire display(OLED_I2C_ADDR, OLED_SDA_PIN, OLED_SCL_PIN);

WiFiMulti wifiMulti;

// Build arrays from config
const String stockSymbols[] = {STOCK_1, STOCK_2, STOCK_3, STOCK_4, STOCK_5};
const float sharesOwned[] = {SHARES_STOCK_1, SHARES_STOCK_2, SHARES_STOCK_3, SHARES_STOCK_4, SHARES_STOCK_5};
const String apiKey = FINNHUB_API_KEY;
const unsigned long REFRESH_INTERVAL = REFRESH_SECONDS * 1000;

// Display modes
enum DisplayMode {
  MODE_PRICES,      // Show stock prices
  MODE_HOLDINGS,    // Show portfolio value
  MODE_DETAILS      // Show detailed info (one stock at a time)
};

// Stock data structure
struct StockData {
  String symbol;
  float currentPrice;
  float change;
  float percentChange;
  float previousClose;
  float highPrice;
  float lowPrice;
  bool valid;
};

// Global variables
StockData stocks[5];
volatile DisplayMode currentMode = MODE_PRICES;
volatile int detailStockIndex = 0;
volatile bool buttonPressed = false;
unsigned long lastUpdate = 0;

// Interrupt Service Routine for button press
void IRAM_ATTR buttonISR() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== HTIT-WB32 Stock Ticker ===");
  Serial.println("Configuration loaded from config.h");
  
  // Reset OLED display (required for HTIT-WB32)
  pinMode(OLED_RST_PIN, OUTPUT);
  digitalWrite(OLED_RST_PIN, LOW);
  delay(50);
  digitalWrite(OLED_RST_PIN, HIGH);
  delay(50);
  
  // Initialize display
  display.init();
  if (FLIP_DISPLAY) {
    display.flipScreenVertically();
  }
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();
  
  // Setup button interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  
  // Show startup screen
  displayStartupScreen();
  delay(2000);
  
  // Connect to WiFi
  connectWiFi();
  
  // Initial data fetch
  fetchAllStockData();
  lastUpdate = millis();
}

void loop() {
  // Handle button press
  if (buttonPressed) {
    buttonPressed = false;
    handleButtonPress();
    delay(200);  // Debounce
  }
  
  // Refresh data periodically
  if (millis() - lastUpdate >= REFRESH_INTERVAL) {
    fetchAllStockData();
    lastUpdate = millis();
  }
  
  // Check WiFi connection
  if (wifiMulti.run() != WL_CONNECTED) {
    displayMessage("WiFi Disconnected!", "Reconnecting...");
    connectWiFi();
  }
  
  // Update display based on current mode
  updateDisplay();
  
  delay(100);
}

void displayStartupScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 10, "STOCK");
  display.drawString(64, 28, "TICKER");
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 50, "HTIT-WB32");
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
}

void connectWiFi() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Connecting to WiFi...");
  display.display();
  
  // Add configured WiFi networks
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASSWORD_1);
  if (strlen(WIFI_SSID_2) > 0) {
    wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);
  }
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    display.drawString(attempts * 4, 15, ".");
    display.display();
    attempts++;
  }
  
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println("SSID: " + WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    
    display.clear();
    display.drawString(0, 0, "WiFi Connected!");
    display.drawString(0, 12, "IP: " + WiFi.localIP().toString());
    display.drawString(0, 24, "SSID: " + WiFi.SSID());
    display.display();
    delay(1500);
  } else {
    Serial.println("\nWiFi connection failed!");
    displayMessage("WiFi Failed!", "Check config.h");
  }
}

void fetchAllStockData() {
  Serial.println("\n--- Fetching stock data ---");
  
  for (int i = 0; i < NUM_STOCKS && i < 5; i++) {
    fetchStockData(i);
    delay(250);
  }
}

void fetchStockData(int index) {
  if (index >= NUM_STOCKS) return;
  
  HTTPClient http;
  String symbol = stockSymbols[index];
  
  String url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + apiKey;
  
  Serial.println("Fetching: " + symbol);
  http.begin(url);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Response: " + payload);
    
    JSONVar data = JSON.parse(payload);
    
    if (JSON.typeof(data) != "undefined") {
      stocks[index].symbol = symbol;
      stocks[index].currentPrice = (double)data["c"];
      stocks[index].change = (double)data["d"];
      stocks[index].percentChange = (double)data["dp"];
      stocks[index].previousClose = (double)data["pc"];
      stocks[index].highPrice = (double)data["h"];
      stocks[index].lowPrice = (double)data["l"];
      stocks[index].valid = (stocks[index].currentPrice > 0);
      
      Serial.printf("%s: $%.2f (%.2f%%)\n", 
                    symbol.c_str(), 
                    stocks[index].currentPrice, 
                    stocks[index].percentChange);
    }
  } else {
    Serial.println("HTTP Error: " + String(httpCode));
    stocks[index].valid = false;
  }
  
  http.end();
}

void handleButtonPress() {
  if (currentMode == MODE_PRICES) {
    currentMode = MODE_HOLDINGS;
  } else if (currentMode == MODE_HOLDINGS) {
    currentMode = MODE_DETAILS;
    detailStockIndex = 0;
  } else if (currentMode == MODE_DETAILS) {
    detailStockIndex++;
    if (detailStockIndex >= NUM_STOCKS) {
      currentMode = MODE_PRICES;
      detailStockIndex = 0;
    }
  }
  
  Serial.println("Mode changed: " + String(currentMode));
}

void updateDisplay() {
  switch (currentMode) {
    case MODE_PRICES:
      displayPrices();
      break;
    case MODE_HOLDINGS:
      displayHoldings();
      break;
    case MODE_DETAILS:
      displayDetails();
      break;
  }
}

void displayPrices() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  display.drawString(0, 0, "STOCK PRICES");
  display.drawHorizontalLine(0, 11, 128);
  
  int y = 14;
  int displayCount = min(NUM_STOCKS, 4);
  
  for (int i = 0; i < displayCount; i++) {
    if (stocks[i].valid) {
      display.drawString(0, y, stocks[i].symbol);
      
      String priceStr = "$" + formatPrice(stocks[i].currentPrice);
      display.drawString(35, y, priceStr);
      
      String changeStr;
      if (stocks[i].percentChange >= 0) {
        changeStr = "+" + String(stocks[i].percentChange, 1) + "%";
      } else {
        changeStr = String(stocks[i].percentChange, 1) + "%";
      }
      display.drawString(90, y, changeStr);
    } else {
      display.drawString(0, y, stocks[i].symbol + ": --");
    }
    y += 12;
  }
  
  display.drawString(90, 54, "[PRICE]");
  display.display();
}

void displayHoldings() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  display.drawString(0, 0, "PORTFOLIO VALUE");
  display.drawHorizontalLine(0, 11, 128);
  
  float totalValue = 0;
  float totalChange = 0;
  
  int y = 14;
  int displayCount = min(NUM_STOCKS, 3);
  
  for (int i = 0; i < displayCount; i++) {
    if (stocks[i].valid && i < 5) {
      float value = stocks[i].currentPrice * sharesOwned[i];
      float dayChange = stocks[i].change * sharesOwned[i];
      totalValue += value;
      totalChange += dayChange;
      
      String line = stocks[i].symbol + " " + String((int)sharesOwned[i]) + "x";
      display.drawString(0, y, line);
      
      String valueStr = "$" + formatPrice(value);
      display.drawString(70, y, valueStr);
      
      y += 12;
    }
  }
  
  display.drawHorizontalLine(0, y, 128);
  y += 2;
  
  display.drawString(0, y, "TOTAL:");
  
  String totalStr = "$" + formatPrice(totalValue);
  display.drawString(40, y, totalStr);
  
  String changeStr;
  if (totalChange >= 0) {
    changeStr = "+$" + formatPrice(totalChange);
  } else {
    changeStr = "-$" + formatPrice(abs(totalChange));
  }
  display.drawString(90, y, changeStr);
  
  display.drawString(85, 54, "[HOLD]");
  display.display();
}

void displayDetails() {
  if (detailStockIndex >= NUM_STOCKS || !stocks[detailStockIndex].valid) {
    displayMessage("No Data", "Press button");
    return;
  }
  
  StockData& stock = stocks[detailStockIndex];
  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, stock.symbol);
  
  display.setFont(ArialMT_Plain_10);
  String navStr = String(detailStockIndex + 1) + "/" + String(NUM_STOCKS);
  display.drawString(100, 0, navStr);
  
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 18, "$" + formatPrice(stock.currentPrice));
  
  display.setFont(ArialMT_Plain_10);
  String changeStr;
  if (stock.change >= 0) {
    changeStr = "+" + String(stock.change, 2) + " (+" + String(stock.percentChange, 2) + "%)";
  } else {
    changeStr = String(stock.change, 2) + " (" + String(stock.percentChange, 2) + "%)";
  }
  display.drawString(0, 44, changeStr);
  
  String hlStr = "H:" + formatPrice(stock.highPrice) + " L:" + formatPrice(stock.lowPrice);
  display.drawString(0, 54, hlStr);
  
  display.display();
}

void displayMessage(String line1, String line2) {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, line1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 40, line2);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();
}

String formatPrice(float price) {
  if (price >= 1000) {
    return String(price, 0);
  } else if (price >= 100) {
    return String(price, 1);
  } else {
    return String(price, 2);
  }
}
