/**
 * HTIT-WB32 Stock Ticker Display
 * 
 * Displays real-time stock prices on the Heltec WiFi Kit 32 (HTIT-WB32)
 * Uses Finnhub.io API for stock data (free tier available)
 * 
 * Hardware: HTIT-WB32 / Heltec WiFi Kit 32
 * Display: 0.96" SSD1306 OLED (128x64)
 * 
 * Features:
 * - Cycles through multiple stock tickers
 * - Shows current price, daily change, and percentage
 * - Button press cycles display modes (prices / holdings / details)
 * - Visual indicators for positive/negative changes
 * 
 * Setup:
 * 1. Get a free API key from https://finnhub.io/
 * 2. Install libraries: "Heltec ESP32 Dev-Boards", "Arduino_JSON"
 * 3. Fill in WiFi credentials and API key below
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include "heltec.h"
#include <String.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

WiFiMulti wifiMulti;

// ==================== CONFIGURATION ====================
// WiFi credentials (supports multiple networks)
#define SSID1 "YOUR_WIFI_SSID"
#define PW1   "YOUR_WIFI_PASSWORD"

#define SSID2 ""  // Optional second network
#define PW2   ""

// Finnhub API key - Get free at https://finnhub.io/
const String FINNHUB_API_KEY = "YOUR_FINNHUB_API_KEY";

// Stock tickers to display (use standard symbols like AAPL, TSLA, NVDA, etc.)
const String stockSymbols[] = {"AAPL", "TSLA", "NVDA", "MSFT", "GOOGL"};
const int NUM_STOCKS = sizeof(stockSymbols) / sizeof(stockSymbols[0]);

// Amount of shares owned for each stock (for holdings calculation)
const float sharesOwned[] = {10.0, 5.0, 15.0, 8.0, 3.0};

// Refresh interval in milliseconds (Finnhub free tier: 60 calls/min)
const unsigned long REFRESH_INTERVAL = 15000;  // 15 seconds

// ==================== END CONFIGURATION ====================

// Pin definitions
const int BUTTON_PIN = 0;  // Built-in PRG button on HTIT-WB32

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
StockData stocks[5];  // Max 5 stocks
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
  
  // Initialize Heltec display
  Heltec.begin(true /*DisplayEnable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  
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
  
  delay(100);  // Small delay to prevent flickering
}

void displayStartupScreen() {
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(64, 10, "STOCK");
  Heltec.display->drawString(64, 28, "TICKER");
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(64, 50, "HTIT-WB32");
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->display();
}

void connectWiFi() {
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, 0, "Connecting to WiFi...");
  Heltec.display->display();
  
  wifiMulti.addAP(SSID1, PW1);
  if (strlen(SSID2) > 0) {
    wifiMulti.addAP(SSID2, PW2);
  }
  
  int attempts = 0;
  while (wifiMulti.run() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    
    // Show progress
    Heltec.display->drawString(attempts * 4, 15, ".");
    Heltec.display->display();
    attempts++;
  }
  
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP: " + WiFi.localIP().toString());
    
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "WiFi Connected!");
    Heltec.display->drawString(0, 12, "IP: " + WiFi.localIP().toString());
    Heltec.display->drawString(0, 24, "SSID: " + WiFi.SSID());
    Heltec.display->display();
    delay(1500);
  } else {
    displayMessage("WiFi Failed!", "Check credentials");
  }
}

void fetchAllStockData() {
  Serial.println("\n--- Fetching stock data ---");
  
  for (int i = 0; i < NUM_STOCKS && i < 5; i++) {
    fetchStockData(i);
    delay(200);  // Small delay between API calls
  }
}

void fetchStockData(int index) {
  if (index >= NUM_STOCKS) return;
  
  HTTPClient http;
  String symbol = stockSymbols[index];
  
  // Finnhub API endpoint for stock quote
  String url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + FINNHUB_API_KEY;
  
  Serial.println("Fetching: " + symbol);
  http.begin(url);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Response: " + payload);
    
    JSONVar data = JSON.parse(payload);
    
    if (JSON.typeof(data) != "undefined") {
      stocks[index].symbol = symbol;
      stocks[index].currentPrice = (double)data["c"];      // Current price
      stocks[index].change = (double)data["d"];            // Change
      stocks[index].percentChange = (double)data["dp"];    // Percent change
      stocks[index].previousClose = (double)data["pc"];    // Previous close
      stocks[index].highPrice = (double)data["h"];         // Day high
      stocks[index].lowPrice = (double)data["l"];          // Day low
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
  
  Serial.println("Mode: " + String(currentMode) + ", Detail Index: " + String(detailStockIndex));
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
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_10);
  
  // Header
  Heltec.display->drawString(0, 0, "STOCK PRICES");
  Heltec.display->drawHorizontalLine(0, 11, 128);
  
  // Display up to 4 stocks in compact format
  int y = 14;
  int displayCount = min(NUM_STOCKS, 4);
  
  for (int i = 0; i < displayCount; i++) {
    if (stocks[i].valid) {
      // Symbol
      Heltec.display->drawString(0, y, stocks[i].symbol);
      
      // Price (right-aligned at x=70)
      String priceStr = "$" + formatPrice(stocks[i].currentPrice);
      Heltec.display->drawString(35, y, priceStr);
      
      // Change indicator and percentage
      String changeStr = (stocks[i].percentChange >= 0 ? "+" : "") + 
                         String(stocks[i].percentChange, 1) + "%";
      
      // Draw arrow indicator
      if (stocks[i].percentChange >= 0) {
        Heltec.display->drawString(90, y, "^" + changeStr);
      } else {
        Heltec.display->drawString(90, y, "v" + changeStr);
      }
    } else {
      Heltec.display->drawString(0, y, stocks[i].symbol + ": --");
    }
    y += 12;
  }
  
  // Mode indicator at bottom
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(90, 54, "[PRICE]");
  
  Heltec.display->display();
}

void displayHoldings() {
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_10);
  
  // Header
  Heltec.display->drawString(0, 0, "PORTFOLIO VALUE");
  Heltec.display->drawHorizontalLine(0, 11, 128);
  
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
      Heltec.display->drawString(0, y, line);
      
      String valueStr = "$" + formatPrice(value);
      Heltec.display->drawString(70, y, valueStr);
      
      y += 12;
    }
  }
  
  // Total line
  Heltec.display->drawHorizontalLine(0, y, 128);
  y += 2;
  
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0, y, "TOTAL:");
  
  String totalStr = "$" + formatPrice(totalValue);
  Heltec.display->drawString(40, y, totalStr);
  
  String changeStr = (totalChange >= 0 ? "+" : "") + "$" + formatPrice(abs(totalChange));
  Heltec.display->drawString(90, y, changeStr);
  
  // Mode indicator
  Heltec.display->drawString(85, 54, "[HOLD]");
  
  Heltec.display->display();
}

void displayDetails() {
  if (detailStockIndex >= NUM_STOCKS || !stocks[detailStockIndex].valid) {
    displayMessage("No Data", "Press button");
    return;
  }
  
  StockData& stock = stocks[detailStockIndex];
  
  Heltec.display->clear();
  
  // Large symbol header
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0, 0, stock.symbol);
  
  // Navigation indicator
  Heltec.display->setFont(ArialMT_Plain_10);
  String navStr = String(detailStockIndex + 1) + "/" + String(NUM_STOCKS);
  Heltec.display->drawString(100, 0, navStr);
  
  // Large current price
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 18, "$" + formatPrice(stock.currentPrice));
  
  // Change info
  Heltec.display->setFont(ArialMT_Plain_10);
  String changeStr = (stock.change >= 0 ? "+" : "") + 
                     String(stock.change, 2) + " (" + 
                     String(stock.percentChange, 2) + "%)";
  Heltec.display->drawString(0, 44, changeStr);
  
  // High/Low
  String hlStr = "H:" + formatPrice(stock.highPrice) + " L:" + formatPrice(stock.lowPrice);
  Heltec.display->drawString(0, 54, hlStr);
  
  // Mode indicator
  Heltec.display->drawString(88, 54, "[DETAIL]");
  
  Heltec.display->display();
}

void displayMessage(String line1, String line2) {
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawString(64, 15, line1);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(64, 40, line2);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->display();
}

// Format price nicely (handles large and small values)
String formatPrice(float price) {
  if (price >= 1000) {
    return String(price, 0);
  } else if (price >= 100) {
    return String(price, 1);
  } else {
    return String(price, 2);
  }
}
