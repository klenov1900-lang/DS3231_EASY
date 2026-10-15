# DS3231_EASY

**Языки:** Русский · [中文](README_CN.md)

[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/klenov1900-lang/DS3231_EASY/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/platform-AVR-orange)](https://www.microchip.com/)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com/klenov1900-lang/DS3231_EASY)
[![Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-approved-blue)](https://www.arduino.cc/reference/en/libraries/)

Простая библиотека для RTC **DS3231** (I2C). Использует `I2C_EASY` как низкоуровневый слой TWI. **Без `Wire.h`.** Часть семейства `*_EASY`.

## ✨ Особенности

- ✅ **Использует `I2C_EASY`** — без `Wire.h`
- ✅ **Чтение времени и даты** — пакетно, одной транзакцией
- ✅ **Установка времени и даты**
- ✅ **Два аппаратных будильника** (Alarm 1 и Alarm 2)
- ✅ **День недели** — автоматический расчёт (формула Зеллера)
- ✅ **Валидация входных данных** — clamp-функции
- ✅ **Работа с OSF** — сброс флага остановки осциллятора
- ✅ **Единая диагностика** — `rtc_last_error()`
- ✅ **C-совместимость** — `extern "C"`

## ⚙️ Требования и совместимость

- **Микроконтроллер:** ATmega328P (8-битный AVR).
- **Платы:** Arduino Uno, Nano, Pro Mini.
- **Зависимости:** [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY).
- **Другие платформы:** ESP8266, ESP32, STM32 — **не поддерживаются** (нет прямого доступа к регистрам AVR).

## 📦 Установка

### Способ 1: Через менеджер библиотек Arduino IDE

1. Откройте Arduino IDE
2. Перейдите в **Скетч → Подключить библиотеку → Управлять библиотеками…**
3. В поиске введите `DS3231_EASY`
4. Нажмите **Установить**

### Способ 2: Вручную (через ZIP)

1. Скачайте ZIP-архив с [последней версией](https://github.com/klenov1900-lang/DS3231_EASY/archive/refs/heads/main.zip)
2. Arduino IDE: **Скетч → Подключить библиотеку → Добавить .ZIP библиотеку…**
3. Выберите скачанный архив

### Способ 3: Через Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/DS3231_EASY.git
```

## 🚀 Быстрый старт

```c
#include <I2C_EASY.h>
#include <DS3231_EASY.h>

void setup() {
    Serial.begin(9600);
    i2c_init(400000);

    rtc_init();

    if (!rtc_is_connected()) {
        Serial.println(F("DS3231 not found"));
        while (1);
    }

    // Установка времени
    rtc_set_datetime(12, 30, 0, 15, 10, 26, 4);   // 12:30:00, 15.10.26, четверг
}

void loop() {
    uint8_t hh, mm, ss, dd, mo, yy, dow;

    // ★ ОДНА ТРАНЗАКЦИЯ — читаем всё
    rtc_get_datetime(&hh, &mm, &ss, &dd, &mo, &yy, &dow);

    Serial.print(hh); Serial.print(':');
    Serial.print(mm); Serial.print(':');
    Serial.print(ss);
    Serial.print("  ");
    Serial.print(dd); Serial.print('.');
    Serial.print(mo); Serial.print(".20");
    Serial.print(yy);
    Serial.print("  DOW: ");
    Serial.println(dow);

    delay(1000);
}
```

## 📖 Примеры

| Пример | Описание | Зависимости |
|---|---|---|
| **Basic** | Минимальный пример — установка времени из компиляции + Serial | `DS3231_EASY` |
| **Clock** | Простые часы на TM1637 — без Serial | `DS3231_EASY`, `TM1637_6Easy` |
> **Примечание:** Пример `Clock` требует библиотеку [TM1637_6Easy](https://github.com/klenov1900-lang/TM1637_6Easy).

## 📚 API

### Инициализация

| Функция | Описание |
|---|---|
| `rtc_init()` | Инициализация (адрес 0x68) |
| `rtc_is_connected()` | Проверка связи (1 = есть) |
| `rtc_last_error()` | Код последней ошибки |

### Чтение времени и даты

| Функция | Описание |
|---|---|
| `rtc_get_time(&hh, &mm, &ss)` | Чтение времени |
| `rtc_get_date(&dd, &mo, &yy)` | Чтение даты |
| `rtc_get_dow()` | Чтение дня недели (1=Пн, ..., 7=Вс) |
| `rtc_get_datetime(...)` | **Одна транзакция** — всё сразу (7 байт) |

### Установка времени и даты

| Функция | Описание |
|---|---|
| `rtc_set_time(hh, mm, ss)` | Установка времени |
| `rtc_set_date(dd, mo, yy)` | Установка даты (DOW пересчитывается) |
| `rtc_set_datetime(...)` | **Одна транзакция** — всё сразу (7 байт) |

### Будильники

| Функция | Описание |
|---|---|
| `rtc_alarm1_set(hh, mm, ss, dow)` | Установка Alarm 1 (с секундами) |
| `rtc_alarm2_set(hh, mm, dow)` | Установка Alarm 2 (без секунд) |
| `rtc_alarm_disable()` | Отключить оба будильника |
| `rtc_alarm_check()` | Проверить срабатывание (0, 1, 2) |

> **Примечание по `dow`:**
> * `0` — каждый день
> * `1..7` — конкретный день недели (1=Пн, ..., 7=Вс)
> * Другое значение — игнорируется (каждый день)

## 📊 Коды ошибок

| Код | Константа | Описание |
|---|---|---|
| 0 | `I2C_OK` | Успех |
| 1 | `I2C_ERR_TIMEOUT` | Таймаут ожидания TWINT |
| 2 | `I2C_ERR_START` | Ошибка START |
| 3 | `I2C_ERR_ADDR_NACK` | Устройство не ответило |
| 4 | `I2C_ERR_DATA_NACK` | NACK на данные |
| 5 | `I2C_ERR_BUS` | Ошибка состояния шины |
| 6 | `I2C_ERR_ARB_LOST` | Потеря арбитража |
| 7 | `I2C_ERR_BUS_LOCKED` | SDA удерживается низко |

## 🔧 Регистры DS3231

| Регистр | Адрес | Описание |
|---|---|---|
| SECONDS | 0x00 | Секунды |
| MINUTES | 0x01 | Минуты |
| HOURS | 0x02 | Часы |
| DOW | 0x03 | День недели |
| DATE | 0x04 | Дата |
| MONTH | 0x05 | Месяц |
| YEAR | 0x06 | Год |
| ALM1_SEC | 0x07 | Alarm 1: секунды |
| ALM1_MIN | 0x08 | Alarm 1: минуты |
| ALM1_HOUR | 0x09 | Alarm 1: часы |
| ALM1_DAY | 0x0A | Alarm 1: день |
| ALM2_MIN | 0x0B | Alarm 2: минуты |
| ALM2_HOUR | 0x0C | Alarm 2: часы |
| ALM2_DAY | 0x0D | Alarm 2: день |
| CONTROL | 0x0E | Управление |
| STATUS | 0x0F | Статус |

## 📝 История версий

| Версия | Дата | Изменения |
|---|---|---|
| **v1.0.0** | 2026-10-15 | Первый релиз. Базовый API, 2 примера, диагностика. |

## 📄 Лицензия

MIT License. См. [LICENSE](LICENSE).

## 📧 Контакты

- **Email:** gray_wolf19@mail.ru
- **GitHub:** [klenov1900-lang](https://github.com/klenov1900-lang)

---

⭐ Если библиотека вам полезна, поставьте звезду на GitHub! ⭐
