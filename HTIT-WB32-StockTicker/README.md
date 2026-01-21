# HTIT-WB32 Stock Ticker

Real-time stock ticker display for the Heltec WiFi Kit 32 (HTIT-WB32) ESP32 board with built-in 0.96" OLED display.

![HTIT-WB32](../cryptoESPHeltec32.jpg)

## Features

- **Real-time stock prices** from Finnhub.io API
- **Multiple display modes** (toggle with PRG button):
  - **Prices**: Shows up to 4 stocks with price and % change
  - **Holdings**: Portfolio value based on shares owned
  - **Details**: Detailed view of individual stocks (high/low/change)
- **Auto-refresh** every 15 seconds
- **Multiple WiFi** network support
- **Visual indicators** for positive/negative changes

## Hardware Requirements

- **Heltec WiFi Kit 32 (HTIT-WB32)**
  - ESP32 with built-in 0.96" SSD1306 OLED (128x64)
  - Built-in PRG button on GPIO 0
  - Display pins: SDA=4, SCL=15, RST=16 (handled by library)

## Software Requirements

### Arduino IDE Setup

1. **Add ESP32 Board Support**
   - Go to `File > Preferences`
   - Add to "Additional Board Manager URLs":
     ```
     https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/releases/download/0.0.9/package_heltec_esp32_index.json
     ```
   - Go to `Tools > Board > Boards Manager`
   - Search for "Heltec ESP32" and install

2. **Install Required Libraries** (via Library Manager):
   - `Arduino_JSON` (by Arduino)
   
3. **Select Board**
   - Go to `Tools > Board > Heltec ESP32 Arduino`
   - Select `WiFi Kit 32`

### API Key Setup

1. Go to [Finnhub.io](https://finnhub.io/)
2. Create a free account
3. Get your API key from the dashboard
4. Free tier includes 60 API calls/minute

## Configuration

Edit the following in `HTIT-WB32-StockTicker.ino`:

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
- `AAPL` - Apple Inc.
- `TSLA` - Tesla Inc.
- `NVDA` - NVIDIA Corporation
- `MSFT` - Microsoft Corporation
- `GOOGL` - Alphabet Inc.
- `AMZN` - Amazon.com Inc.
- `META` - Meta Platforms Inc.
- `AMD` - Advanced Micro Devices
- `INTC` - Intel Corporation

## Usage

### Display Modes

Press the **PRG button** to cycle through modes:

1. **PRICES Mode** - Shows all stocks with current price and % change
   ```
   STOCK PRICES
   ─────────────────
   AAPL  $189.95  ^+1.2%
   TSLA  $248.50  v-0.8%
   NVDA  $495.22  ^+2.1%
   MSFT  $378.91  ^+0.5%
   ```

2. **HOLDINGS Mode** - Shows portfolio value
   ```
   PORTFOLIO VALUE
   ─────────────────
   AAPL 10x     $1899.50
   TSLA 5x      $1242.50
   NVDA 15x     $7428.30
   ─────────────────
   TOTAL: $10570  +$125
   ```

3. **DETAILS Mode** - Detailed single stock view (cycles through each)
   ```
   AAPL           1/5
   
   $189.95
   
   +2.50 (+1.34%)
   H:191.20 L:187.50
   ```

## Troubleshooting

### "HTTP Error" displayed
- Check your API key is correct
- Verify WiFi is connected
- Finnhub free tier has 60 calls/min limit

### Display not working
- Ensure you selected "WiFi Kit 32" board
- Check the Heltec library is installed correctly

### WiFi won't connect
- Verify SSID and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

## API Response Format

Finnhub returns:
```json
{
  "c": 189.95,   // Current price
  "d": 2.50,    // Change
  "dp": 1.34,   // Percent change
  "h": 191.20,  // High price of the day
  "l": 187.50,  // Low price of the day
  "o": 188.00,  // Open price
  "pc": 187.45  // Previous close
}
```

## License

MIT License - Feel free to modify and share!

## Credits

Based on the original CryptoESP project, adapted for stock market data.
