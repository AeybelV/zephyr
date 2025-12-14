/*
 * Copyright (c) 2025 Aeybel Varghese <aeybelvarghese@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>

#include <zephyr/kernel.h>

#include "soc.h"

/* Run Mode Clock Gating Control Registers */
#define RCGCUART_R (*((volatile uint32_t *)0x400FE104))
#define RCGCGPIO_R (*((volatile uint32_t *)0x400FE608))

/* GPIO Port A Registers */
#define GPIO_PORTA_AFSEL_R (*((volatile uint32_t *)0x40004420))
#define GPIO_PORTA_DEN_R   (*((volatile uint32_t *)0x4000451C))
#define GPIO_PORTA_PCTL_R  (*((volatile uint32_t *)0x4000452C))
#define GPIO_PORTA_AMSEL_R (*((volatile uint32_t *)0x40004528))

/* GPIO Port F Registers */
#define GPIO_PORTF_DATA_BITS_R ((volatile uint32_t *)0x40025000)
#define GPIO_PORTF_DATA_R      (*((volatile uint32_t *)0x400253FC))
#define GPIO_PORTF_DIR_R       (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_IS_R        (*((volatile uint32_t *)0x40025404))
#define GPIO_PORTF_IBE_R       (*((volatile uint32_t *)0x40025408))
#define GPIO_PORTF_IEV_R       (*((volatile uint32_t *)0x4002540C))
#define GPIO_PORTF_IM_R        (*((volatile uint32_t *)0x40025410))
#define GPIO_PORTF_RIS_R       (*((volatile uint32_t *)0x40025414))
#define GPIO_PORTF_MIS_R       (*((volatile uint32_t *)0x40025418))
#define GPIO_PORTF_ICR_R       (*((volatile uint32_t *)0x4002541C))
#define GPIO_PORTF_AFSEL_R     (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DR2R_R      (*((volatile uint32_t *)0x40025500))
#define GPIO_PORTF_DR4R_R      (*((volatile uint32_t *)0x40025504))
#define GPIO_PORTF_DR8R_R      (*((volatile uint32_t *)0x40025508))
#define GPIO_PORTF_ODR_R       (*((volatile uint32_t *)0x4002550C))
#define GPIO_PORTF_PUR_R       (*((volatile uint32_t *)0x40025510))
#define GPIO_PORTF_PDR_R       (*((volatile uint32_t *)0x40025514))
#define GPIO_PORTF_SLR_R       (*((volatile uint32_t *)0x40025518))
#define GPIO_PORTF_DEN_R       (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_LOCK_R      (*((volatile uint32_t *)0x40025520))
#define GPIO_PORTF_CR_R        (*((volatile uint32_t *)0x40025524))
#define GPIO_PORTF_AMSEL_R     (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R      (*((volatile uint32_t *)0x4002552C))
#define GPIO_PORTF_ADCCTL_R    (*((volatile uint32_t *)0x40025530))
#define GPIO_PORTF_DMACTL_R    (*((volatile uint32_t *)0x40025534))

/* PRGPIO */
#define SYSCTL_PRGPIO_R (*((volatile uint32_t *)0x400FEA08))

/* GPIO Locks */
#define GPIO_LOCK_KEY 0x4C4F434B /*  Unlocks the GPIO_CR register */

/* RCGC UART Bitmasks */
#define RCGCUART_UART0_EN 0x00000001

/* RCGC GPIO Bitmasks */
#define RCGGPIO_PORTA_EN 0x00000001
#define RCGGPIO_PORTF_EN 0x00100000

#ifdef CONFIG_UART_STELLARIS

#include <zephyr/drivers/uart.h>

/**
 * @brief Initializes UART0 by setting up clocks and configuring pin functionality
 */
static void UART0_Init(void)
{
	/* TODO: Until pinctrl is implemented, this clock for Port A must be enabled manually in the
	 * RCGCPIO
	 */
	RCGCGPIO_R |= RCGGPIO_PORTA_EN; /*  Enable Port A GPIO module */
	GPIO_PORTA_AFSEL_R |= 0x03;     /*  Enable alt funct on PA1-0 */
	GPIO_PORTA_DEN_R |= 0x03;       /*  Enable digital I/O on PA1-0 */
	GPIO_PORTA_PCTL_R =
		(GPIO_PORTA_PCTL_R & 0xFFFFFF00) +
		0x00000011; /*  Configure PA1-0 with their alternate function as UART0 TX/RX */
	GPIO_PORTA_AMSEL_R &= ~0x03; /*  Disable analog functionality on PA1-0 */
}

/**
 * @brief To use the UART's on the TM4C, the peripheral clock must be enabled
 * as well as the clock to the appropriate GPIO module. In the absence of proper
 * clock control and pinctrl drivers for the platform, this is a routine that performs
 * the clock and gpio setup manually to emphasize Zephyr bringup on the MCU until those drivers
 * get implemented.
 *
 * UART operations are implemented by the uart_stellaris driver
 *
 * @return
 */
static int tm4c_uart_init(void)
{

#ifdef CONFIG_UART_STELLARIS_PORT_0
	UART0_Init();
#endif /* CONFIG_UART_STELLARIS_PORT_0 */

	return 0;
}

/*
 * Initialize UART peripheral clocks and pins before stellaris driver setups
 *  TODO: This must be replaced with the usage of proper clock control and pinctrl drivers
 */
SYS_INIT(tm4c_uart_init, EARLY, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

#endif /* CONFIG_UART_STELLARIS */
