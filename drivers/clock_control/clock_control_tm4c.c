/*
 * Copyright (c) 2025 Aeybel Varghese <aeybelvarghese@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT ti_tm4c_clock

#include <soc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_io.h>

LOG_MODULE_REGISTER(clock_control_tm4c, CONFIG_CLOCK_CONTROL_LOG_LEVEL);

#define TM4C_PERIPHERAL_CLOCK_READY_TIMEOUT 10000000

/* SYSCTL bank bases (offsets from SYSCTL base) */
#define TM4C_SYSCTL_RCGC_BASE 0x600u
#define TM4C_SYSCTL_SCGC_BASE 0x700u
#define TM4C_SYSCTL_DCGC_BASE 0x800u
#define TM4C_SYSCTL_PR_BASE   0xA00u

struct tm4c_clk_config {
	mm_reg_t sysctl_base;
	uint32_t sys_clk_hz; /* optional */
};

struct tm4c_clk_data {
	struct k_spinlock lock;
};

/* ========== Helper Functions ========== */

/**
 * @brief Returns the address of a register that lies in a gating control bank.
 *
 * @param sysctl_base Base address of SYSCTL
 * @param bank_base Offset of the bank from SYSCTL
 * @param off Offset within the bank
 * @return Address of the banked register
 */
static inline mm_reg_t tm4c_bank_reg(mm_reg_t sysctl_base, uint32_t bank_base, uint16_t off)
{
	return (mm_reg_t)((mem_addr_t)sysctl_base + (mem_addr_t)bank_base + (mem_addr_t)off);
}

/**
 * @brief Checks if the peripheral is ready for use by software by checking its respective bit
 * in the peripherals peripheral-ready register
 *
 * @param pr_reg Address of the Peripheral Ready Register for the peripheral Block
 * @param bit Bit field within the register corresponding to a specific peripheral (UART0, UART1,
 * etc)
 * @return Error code if peripheral is not ready (-ETIMEDOUT), 0 if ready,
 */
static int tm4c_wait_pr_ready(mm_reg_t pr_reg, uint32_t bit)
{
	for (int waited = 0; waited < TM4C_PERIPHERAL_CLOCK_READY_TIMEOUT; waited++) {
		if (sys_test_bit(pr_reg, bit)) {
			return 0;
		}
		k_busy_wait(1);
	}

	return -ETIMEDOUT;
}

/* ========== Clock Control API ========== */

/**
 * @brief Turns on a clock for a TM4C peripheral
 *
 * @param dev Clock Control Device
 * @param subsys Subsystem/Peripheral wbole clock will be enabled
 * @return Error code
 */
