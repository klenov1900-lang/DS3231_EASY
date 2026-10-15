// ============================================================
// DS3231_EASY.cpp
// Реализация библиотеки для DS3231.
// TWI через I2C_EASY.
// ============================================================
#include "DS3231_EASY.h"

// ============================================================
// ВНУТРЕННИЕ ПЕРЕМЕННЫЕ
// ============================================================
static uint8_t rtc_addr  = RTC_ADDR_DEFAULT;
static uint8_t rtc_error = I2C_OK;

// ============================================================
// BCD ↔ DECIMAL
// ============================================================
static inline uint8_t bcd2dec(uint8_t b) {
    return ((b >> 4) & 0x0F) * 10 + (b & 0x0F);
}

static inline uint8_t dec2bcd(uint8_t d) {
    return ((d / 10) << 4) | (d % 10);
}

// ============================================================
// ВАЛИДАЦИЯ ВХОДНЫХ ЗНАЧЕНИЙ (clamp)
// ============================================================
static inline uint8_t clamp_hour(uint8_t h)    { return (h > 23) ? 23 : h; }
static inline uint8_t clamp_min_sec(uint8_t v) { return (v > 59) ? 59 : v; }
static inline uint8_t clamp_date(uint8_t d)    { return (d < 1 || d > 31) ? 1 : d; }
static inline uint8_t clamp_month(uint8_t m)   { return (m < 1 || m > 12) ? 1 : m; }
static inline uint8_t clamp_dow(uint8_t d)     { return (d < 1 || d > 7) ? 1 : d; }

// ============================================================
// ДЕНЬ НЕДЕЛИ (формула Зеллера), 1=Пн .. 7=Вс
// ============================================================
static uint8_t calc_dow(uint8_t dd, uint8_t mo, uint8_t yy) {
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    uint8_t y = yy, m = mo, d = dd;
    if (m < 3) y--;
    uint8_t dow = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
    return (dow == 0) ? 7 : dow;
}

// ============================================================
// ВНУТРЕННИЕ: OSF
// ============================================================
static uint8_t clear_osf(void) {
    uint8_t status;
    if (i2c_read_reg(rtc_addr, DS3231_REG_STATUS, &status) != I2C_OK) {
        return I2C_ERR_ADDR_NACK;
    }
    status &= ~(1 << DS3231_BIT_OSF);
    if (i2c_write_reg(rtc_addr, DS3231_REG_STATUS, status) != I2C_OK) {
        return I2C_ERR_ADDR_NACK;
    }
    return I2C_OK;
}

