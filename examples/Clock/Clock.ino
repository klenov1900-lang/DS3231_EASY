// ============================================================
// Clock.ino
// DS3231_EASY + TM1637_6Easy — простые часы.
// Время уже установлено (батарейка CR2032 на DS3231).
// Serial не используется.
// ============================================================
#include <I2C_EASY.h>
#include <DS3231_EASY.h>
#include <TM1637_6Easy.h>

// Пины TM1637
#define PIN_TM_CLK   8
#define PIN_TM_DIO   9

// TM1637: 6 разрядов, яркость 0
TM1637_6Easy display6(PIN_TM_CLK, PIN_TM_DIO, 0);

// ============================================================
// setup
// ============================================================
void setup() {
    // 1. Инициализация I2C на 400 кГц
    i2c_init(400000);

    // 2. Инициализация TM1637
    display6.begin();
    display6.setBrightness(0);

    // 3. Инициализация DS3231
    rtc_init();

    // 4. Проверка связи
    if (!rtc_is_connected()) {
        // RTC не найдена — показываем "Err"
        display6.showString("no rtc");
        while (1);
    }

    // 5. Проверка: время установлено?
    //    Если OSF=1 (осциллятор останавливался) — время невалидно.
    uint8_t hh, mm, ss, dd, mo, yy, dow;
    rtc_get_datetime(&hh, &mm, &ss, &dd, &mo, &yy, &dow);

    if (yy < 20) {
        // RTC сброшена — время не установлено
        display6.showString("set rtc");
        while (1);
    }

    // 6. Приветствие
    display6.showString("Start");
    delay(1000);
    display6.clear();
    display6.update();
}

// ============================================================
// loop
// ============================================================
void loop() {
    static uint32_t last = 0;
    static uint8_t mode = 0;

    // Переключение каждые 3 секунды: время ↔ дата
    if (millis() - last >= 3000) {
        last = millis();
        mode = (mode + 1) % 2;
    }

    uint8_t hh, mm, ss, dd, mo, yy, dow;

    // Одна транзакция — читаем всё
    rtc_get_datetime(&hh, &mm, &ss, &dd, &mo, &yy, &dow);

    if (rtc_last_error() != I2C_OK) {
        // Ошибка I2C — показываем "Err"
        display6.showString("Err rd");
        return;
    }

    // Вывод на TM1637
    if (mode == 0) {
        display6.showValues(hh, mm, ss);    // HH.MM.SS
    } else {
        display6.showValues(dd, mo, yy);    // DD.MM.YY
    }
}
