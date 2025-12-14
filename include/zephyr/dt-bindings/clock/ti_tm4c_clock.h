/*
 * Copyright (c) 2025 Aeybel Varghese <aeybelvarghese@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DT_BINDINGS_CLOCK_TM4C_CLOCK_H_
#define ZEPHYR_DT_BINDINGS_CLOCK_TM4C_CLOCK_H_

/*
 * Raw Constructor. Clock spec cells are: <rcgc_offset bitpos>
 */
#define TM4C_CLK(rcgc_off, bitpos) (rcgc_off)(bitpos)

/*
 * The TM4C Has 4 Bank of Registers in regards to peripheral clock gating and status checks (offset
 * relative to  SYSCTL base)
 *
 * - RCGCx (run mode clock gating control) base offset 0x600
 * - SCGCx (sleep mode clock gating control) base offset 0x700
 * - DCGCx (deep-sleep mode clock gatic control) base offset 0x800
 * - PR (peripheral ready) base offset 0xA00
 *
 * Each peripheral has registers in each bank at a specific offset. For example GPIO has
 * a offset of 0x08. So its RCGC, SCGC, DCGC, and PR registers are located at
 * 0x608, 0x708, 0x808, and 0xA08 respectively.
 *
 * The bit position within each peripheral refers to the specific peripheral (ex. GPIOA, GPIOB or
 * UART0, UART1)
 */

#define TM4C_CLKBLK_WD     0x00
#define TM4C_CLKBLK_TIMER  0x04
#define TM4C_CLKBLK_GPIO   0x08
#define TM4C_CLKBLK_DMA    0x0C
#define TM4C_CLKBLK_HIB    0x14
#define TM4C_CLKBLK_UART   0x18
#define TM4C_CLKBLK_SSI    0x1C
#define TM4C_CLKBLK_I2C    0x20
#define TM4C_CLKBLK_USB    0x28
#define TM4C_CLKBLK_CAN    0x34
#define TM4C_CLKBLK_ADC    0x38
#define TM4C_CLKBLK_ACMP   0x3C
#define TM4C_CLKBLK_PWM    0x40
#define TM4C_CLKBLK_QEI    0x44
#define TM4C_CLKBLK_EEPROM 0x58
#define TM4C_CLKBLK_WTIMER 0x5C

/* Common convenience constructors for use in device trees, where n is the peripheral number (bitpos
 * within RCGC/PR register)
 */
#define TM4C_CLK_WD(n)            TM4C_CLK(TM4C_CLKBLK_WD, (n))
#define TM4C_CLK_TIMER(n)         TM4C_CLK(TM4C_CLKBLK_TIMER, (n))
#define TM4C_CLK_GPIO(port_index) TM4C_CLK(TM4C_CLKBLK_GPIO, (port_index))
#define TM4C_CLK_DMA(n)           TM4C_CLK(TM4C_CLKBLK_DMA, (n))
#define TM4C_CLK_HIB(n)           TM4C_CLK(TM4C_CLKBLK_HIB, (n))
#define TM4C_CLK_UART(n)          TM4C_CLK(TM4C_CLKBLK_UART, (n))
#define TM4C_CLK_SSI(n)           TM4C_CLK(TM4C_CLKBLK_SSI, (n))
#define TM4C_CLK_I2C(n)           TM4C_CLK(TM4C_CLKBLK_I2C, (n))
#define TM4C_CLK_USB(n)           TM4C_CLK(TM4C_CLKBLK_USB, (n))
#define TM4C_CLK_CAN(n)           TM4C_CLK(TM4C_CLKBLK_CAN, (n))
#define TM4C_CLK_ADC(n)           TM4C_CLK(TM4C_CLKBLK_ADC, (n))
#define TM4C_CLK_ACMP(n)          TM4C_CLK(TM4C_CLKBLK_ACMP, (n))
#define TM4C_CLK_PWM(n)           TM4C_CLK(TM4C_CLKBLK_PWM, (n))
#define TM4C_CLK_QEI(n)           TM4C_CLK(TM4C_CLKBLK_QEI, (n))
#define TM4C_CLK_EEPROM(n)        TM4C_CLK(TM4C_CLKBLK_EEPROM, (n))
#define TM4C_CLK_WTIMER(n)        TM4C_CLK(TM4C_CLKBLK_WTIMER, (n))

#endif /* ZEPHYR_DT_BINDINGS_CLOCK_TM4C_CLOCK_H_ */
