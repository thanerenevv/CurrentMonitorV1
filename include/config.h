#pragma once

#define ACS_PIN              A0
#define ACS_SAMPLES          1000
#define ACS_VREF             3.3f
#define ACS_ADC_RES          4095.0f
#define ACS_SENSITIVITY      0.066f
#define ACS_MIDPOINT         (ACS_VREF / 2.0f)
#define ACS_OVERCURRENT      28.0f

#define MAINS_VOLTAGE        230.0f

#define OLED_WIDTH           128
#define OLED_HEIGHT          64
#define OLED_ADDR            0x3C
#define OLED_SDA             8
#define OLED_SCL             9
#define OLED_RESET           -1

#define WM_AP_NAME           "currentmonitorsetup"
#define WM_AP_PASSWORD       "123456789"
#define WM_TIMEOUT_SEC       180

#define SAMPLE_INTERVAL_MS   500
#define DISPLAY_INTERVAL_MS  500
#define ENERGY_INTERVAL_MS   1000