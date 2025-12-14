/*
 * Copyright (c) 2025 Aeybel Varghese <aeybelvarghese@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BOARD__H_
#define _BOARD__H_

#include <cmsis_core_m_defaults.h>
#include <zephyr/sys/util.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/clock_control.h>

#define SYSCLK_DEFAULT_IOSC_HZ MHZ(16)

/*
 * TM4C clock-control subsystem ID
 *
 * This is what clock_control_subsys_t points to for the TM4C clock driver.
 *
 * DT clock specifier format:
 *   <offset bit>
 *
 *  - offset: offset from the clock bank base (0x600, 0x700, 0x800, 0xA00)
 *  - bit:    peripheral instance bit (0..31)
 */
struct tm4c_clk_id {
	uint16_t off;
	uint8_t bit;
};

/*
 * Define a static tm4c_clk_id for a devicetree *instance*.
 *
 * This assumes:
 *  - the node has exactly ONE clocks specifier
 *  - #clock-cells = <2>
 *
 * Usage:
 *   TM4C_CLK_ID_FROM_INST(inst);
 *   clock_control_on(clk_dev, (clock_control_subsys_t)&tm4c_clk_id_##inst);
 */
#define TM4C_CLK_ID_FROM_INST(inst)                                                                \
	static const struct tm4c_clk_id tm4c_clk_id_##inst = {                                     \
		.off = (uint16_t)DT_INST_CLOCKS_CELL(inst, offset),                                \
		.bit = (uint8_t)DT_INST_CLOCKS_CELL(inst, bit),                                    \
	};
/*
 * Helper to get the clock controller device for an instance
 */
#define TM4C_CLK_DEV_FROM_INST(inst) DEVICE_DT_GET(DT_INST_CLOCKS_CTLR(inst))

#endif
