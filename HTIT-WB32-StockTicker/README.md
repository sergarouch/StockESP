# HTIT-WB32 Stock Ticker

Real-time stock ticker display for the Heltec WiFi Kit 32 (HTIT-WB32) ESP32 board with built-in 0.96" OLED display.

## Features

- **Real-time stock prices** from Finnhub.io API (free)
- **3 Display modes** (toggle with PRG button):
  - **Prices**: Up to 4 stocks with price and % change
  - **Holdings**: Portfolio value based on shares owned
  - **Details**: Detailed single stock view with high/low
- **Auto-refresh** every 15 seconds
- **Multiple WiFi** network support
- **Visual indicators** for gains/losses

## Hardware

- **Board**: Heltec WiFi Kit 32 (HTIT-WB32) V2
- **Display**: Built-in 0.96" SSD1306 OLED (128x64)
- **Button**: Built-in PRG button (GPIO 0)
- **Display Pins**: SDA=4, SCL=15, RST=16

## Required Libraries

Install these via **Arduino IDE → Tools → Manage Libraries**:

| Library | Author | Purpose |
|---------|--------|---------|
| ESP8266 and ESP32 OLED driver for SSD1306 displays | ThingPulse | OLED display |
| Arduino_JSON | Arduino | JSON parsing |

## Arduino IDE Setup

### 1. Add ESP32 Board Support

1. Go to **File → Preferences**
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search **"esp32"** and install **"esp32 by Espressif Systems"**

### 2. Select Board Settings

| Setting | Value |
|---------|-------|
| Board | ESP32 Dev Module |
| Upload Speed | 921600 |
| CPU Frequency | 240MHz |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Flash Size | 4MB |
| Partition Scheme | Default 4MB |

### 3. Install USB Driver

The HTIT-WB32 uses a CP210x USB-to-Serial chip. If your computer doesn't recognize the board:

1. Download [CP210x Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)
2. Install and restart your computer
3. Plug in the board
4. Select the new COM port in **Tools → Port**

## API Key Setup

1. Go to [finnhub.io](https://finnhub.io/)
2. Create a free account
3. Copy your API key from the dashboard
4. Free tier: 60 API calls/minute

## Configuration

Edit these values in `HTIT-WB32-StockTicker.ino`:

```cpp
// WiFi credentials
#define SSID1 "YOUR_WIFI_SSID"
#define PW1   "YOUR_WIFI_PASSWORD"

// Finnhub API key
const String FINNHUB_API_KEY = "YOUR_FINNHUB_API_KEY";

// Stock tickers to display
const String stockSymbols[] = {"AAPL", "TSLA", "NVDA", "MSFT", "GOOGL"};

// Shares owned (for portfolio calculation)
const float sharesOwned[] = {10.0, 5.0, 15.0, 8.0, 3.0};

// Refresh interval (milliseconds)
const unsigned long REFRESH_INTERVAL = 15000;
```

## Stock Symbols

Use standard US stock exchange symbols:

| Symbol | Company |
|--------|---------|
| AAPL | Apple Inc. |
| TSLA | Tesla Inc. |
| NVDA | NVIDIA Corporation |
| MSFT | Microsoft Corporation |
| GOOGL | Alphabet Inc. |
| AMZN | Amazon.com Inc. |
| META | Meta Platforms Inc. |
| AMD | Advanced Micro Devices |
| SOFI | SoFi Technologies |
| INTC | Intel Corporation |

## Usage

### Display Modes

Press the **PRG button** to cycle through modes:

**1. PRICES Mode**
```
STOCK PRICES
─────────────────────
TSLA  $248.50   +1.2%
SOFI  $12.35    -0.8%
NVDA  $495.22   +2.1%
MSFT  $378.91   +0.5%
            [PRICE]
```

**2. HOLDINGS Mode**
```
PORTFOLIO VALUE
─────────────────────
TSLA 80x      $19880
SOFI 0x       $0
NVDA 0x       $0
─────────────────────
TOTAL: $19880  +$240
             [HOLD]
```

**3. DETAILS Mode** (cycles through each stock)
```
TSLA              1/5

$248.50

+3.25 (+1.32%)
H:251.20 L:245.10
```

## Troubleshooting

### "Port not available"
- Install the [CP210x USB driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Try a different USB cable (some are charge-only)
- Restart Arduino IDE after plugging in the board

### Upload fails
- Hold the **PRG button** while clicking Upload
- Release after "Connecting..." appears in the console

### Display shows nothing
- Check that OLED RST pin reset is working (code does this automatically)
- Verify I2C address is 0x3c (most common)

### "HTTP Error" on display
- Verify your Finnhub API key is correct
- Check WiFi is connected (look at Serial Monitor)
- Free tier limit: 60 calls/min

### WiFi won't connect
- ESP32 only supports **2.4GHz** networks (not 5GHz)
- Check SSID and password for typos
- Move closer to the router

## Wiring (for reference)

The HTIT-WB32 has the OLED built-in with these internal connections:

| OLED Pin | ESP32 GPIO |
|----------|------------|
| SDA | GPIO 4 |
| SCL | GPIO 15 |
| RST | GPIO 16 |

## API Response

Finnhub returns this JSON structure:
```json
{
  "c": 248.50,   // Current price
  "d": 3.25,    // Change ($)
  "dp": 1.32,   // Change (%)
  "h": 251.20,  // Day high
  "l": 245.10,  // Day low
  "o": 246.00,  // Open price
  "pc": 245.25  // Previous close
}
```

## License

MIT License - Feel free to modify and share!

## Credits

Based on the [CryptoESP](https://github.com/amartora/cryptoESP) project, adapted for stock market data using standard ESP32 libraries.
