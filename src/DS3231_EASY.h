// ============================================================
// DS3231_EASY.h
// Библиотека для RTC DS3231 (I2C).
// Использует I2C_EASY как низкоуровневый слой TWI.
// ============================================================
#ifndef DS3231_EASY_H
#define DS3231_EASY_H

#include <stdint.h>
#include "I2C_EASY.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// АДРЕС ПО УМОЛЧАНИЮ
// ============================================================
#define RTC_ADDR_DEFAULT   0x68

// ============================================================
// РЕГИСТРЫ DS3231
// ============================================================
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DOW        0x03
#define DS3231_REG_DATE       0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06
#define DS3231_REG_ALM1_SEC   0x07
#define DS3231_REG_ALM1_MIN   0x08
#define DS3231_REG_ALM1_HOUR  0x09
#define DS3231_REG_ALM1_DAY   0x0A
#define DS3231_REG_ALM2_MIN   0x0B
#define DS3231_REG_ALM2_HOUR  0x0C
#define DS3231_REG_ALM2_DAY   0x0D
#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS     0x0F

// ============================================================
// БИТЫ РЕГИСТРА CONTROL (0x0E)
// ============================================================
#define DS3231_BIT_A1IE       0
#define DS3231_BIT_A2IE       1
#define DS3231_BIT_INTCN      2
#define DS3231_BIT_RS1        3
#define DS3231_BIT_RS2        4
#define DS3231_BIT_CONV       5
#define DS3231_BIT_BBSQW      6
#define DS3231_BIT_EOSC       7

// ============================================================
// БИТЫ РЕГИСТРА STATUS (0x0F)
// ============================================================
#define DS3231_BIT_A1F        0
#define DS3231_BIT_A2F        1
#define DS3231_BIT_BSY        2
#define DS3231_BIT_EN32kHz    3
#define DS3231_BIT_OSF        7

// ============================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================
void    rtc_init(void);

// ============================================================
// ВРЕМЯ И ДАТА
// ============================================================
void    rtc_get_time(uint8_t* hh, uint8_t* mm, uint8_t* ss);
void    rtc_get_date(uint8_t* dd, uint8_t* mo, uint8_t* yy);
void    rtc_set_time(uint8_t hh, uint8_t mm, uint8_t ss);

// ВНИМАНИЕ: rtc_set_date автоматически пересчитывает и перезаписывает
// регистр дня недели (0x03) по формуле Зеллера.
void    rtc_set_date(uint8_t dd, uint8_t mo, uint8_t yy);

// ============================================================
// ВРЕМЯ + ДАТА + ДЕНЬ НЕДЕЛИ (одна транзакция, 7 байт)
// ============================================================
void    rtc_get_datetime(uint8_t* hh, uint8_t* mm, uint8_t* ss,
                         uint8_t* dd, uint8_t* mo, uint8_t* yy,
                         uint8_t* dow);
void    rtc_set_datetime(uint8_t hh, uint8_t mm, uint8_t ss,
                         uint8_t dd, uint8_t mo, uint8_t yy,
                         uint8_t dow);

// ============================================================
// ДЕНЬ НЕДЕЛИ (1=Пн, ..., 7=Вс)
// ============================================================
uint8_t rtc_get_dow(void);

// ============================================================
// БУДИЛЬНИКИ
// ============================================================
void    rtc_alarm1_set(uint8_t hh, uint8_t mm, uint8_t ss, uint8_t dow);
void    rtc_alarm2_set(uint8_t hh, uint8_t mm, uint8_t dow);
void    rtc_alarm_disable(void);
uint8_t rtc_alarm_check(void);

// ============================================================
// ДИАГНОСТИКА
// ============================================================
uint8_t rtc_is_connected(void);
uint8_t rtc_last_error(void);

#ifdef __cplusplus
}
#endif

#endif // DS3231_EASY_H