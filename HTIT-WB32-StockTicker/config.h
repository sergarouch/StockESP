/**
 * ============================================
 *  HTIT-WB32 Stock Ticker - Configuration File
 * ============================================
 * 
 * Edit this file to customize your stock ticker.
 * The main code (HTIT-WB32-StockTicker.ino) will use these settings.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== WiFi Settings ====================
// Primary WiFi network
#define WIFI_SSID_1     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD_1 "YOUR_WIFI_PASSWORD"

// Secondary WiFi network (optional - leave empty if not needed)
#define WIFI_SSID_2     ""
#define WIFI_PASSWORD_2 ""

// ==================== Finnhub API ====================
// Get your free API key at: https://finnhub.io/
#define FINNHUB_API_KEY "YOUR_FINNHUB_API_KEY"

// ==================== Stock Tickers ====================
// Add up to 5 stock symbols (use standard US exchange symbols)
// Examples: AAPL, TSLA, NVDA, MSFT, GOOGL, AMZN, META, AMD, SOFI, INTC
#define STOCK_1  "TSLA"
#define STOCK_2  "AAPL"
#define STOCK_3  "NVDA"
#define STOCK_4  "MSFT"
#define STOCK_5  "GOOGL"

// Number of stocks to display (1-5)
#define NUM_STOCKS 5

// ==================== Portfolio (Shares Owned) ====================
// Enter the number of shares you own for each stock
// Set to 0 if you don't own shares (or just want to track price)
#define SHARES_STOCK_1  80.0    // TSLA shares
#define SHARES_STOCK_2  10.0    // AAPL shares
#define SHARES_STOCK_3  5.0     // NVDA shares
#define SHARES_STOCK_4  0.0     // MSFT shares
#define SHARES_STOCK_5  0.0     // GOOGL shares

// ==================== Display Settings ====================
// Refresh interval in seconds (minimum 10 for free API tier)
// Free Finnhub tier allows 60 calls/minute
#define REFRESH_SECONDS 15

// Flip display upside down? (true/false)
#define FLIP_DISPLAY true

// ==================== Hardware Pins (Don't change unless needed) ====================
// HTIT-WB32 V2 OLED pins
#define OLED_SDA_PIN 4
#define OLED_SCL_PIN 15
#define OLED_RST_PIN 16
#define OLED_I2C_ADDR 0x3c

// Built-in button
#define BUTTON_PIN 0

#endif // CONFIG_H
