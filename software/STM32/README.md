
# IoT Input Pedal Project (STM32 side)

## Overview

This project implements an **IoT-based input pedal system** using **STM32F103c8t6** and **ESP8266** microcontrollers. The goal is to detect pedal presses through external interrupts and send timestamped events to a server over the internet.

## Features

- 🕒 Real-Time Clock (RTC) synchronization via **NTP**
- ⏱️ Software debounce using **Timer2**
- 📶 UART communication with **DMA** and **IDLE line detection**
- 🔁 Scalable architecture with a basic UART state machine
- 🦶 Pedal press detection via EXTI on `GPIO_PIN_5`

## Functional Description

1. **Time Synchronization**  
   The STM32 receives the current epoch time via UART from the ESP8266. This epoch is converted into human-readable format and configured into the RTC.

2. **Pedal Detection**  
   A GPIO pin (PA5) is used to detect pedal press via an EXTI interrupt. After a debounce check using HAL tick and Timer2, the time and date of the press are fetched from RTC and sent over UART.

3. **UART Communication**  
   - Receives messages using `HAL_UARTEx_ReceiveToIdle_DMA`
   - Detects messages starting with `"epoch:"` to set time
   - Sends acknowledgment `"successfully\n"`
   - Sends timestamp log like `Time:12:34:56 Date:24-06-07\n`

## Peripherals Used

| Peripheral | Purpose                    |
|------------|----------------------------|
| RTC        | Real-time clock tracking   |
| Timer2     | Debounce logic             |
| UART1      | Communication with ESP8266 |
| DMA        | Efficient UART RX/TX       |
| GPIO/EXTI  | Pedal press detection      |

## Hardware Requirements

- STM32F103 microcontroller
- ESP8266 WiFi module
- Push button or pedal switch (connected to PA5)
- LED on PB10 for status (optional)

## Communication Flow

```
[ESP8266] --epoch time--> [STM32]
                         |
                         v
          [STM32 RTC updated]
                         |
          [Pedal Press Detected]
                         |
                         v
           [Get current time/date]
                         |
                         v
           [Send to ESP8266 via UART DMA]
                         |
                         v
           [ESP8266 sends data to server]
```

## Example Messages

**From ESP8266 to STM32:**

```
epoch:1749109618
```

**From STM32 to ESP8266 (after pedal press):**

```
Time:12:45:08 Date:25-06-07
```

**Acknowledgment to ESP8266:**

```
successfully
```

## Future Improvements

- Implement full UART state machine (`State_machine_t`)
- Add timeout or validation for epoch time reception
- Queue multiple pedal presses if UART is busy
- Implement low-power sleep modes between events

<h2 style = "font-weight:bold; font-size: 24px; font-family: Times New Roman;">
Languages and Tools:
</h2>
<p align="left">
<a href="https://www.cprogramming.com/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/c/c-original.svg" alt="c" width="40" height="40"/> </a>
<a href="https://git-scm.com/" target="_blank" rel="noreferrer"> <img src="https://www.vectorlogo.zone/logos/git-scm/git-scm-icon.svg" alt="git" width="40" height="40"/> </a>
<a href="https://code.visualstudio.com/" target="_blank" rel="noreferrer"> <img src="https://cdn.worldvectorlogo.com/logos/visual-studio-code-1.svg" alt="VScode" width="40" height="40"/> </a>
<a href="https://platformio.org//" target="_blank" rel="noreferrer"> <img src="https://cdn.platformio.org/images/platformio-logo.17fdc3bc.png" alt="platform-io" width="40" height="40"/> </a>
<a href="https://www.keil.com/download/product/" target="_blank" rel="noreferrer"> <img src="https://www.logo.wine/a/logo/Keil_(company)/Keil_(company)-Logo.wine.svg" alt="keil" width="100" height="40"/> </a>
</p>
