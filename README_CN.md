# DS3231_EASY

**语言:** [Русский](README.md) · 中文

[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/klenov1900-lang/DS3231_EASY/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/platform-AVR-orange)](https://www.microchip.com/)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com/klenov1900-lang/DS3231_EASY)
[![Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-approved-blue)](https://www.arduino.cc/reference/en/libraries/)

适用于 RTC **DS3231** (I2C) 的简易库。使用 `I2C_EASY` 作为底层 TWI 层。**不依赖 `Wire.h`。** 属于 `*_EASY` 系列。

## ✨ 特性

- ✅ **使用 `I2C_EASY`** — 不依赖 `Wire.h`
- ✅ **读取时间和日期** — 批量，单次事务
- ✅ **设置时间和日期**
- ✅ **两个硬件闹钟** (Alarm 1 和 Alarm 2)
- ✅ **星期几** — 自动计算 (Зеллер 公式)
- ✅ **输入数据校验** — clamp 函数
- ✅ **OSF 处理** — 复位振荡器停止标志
- ✅ **统一诊断** — `rtc_last_error()`
- ✅ **C 兼容** — `extern "C"`

## ⚙️ 要求与兼容性

- **微控制器：** ATmega328P (8 位 AVR)。
- **开发板：** Arduino Uno、Nano、Pro Mini。
- **依赖：** [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY)。
- **其他平台：** ESP8266、ESP32、STM32 — **不支持**（无法直接访问 AVR 寄存器）。

## 📦 安装

### 方式 1：Arduino IDE 库管理器

1. 打开 Arduino IDE
2. 进入 **项目 → 加载库 → 管理库…**
3. 搜索 `DS3231_EASY`
4. 点击 **安装**

### 方式 2：手动（ZIP）

1. 下载 [最新版本](https://github.com/klenov1900-lang/DS3231_EASY/archive/refs/heads/main.zip) 的 ZIP 压缩包
2. Arduino IDE：**项目 → 加载库 → 添加 .ZIP 库…**
3. 选择下载的压缩包

### 方式 3：通过 Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/DS3231_EASY.git
```

## 🚀 快速开始

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

    // 设置时间
    rtc_set_datetime(12, 30, 0, 15, 10, 26, 4);   // 12:30:00, 15.10.26, 星期四
}

void loop() {
    uint8_t hh, mm, ss, dd, mo, yy, dow;

    // ★ 单次事务 — 读取全部
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

## 📖 示例

| 示例 | 描述 | 依赖 |
|---|---|---|
| **Basic** | 最小示例 — 从编译时间设置时间 + Serial | `DS3231_EASY` |
| **Clock** | 简单时钟，基于 TM1637 — 不用 Serial | `DS3231_EASY`, `TM1637_6Easy` |

> **注意：** 示例 `Clock` 需要库 [TM1637_6Easy](https://github.com/klenov1900-lang/TM1637_6Easy)。

## 📚 API

### 初始化

| 函数 | 描述 |
|---|---|
| `rtc_init()` | 初始化（地址 0x68） |
| `rtc_is_connected()` | 检查连接（1 = 已连接） |
| `rtc_last_error()` | 最后错误码 |

### 读取时间和日期

| 函数 | 描述 |
|---|---|
| `rtc_get_time(&hh, &mm, &ss)` | 读取时间 |
| `rtc_get_date(&dd, &mo, &yy)` | 读取日期 |
| `rtc_get_dow()` | 读取星期几（1=周一, ..., 7=周日） |
| `rtc_get_datetime(...)` | **单次事务** — 一次读取全部（7 字节） |

### 设置时间和日期

| 函数 | 描述 |
|---|---|
| `rtc_set_time(hh, mm, ss)` | 设置时间 |
| `rtc_set_date(dd, mo, yy)` | 设置日期（DOW 自动重算） |
| `rtc_set_datetime(...)` | **单次事务** — 一次写入全部（7 字节） |

### 闹钟

| 函数 | 描述 |
|---|---|
| `rtc_alarm1_set(hh, mm, ss, dow)` | 设置闹钟 1（含秒） |
| `rtc_alarm2_set(hh, mm, dow)` | 设置闹钟 2（不含秒） |
| `rtc_alarm_disable()` | 关闭两个闹钟 |
| `rtc_alarm_check()` | 检查触发（0=无, 1=闹钟1, 2=闹钟2） |

> **关于 `dow`：**
> * `0` — 每天
> * `1..7` — 具体星期几（1=周一, ..., 7=周日）
> * 其他值 — 忽略（每天）

## 📊 错误码

| 代码 | 常量 | 描述 |
|---|---|---|
| 0 | `I2C_OK` | 成功 |
| 1 | `I2C_ERR_TIMEOUT` | TWINT 超时 |
| 2 | `I2C_ERR_START` | START 错误 |
| 3 | `I2C_ERR_ADDR_NACK` | 设备无应答 |
| 4 | `I2C_ERR_DATA_NACK` | 数据 NACK |
| 5 | `I2C_ERR_BUS` | 总线状态错误 |
| 6 | `I2C_ERR_ARB_LOST` | 仲裁丢失 |
| 7 | `I2C_ERR_BUS_LOCKED` | SDA 被拉低 |

## 🔧 DS3231 寄存器

| 寄存器 | 地址 | 描述 |
|---|---|---|
| SECONDS | 0x00 | 秒 |
| MINUTES | 0x01 | 分 |
| HOURS | 0x02 | 时 |
| DOW | 0x03 | 星期几 |
| DATE | 0x04 | 日期 |
| MONTH | 0x05 | 月份 |
| YEAR | 0x06 | 年份 |
| ALM1_SEC | 0x07 | 闹钟 1: 秒 |
| ALM1_MIN | 0x08 | 闹钟 1: 分 |
| ALM1_HOUR | 0x09 | 闹钟 1: 时 |
| ALM1_DAY | 0x0A | 闹钟 1: 日 |
| ALM2_MIN | 0x0B | 闹钟 2: 分 |
| ALM2_HOUR | 0x0C | 闹钟 2: 时 |
| ALM2_DAY | 0x0D | 闹钟 2: 日 |
| CONTROL | 0x0E | 控制 |
| STATUS | 0x0F | 状态 |

## 📝 版本历史

| 版本 | 日期 | 变更 |
|---|---|---|
| **v1.0.0** | 2026-10-15 | 首次发布。基础 API、2 个示例、诊断。 |

## 📄 许可证

MIT License。参见 [LICENSE](LICENSE)。

## 📧 联系方式

- **Email:** gray_wolf19@mail.ru
- **GitHub:** [klenov1900-lang](https://github.com/klenov1900-lang)

---

⭐ 如果这个库对您有用，请在 GitHub 上点个星！⭐