static int tm4c_clock_on(const struct device *dev, clock_control_subsys_t subsys)
{
	const struct tm4c_clk_config *cfg = dev->config;
	struct tm4c_clk_data *data = dev->data;

	/* Get the desired peripheral */
	const struct tm4c_clk_id *id = (const struct tm4c_clk_id *)subsys;

	if (id->bit > 31u) {
		return -EINVAL;
	}

	/* Get the RCGC register for the peripheral */
	mm_reg_t rcgc_addr = tm4c_bank_reg(cfg->sysctl_base, TM4C_SYSCTL_RCGC_BASE, id->off);

	/* Enable clock */
	k_spinlock_key_t key = k_spin_lock(&data->lock);

	sys_set_bit(rcgc_addr, id->bit);

	k_spin_unlock(&data->lock, key);

	mm_reg_t pr_addr = tm4c_bank_reg(cfg->sysctl_base, TM4C_SYSCTL_PR_BASE, id->off);

	/* Ensures the peripheral is ready (wait for clock to stabilize) */
	int ret = tm4c_wait_pr_ready(pr_addr, id->bit);

	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Disables a clock for a TM4C peripheral
 *
 * @param dev Clock Control Device
 * @param subsys Subsystem/peripheral whose clock will be enabled.
 * @return Error code
 */
static int tm4c_clock_off(const struct device *dev, clock_control_subsys_t subsys)
{
	const struct tm4c_clk_config *cfg = dev->config;
	struct tm4c_clk_data *data = dev->data;

	/* Get the desired peripheral */
	const struct tm4c_clk_id *id = (const struct tm4c_clk_id *)subsys;

	if (id->bit > 31u) {
		return -EINVAL;
	}

	/* Get the RCGC register for the peripheral */
	mm_reg_t rcgc_addr = tm4c_bank_reg(cfg->sysctl_base, TM4C_SYSCTL_RCGC_BASE, id->off);

	/* Disable clock */
	k_spinlock_key_t key = k_spin_lock(&data->lock);

	sys_clear_bit(rcgc_addr, id->bit);

	k_spin_unlock(&data->lock, key);

	return 0;
}

/**
 * @brief Returns whether the clock for a TM4C peripheral is enabled
 *
 * @param dev Clock Control Device
 * @param subsys Subsystem/Peripheral whose clock will be enabled
 * @return Clock Control Status (ON/OFF/UNKNOWN) of the peripheral
 */
static enum clock_control_status tm4c_clock_get_status(const struct device *dev,
						       clock_control_subsys_t subsys)
{
	const struct tm4c_clk_config *cfg = dev->config;

	/* Sanity Checks */
	if (subsys == NULL) {
		return CLOCK_CONTROL_STATUS_UNKNOWN;
	}

	const struct tm4c_clk_id *id = (const struct tm4c_clk_id *)subsys;

	if (id->bit > 31u) {
		return CLOCK_CONTROL_STATUS_UNKNOWN;
	}

	/* Get Address for the peripheral block RCGC register */
	mm_reg_t rcgc_addr = tm4c_bank_reg(cfg->sysctl_base, TM4C_SYSCTL_RCGC_BASE, id->off);

	/* Tests whether that specific peripheral (a bit field in the RCGC) is enabled */
	return (sys_test_bit(rcgc_addr, id->bit) ? CLOCK_CONTROL_STATUS_ON
						 : CLOCK_CONTROL_STATUS_OFF);
}

/**
 * @brief Returns the Clock Rate of a Peripheral, or in the case of the TM4C, the system clock
 * frequency.
 *
 * @param dev Clock Control Device
 * @param subsys Subsystem/Peripheral whose clock will be enabled
 * @param rate Destination address of where to store the rate
 * @return Error code
 */
static int tm4c_clock_get_rate(const struct device *dev, clock_control_subsys_t subsys,
			       uint32_t *rate)
{
	ARG_UNUSED(subsys);

	const struct tm4c_clk_config *cfg = dev->config;

	/* Sanit Checks */
	if (rate == NULL) {
		return -EINVAL;
	}

	/* Return System Clock frequency */
	*rate = cfg->sys_clk_hz;

	return 0;
}

/* ========== Clock Control ========= */

static const struct clock_control_driver_api tm4c_clock_api = {
	.on = tm4c_clock_on,
	.off = tm4c_clock_off,
	.get_status = tm4c_clock_get_status,
	.get_rate = tm4c_clock_get_rate,
};

static int tm4c_clk_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}
#define TM4C_SYSCTL_BASE_FROM_SYSCON(node_id) DT_REG_ADDR(DT_PHANDLE(node_id, syscon))

#define TM4C_CLK_INIT(inst)                                                                        \
	static struct tm4c_clk_data tm4c_clk_data_##inst;                                          \
	static const struct tm4c_clk_config tm4c_clk_cfg_##inst = {                                \
		.sysctl_base = (mm_reg_t)TM4C_SYSCTL_BASE_FROM_SYSCON(DT_DRV_INST(inst)),          \
		.sys_clk_hz = DT_INST_PROP_OR(inst, clock_frequency, 0),                           \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, tm4c_clk_init, NULL, &tm4c_clk_data_##inst,                    \
			      &tm4c_clk_cfg_##inst, PRE_KERNEL_1,                                  \
			      CONFIG_CLOCK_CONTROL_INIT_PRIORITY, &tm4c_clock_api);

DT_INST_FOREACH_STATUS_OKAY(TM4C_CLK_INIT)
