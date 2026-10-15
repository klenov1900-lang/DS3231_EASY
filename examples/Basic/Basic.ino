// ============================================================
// Basic.ino
// DS3231_EASY — минимальный пример.
// Установка времени из компиляции + вывод в Serial.
// ============================================================
#include <I2C_EASY.h>
#include <DS3231_EASY.h>
#include <string.h>
#include <stdio.h>

// ============================================================
// День недели (формула Зеллера) — для set_from_build()
// ============================================================
static uint8_t calc_dow(uint8_t dd, uint8_t mo, uint8_t yy) {
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint8_t y = yy, m = mo, d = dd;
    if (m < 3) y--;
    uint8_t dow = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
    return (dow == 0) ? 7 : dow;
}

// ============================================================
// Установка времени/даты из __DATE__ / __TIME__
// ============================================================
static void set_from_build(void) {
    const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char ms[4] = {0};
    int day, year, h, m, s;

    sscanf(__DATE__, "%3s %d %d", ms, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &h, &m, &s);

    uint8_t mo = 1;
    for (uint8_t i = 0; i < 12; i++) {
        if (strncmp(ms, months + i * 3, 3) == 0) { mo = i + 1; break; }
    }

    uint8_t dow = calc_dow((uint8_t)day, mo, (uint8_t)(year - 2000));
    rtc_set_datetime((uint8_t)h, (uint8_t)m, (uint8_t)s,
                     (uint8_t)day, mo, (uint8_t)(year - 2000), dow);
}

// ============================================================
// setup
// ============================================================
void setup() {
    Serial.begin(9600);
    delay(500);
    Serial.println();
    Serial.println(F("== DS3231_EASY Basic =="));

    i2c_init(400000);
    Serial.println(F("I2C init OK (400 kHz)"));

    rtc_init();

    if (!rtc_is_connected()) {
        Serial.println(F("FAIL: DS3231 not found"));
        while (1);
    }
    Serial.println(F("DS3231 found"));

    // Установка времени из компиляции
    set_from_build();
    Serial.print(F("Set from build: "));
    Serial.print(__DATE__);
    Serial.print(' ');
    Serial.println(__TIME__);

    Serial.println();
}

// ============================================================
// loop
// ============================================================
void loop() {
    static uint32_t lastSerial = 0;

    if (millis() - lastSerial >= 1000) {
        lastSerial = millis();

        uint8_t hh, mm, ss, dd, mo, yy, dow;

        // ★ ОДНА ТРАНЗАКЦИЯ — читаем всё
        rtc_get_datetime(&hh, &mm, &ss, &dd, &mo, &yy, &dow);
        uint8_t err = rtc_last_error();

        Serial.print(F("Time: "));
        if (hh < 10) Serial.print('0');
        Serial.print(hh); Serial.print(':');
        if (mm < 10) Serial.print('0');
        Serial.print(mm); Serial.print(':');
        if (ss < 10) Serial.print('0');
        Serial.print(ss);

        Serial.print(F("  Date: "));
        if (dd < 10) Serial.print('0');
        Serial.print(dd); Serial.print('.');
        if (mo < 10) Serial.print('0');
        Serial.print(mo); Serial.print(".20");
        if (yy < 10) Serial.print('0');
        Serial.print(yy);

        Serial.print(F("  DOW: "));
        Serial.print(dow);

        Serial.print(F("  Err: "));
        Serial.println(err);
    }
}