// ============================================================
// ВНУТРЕННИЕ: очистка флагов будильников
// ============================================================
static uint8_t clear_alarm_flags(void) {
    uint8_t status;
    if (i2c_read_reg(rtc_addr, DS3231_REG_STATUS, &status) != I2C_OK) {
        return I2C_ERR_ADDR_NACK;
    }
    status &= ~(1 << DS3231_BIT_A1F);
    status &= ~(1 << DS3231_BIT_A2F);
    if (i2c_write_reg(rtc_addr, DS3231_REG_STATUS, status) != I2C_OK) {
        return I2C_ERR_ADDR_NACK;
    }
    return I2C_OK;
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================
void rtc_init(void) {
    rtc_addr  = RTC_ADDR_DEFAULT;
    rtc_error = I2C_OK;
}

// ============================================================
// ДИАГНОСТИКА
// ============================================================
uint8_t rtc_is_connected(void) {
    return i2c_ping(rtc_addr) ? 1 : 0;
}

uint8_t rtc_last_error(void) {
    return rtc_error;
}

// ============================================================
// ЧТЕНИЕ ВРЕМЕНИ
// ============================================================
void rtc_get_time(uint8_t* hh, uint8_t* mm, uint8_t* ss) {
    uint8_t buf[3];
    if (i2c_read_regs(rtc_addr, DS3231_REG_SECONDS, buf, 3) != I2C_OK) {
        *hh = *mm = *ss = 0;
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    *ss = bcd2dec(buf[0] & 0x7F);
    *mm = bcd2dec(buf[1] & 0x7F);
    *hh = bcd2dec(buf[2] & 0x3F);
    rtc_error = I2C_OK;
}

// ============================================================
// ЧТЕНИЕ ДАТЫ
// ============================================================
void rtc_get_date(uint8_t* dd, uint8_t* mo, uint8_t* yy) {
    uint8_t buf[4];
    if (i2c_read_regs(rtc_addr, DS3231_REG_DATE, buf, 4) != I2C_OK) {
        *dd = *mo = *yy = 0;
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    *dd = bcd2dec(buf[0] & 0x3F);
    *mo = bcd2dec(buf[1] & 0x1F);
    *yy = bcd2dec(buf[2]);
    rtc_error = I2C_OK;
}

// ============================================================
// ЧТЕНИЕ ВРЕМЯ + ДАТА + ДЕНЬ НЕДЕЛИ (одна транзакция)
// ============================================================
void rtc_get_datetime(uint8_t* hh, uint8_t* mm, uint8_t* ss,
                      uint8_t* dd, uint8_t* mo, uint8_t* yy,
                      uint8_t* dow) {
    uint8_t buf[7];

    if (i2c_read_regs(rtc_addr, DS3231_REG_SECONDS, buf, 7) != I2C_OK) {
        *hh = *mm = *ss = 0;
        *dd = *mo = *yy = 0;
        *dow = 0;
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    *ss  = bcd2dec(buf[0] & 0x7F);
    *mm  = bcd2dec(buf[1] & 0x7F);
    *hh  = bcd2dec(buf[2] & 0x3F);
    *dow = bcd2dec(buf[3] & 0x07);
    *dd  = bcd2dec(buf[4] & 0x3F);
    *mo  = bcd2dec(buf[5] & 0x1F);
    *yy  = bcd2dec(buf[6]);

    rtc_error = I2C_OK;
}

// ============================================================
// ЧТЕНИЕ ДНЯ НЕДЕЛИ
// ============================================================
uint8_t rtc_get_dow(void) {
    uint8_t dow;
    if (i2c_read_reg(rtc_addr, DS3231_REG_DOW, &dow) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return 0;
    }
    rtc_error = I2C_OK;
    return bcd2dec(dow & 0x07);
}

// ============================================================
// УСТАНОВКА ВРЕМЕНИ
// ============================================================
void rtc_set_time(uint8_t hh, uint8_t mm, uint8_t ss) {
    hh = clamp_hour(hh);
    mm = clamp_min_sec(mm);
    ss = clamp_min_sec(ss);

    uint8_t buf[3];
    buf[0] = dec2bcd(ss) & 0x7F;
    buf[1] = dec2bcd(mm) & 0x7F;
    buf[2] = dec2bcd(hh) & 0x3F;

    if (i2c_write_regs(rtc_addr, DS3231_REG_SECONDS, buf, 3) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    if (clear_osf() != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    rtc_error = I2C_OK;
}

// ============================================================
// УСТАНОВКА ДАТЫ
// ВНИМАНИЕ: автоматически пересчитывает и перезаписывает DOW.
// ============================================================
void rtc_set_date(uint8_t dd, uint8_t mo, uint8_t yy) {
    dd = clamp_date(dd);
    mo = clamp_month(mo);

    uint8_t buf[4];
    buf[0] = dec2bcd(calc_dow(dd, mo, yy));
    buf[1] = dec2bcd(dd) & 0x3F;
    buf[2] = dec2bcd(mo) & 0x1F;
    buf[3] = dec2bcd(yy);

    if (i2c_write_regs(rtc_addr, DS3231_REG_DOW, buf, 4) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    rtc_error = I2C_OK;
}

// ============================================================
// УСТАНОВКА ВРЕМЯ + ДАТА + ДЕНЬ НЕДЕЛИ (одна транзакция)
// ============================================================
void rtc_set_datetime(uint8_t hh, uint8_t mm, uint8_t ss,
                      uint8_t dd, uint8_t mo, uint8_t yy,
                      uint8_t dow) {
    hh  = clamp_hour(hh);
    mm  = clamp_min_sec(mm);
    ss  = clamp_min_sec(ss);
    dd  = clamp_date(dd);
    mo  = clamp_month(mo);
    dow = clamp_dow(dow);

    uint8_t buf[7];

    buf[0] = dec2bcd(ss) & 0x7F;
    buf[1] = dec2bcd(mm) & 0x7F;
    buf[2] = dec2bcd(hh) & 0x3F;
    buf[3] = dec2bcd(dow) & 0x07;
    buf[4] = dec2bcd(dd) & 0x3F;
    buf[5] = dec2bcd(mo) & 0x1F;
    buf[6] = dec2bcd(yy);

    if (i2c_write_regs(rtc_addr, DS3231_REG_SECONDS, buf, 7) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    if (clear_osf() != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    rtc_error = I2C_OK;
}

// ============================================================
// ALARM 1
// ============================================================
void rtc_alarm1_set(uint8_t hh, uint8_t mm, uint8_t ss, uint8_t dow) {
    hh = clamp_hour(hh);
    mm = clamp_min_sec(mm);
    ss = clamp_min_sec(ss);

    uint8_t buf[4];

    buf[0] = dec2bcd(ss) & 0x7F;
    buf[1] = dec2bcd(mm) & 0x7F;
    buf[2] = dec2bcd(hh) & 0x3F;

    if (dow == 0) {
        buf[3] = 0x80;
    } else if (dow <= 7) {
        buf[3] = dec2bcd(dow) & 0x07;
    } else {
        buf[3] = 0x80;   // некорректный dow — игнорируем (каждый день)
    }

    if (i2c_write_regs(rtc_addr, DS3231_REG_ALM1_SEC, buf, 4) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    uint8_t ctrl;
    if (i2c_read_reg(rtc_addr, DS3231_REG_CONTROL, &ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    ctrl |= (1 << DS3231_BIT_A1IE);
    ctrl |= (1 << DS3231_BIT_INTCN);
    if (i2c_write_reg(rtc_addr, DS3231_REG_CONTROL, ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    if (clear_alarm_flags() != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    rtc_error = I2C_OK;
}

// ============================================================
// ALARM 2
// ============================================================
void rtc_alarm2_set(uint8_t hh, uint8_t mm, uint8_t dow) {
    hh = clamp_hour(hh);
    mm = clamp_min_sec(mm);

    uint8_t buf[3];

    buf[0] = dec2bcd(mm) & 0x7F;
    buf[1] = dec2bcd(hh) & 0x3F;

    if (dow == 0) {
        buf[2] = 0x80;
    } else if (dow <= 7) {
        buf[2] = dec2bcd(dow) & 0x07;
    } else {
        buf[2] = 0x80;   // некорректный dow — игнорируем (каждый день)
    }

    if (i2c_write_regs(rtc_addr, DS3231_REG_ALM2_MIN, buf, 3) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    uint8_t ctrl;
    if (i2c_read_reg(rtc_addr, DS3231_REG_CONTROL, &ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    ctrl |= (1 << DS3231_BIT_A2IE);
    ctrl |= (1 << DS3231_BIT_INTCN);
    if (i2c_write_reg(rtc_addr, DS3231_REG_CONTROL, ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    if (clear_alarm_flags() != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    rtc_error = I2C_OK;
}

// ============================================================
// ОТКЛЮЧЕНИЕ ВСЕХ БУДИЛЬНИКОВ
// ============================================================
void rtc_alarm_disable(void) {
    uint8_t ctrl;

    if (i2c_read_reg(rtc_addr, DS3231_REG_CONTROL, &ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    ctrl &= ~(1 << DS3231_BIT_A1IE);
    ctrl &= ~(1 << DS3231_BIT_A2IE);

    if (i2c_write_reg(rtc_addr, DS3231_REG_CONTROL, ctrl) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }

    if (clear_alarm_flags() != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return;
    }
    rtc_error = I2C_OK;
}

// ============================================================
// ПРОВЕРКА СРАБАТЫВАНИЯ БУДИЛЬНИКА
// ============================================================
uint8_t rtc_alarm_check(void) {
    uint8_t status;
    if (i2c_read_reg(rtc_addr, DS3231_REG_STATUS, &status) != I2C_OK) {
        rtc_error = I2C_ERR_ADDR_NACK;
        return 0;
    }

    uint8_t result = 0;
    uint8_t changed = 0;

    if (status & (1 << DS3231_BIT_A1F)) {
        status &= ~(1 << DS3231_BIT_A1F);
        result = 1;
        changed = 1;
    }

    if (status & (1 << DS3231_BIT_A2F)) {
        status &= ~(1 << DS3231_BIT_A2F);
        result = (result == 1) ? 1 : 2;
        changed = 1;
    }

    if (changed) {
        if (i2c_write_reg(rtc_addr, DS3231_REG_STATUS, status) != I2C_OK) {
            rtc_error = I2C_ERR_ADDR_NACK;
            return 0;
        }
    }

    rtc_error = I2C_OK;
    return result;
}