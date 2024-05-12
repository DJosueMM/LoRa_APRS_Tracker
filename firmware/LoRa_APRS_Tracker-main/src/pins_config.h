#ifndef PINS_CONFIG_H_
#define PINS_CONFIG_H_

#undef OLED_SDA
#undef OLED_SCL
#undef OLED_RST


#if defined(TTGO_T_Beam_V1_0) || defined(TTGO_T_Beam_V1_2) || defined(TTGO_T_Beam_V1_0_SX1268) || defined(TTGO_T_Beam_V1_2_SX1262)
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              12
#define GPS_TX              34
#define BUTTON_PIN          38 // The middle button GPIO on the T-Beam
#endif


#if defined(ESP32_DIY_LoRa_GPS) || defined(TTGO_T_LORA32_V2_1_GPS)
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              12
#define GPS_TX              34
#define BATTERY_PIN         35  //LoRa32 Battery PIN 100k/100k
#define LORA_SCK            5
#define LORA_MISO           19
#define LORA_MOSI           27
#define LORA_CS             18  // CS  --> NSS
#define LORA_RST            23
#define LORA_IRQ            26  // IRQ --> DIO0
#endif


#ifdef ESP32_DIY_1W_LoRa_GPS
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              17
#define GPS_TX              16
#define BUTTON_PIN          15
#define BATTERY_PIN         35
#define RADIO_SCLK_PIN      18
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      23
#define RADIO_CS_PIN        5
#define RADIO_RST_PIN       27
#define RADIO_DIO1_PIN      12
#define RADIO_BUSY_PIN      14
#define RADIO_RXEN          32
#define RADIO_TXEN          25
#endif


#if defined(TTGO_T_Beam_V1_0_SX1268) || defined(TTGO_T_Beam_V1_2_SX1262)
#define RADIO_SCLK_PIN      5
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      27
#define RADIO_CS_PIN        18
#define RADIO_DIO0_PIN      26
#define RADIO_RST_PIN       23
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      32
#endif


#ifdef TTGO_T_Beam_V0_7
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              15
#define GPS_TX              12
#define BUTTON_PIN          39
#define BATTERY_PIN         35
#endif


#ifdef TTGO_T_LORA32_V2_1_TNC
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              -1
#define GPS_TX              -1
#define BATTERY_PIN         35
#endif


#ifdef TTGO_T_Beam_S3_SUPREME_V3
#define OLED_SDA            17
#define OLED_SCL            18
#define OLED_RST            16
#define GPS_RX              8
#define GPS_TX              9
#define BUTTON_PIN          0
#define RADIO_SCLK_PIN      12
#define RADIO_MISO_PIN      13
#define RADIO_MOSI_PIN      11
#define RADIO_CS_PIN        10
#define RADIO_DIO0_PIN      -1
#define RADIO_RST_PIN       5
#define RADIO_DIO1_PIN      1
#define RADIO_BUSY_PIN      4
#endif


#ifdef OE5HWN_MeshCom
#define OLED_SDA            21
#define OLED_SCL            22
#define OLED_RST            16
#define GPS_RX              17
#define GPS_TX              16
#define BUTTON_PIN          12
#define BATTERY_PIN         35
#define RADIO_SCLK_PIN      18
#define RADIO_MISO_PIN      19
#define RADIO_MOSI_PIN      23
#define RADIO_CS_PIN        5
#define RADIO_RST_PIN       27
#define RADIO_DIO1_PIN      33
#define RADIO_BUSY_PIN      26
#define RADIO_RXEN          14
#define RADIO_TXEN          13
#endif


#ifdef ESP32_C3_DIY_LoRa_GPS
#define OLED_SDA            8
#define OLED_SCL            9
#define OLED_RST            10
#define GPS_RX              20
#define GPS_TX              21
#define BATTERY_PIN         1
#define LORA_SCK            4
#define LORA_MISO           5
#define LORA_MOSI           6
#define LORA_CS             7  // CS  --> NSS
#define LORA_RST            3
#define LORA_IRQ            2  // IRQ --> DIO0
#endif


#ifdef HELTEC_V3_GPS
#define OLED_SDA            17
#define OLED_SCL            18
#define OLED_RST            21
#define BOARD_I2C_SDA       45
#define BOARD_I2C_SCL       46
#define GPS_RX              47
#define GPS_TX              48
#define BUTTON_PIN          0
#define BATTERY_PIN         1
#define VEXT_CTRL           36
#define ADC_CTRL            37  // Heltec V3 needs ADC_CTRL = LOW powers the voltage divider to read BatteryPin
#define RADIO_SCLK_PIN      9
#define RADIO_MISO_PIN      11
#define RADIO_MOSI_PIN      10
#define RADIO_CS_PIN        8
#define RADIO_RST_PIN       12
#define RADIO_DIO1_PIN      14
#define RADIO_BUSY_PIN      13
#endif


#ifdef HELTEC_WIRELESS_TRACKER
#define GPS_RX              34
#define GPS_TX              33
#define GPS_PPS             36
#define GPS_RESET           35
#define BUTTON_PIN          0
#define BATTERY_PIN         1
#define ADC_CTRL            2   // HELTEC Wireless Tracker ADC_CTRL = HIGH powers the voltage divider to read BatteryPin. Only on V05 = V1.1
#define VEXT_CTRL           3   // To turn on GPS and TFT
#define BOARD_I2C_SDA       7
#define BOARD_I2C_SCL       6
#define RADIO_SCLK_PIN      9
#define RADIO_MISO_PIN      11
#define RADIO_MOSI_PIN      10
#define RADIO_CS_PIN        8
#define RADIO_RST_PIN       12
#define RADIO_DIO1_PIN      14  // SX1262 IRQ
#define RADIO_BUSY_PIN      13  // SX1262 BUSY
#endif


#ifdef TTGO_T_DECK_GPS
#define GPS_RX              43
#define GPS_TX              44
#define BOARD_I2C_SDA       18
#define BOARD_I2C_SCL       8
#define BOARD_POWERON       10
#define BOARD_SDCARD_CS     39

#define RADIO_SCLK_PIN      40
#define RADIO_MISO_PIN      38
#define RADIO_MOSI_PIN      41
#define RADIO_CS_PIN        9
#define RADIO_RST_PIN       17
#define RADIO_DIO1_PIN      45
#define RADIO_BUSY_PIN      13
#define TrackBallCenter     0
#define TrackBallUp         3   // G S1
#define TrackBallDown       15  // G S3
#define TrackBallLeft       1   // G S4
#define TrackBallRight      2   // G S2
/*#define BOARD_I2S_WS    5     // esto es para el audio!
#define BOARD_I2S_BCK   7
#define BOARD_I2S_DOUT  6*/
#endif

#endif