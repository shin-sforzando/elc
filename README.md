# Entrance Lock Canceller

[![Last Commit](https://img.shields.io/github/last-commit/shin-sforzando/elc)](https://github.com/shin-sforzando/elc/graphs/commit-activity)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg)](http://commitizen.github.io/cz-cli/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

- [Prerequisites](#prerequisites)
- [How to](#how-to)
  - [Setup](#setup)
    - [Prepare `includes/secrets.h`](#prepare-includessecretsh)
  - [Develop](#develop)
  - [Upload](#upload)
  - [Monitor](#monitor)
  - [Print Enclosure](#print-enclosure)
- [Misc](#misc)
  - [Note](#note)
  - [LICENSE](#license)
  - [Contributors](#contributors)

## Prerequisites

- [M5StickC Plus 2](https://docs.m5stack.com/en/core/M5StickC%20PLUS2)
- [M5Stack Unit Light](https://docs.m5stack.com/en/unit/LIGHT)
- [SwitchBot Bot](https://www.switchbot.jp/products/switchbot-bot)
  - For Bot BLE API info, see [OpenWonderLabs/SwitchBotAPI-BLE](https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/blob/latest/devicetypes/bot.md)

## How to

### Setup

#### Prepare `includes/secrets.h`

Copy from `includes/secrets_template.h` to set your secret values.

### Develop

(T. B. D.)

### Upload

`pio run --target upload` to build and upload programs.

### Monitor

`pio run --target monitor` to start serial monitoring.

### Print Enclosure

(T. B. D.)

## Misc

### Note

This repository is [Commitizen](https://commitizen.github.io/cz-cli/) friendly, following [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow).

### LICENSE

See [LICENSE](./LICENSE).

### Contributors

- [sforzando LLC. and Inc.](https://sforzando.co.jp/)
  - [Shin'ichiro Suzuki](https://github.com/shin-sforzando)
