/*-----------------------------------------------------------------------------
	Copyright 2015 Benjamin Vedder	benjamin@vedder.se

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include "hw.h"
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"

// Motor IDs
static const uint8_t MOTOR_TOP = 10;
static const uint8_t MOTOR_LEFT = 11;
static const uint8_t MOTOR_RIGHT = 12;

// Variables
static volatile bool i2c_running = false;

// I2C configuration
static const I2CConfig i2cfg = {
	OPMODE_I2C,
	100000,
	STD_DUTY_CYCLE};

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
uint8_t hw_id_from_pins(void)
{
	stm32_gpio_t *hw_id_ports[] = {HW_ID_PIN_GPIOS_VOLLEY};
	const uint16_t hw_id_pins[] = {HW_ID_PIN_PINS_VOLLEY};
	const uint16_t hw_id_num_pins = sizeof(hw_id_pins) / sizeof(uint16_t);
	bool pin_state[hw_id_num_pins];

	for (uint8_t i = 0; i < hw_id_num_pins; i++)
	{
		palSetPadMode(hw_id_ports[i], hw_id_pins[i], PAL_MODE_INPUT);
		pin_state[i] = palReadPad(hw_id_ports[i], hw_id_pins[i]);
	}

	// The ID is the binary combination of the two resistors + 10
	// Top: PC3 = 0, PC1 = 0 (CAN ID = 10d)
	// Left: PC3 = 0, PC1 = 1 (CAN ID = 11d)
	// Right: PC3 = 1, PC1 = 0 (CAN ID = 12d)
	// Unused: PC3 = 1, PC1 = 1 (CAN ID = 13d)
	uint8_t id = (uint8_t)((pin_state[1] << 1 | pin_state[0]) + 10);
	return id;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_init_app_config(app_configuration *conf)
{
	uint8_t id = hw_id_from_pins();
	conf->controller_id = id;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_init_mc_config(mc_configuration *conf)
{
	uint8_t id = hw_id_from_pins();
	if (id == MOTOR_LEFT)
		conf->m_invert_direction = false;
	else
		conf->m_invert_direction = true;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_init_gpio(void)
{
	// GPIO clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// LEDs
	palSetPadMode(GPIOC, 4,
				  PAL_MODE_OUTPUT_PUSHPULL |
					  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOC, 5,
				  PAL_MODE_OUTPUT_PUSHPULL |
					  PAL_STM32_OSPEED_HIGHEST);

	// GPIOC (ENABLE_GATE)
	palSetPadMode(GPIOC, 10,
				  PAL_MODE_OUTPUT_PUSHPULL |
					  PAL_STM32_OSPEED_HIGHEST);
	DISABLE_GATE();

	// GPIOB (DCCAL)
	palSetPadMode(GPIOB, 12,
				  PAL_MODE_OUTPUT_PUSHPULL |
					  PAL_STM32_OSPEED_HIGHEST);

	// GPIOA Configuration: Channel 1 to 3 as alternate function push-pull
	palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);

	palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);

	// Hall sensors
	palSetPadMode(HW_HALL_ENC_GPIO1, HW_HALL_ENC_PIN1, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO2, HW_HALL_ENC_PIN2, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3, PAL_MODE_INPUT_PULLUP);

	// Fault pin
	palSetPadMode(GPIOC, 12, PAL_MODE_INPUT_PULLUP);

	// ADC Pins
	palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 2, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 3, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG);

	palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOB, 1, PAL_MODE_INPUT_ANALOG);

	palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_setup_adc_channels(void)
{
	// ADC1 regular channels
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 3, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_15Cycles);

	// ADC2 regular channels
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 2, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 3, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 4, ADC_SampleTime_15Cycles);

	// ADC3 regular channels
	ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 1, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 2, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 3, ADC_SampleTime_15Cycles);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 4, ADC_SampleTime_15Cycles);

	// Injected channels
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_9, 2, ADC_SampleTime_15Cycles);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_start_i2c(void)
{
	i2cAcquireBus(&HW_I2C_DEV);

	if (!i2c_running)
	{
		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
						  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);
		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
						  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);

		i2cStart(&HW_I2C_DEV, &i2cfg);
		i2c_running = true;
	}

	i2cReleaseBus(&HW_I2C_DEV);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_stop_i2c(void)
{
	i2cAcquireBus(&HW_I2C_DEV);

	if (i2c_running)
	{
		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN, PAL_MODE_INPUT);
		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN, PAL_MODE_INPUT);

		i2cStop(&HW_I2C_DEV);
		i2c_running = false;
	}

	i2cReleaseBus(&HW_I2C_DEV);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void hw_try_restore_i2c(void)
{
	if (i2c_running)
	{
		i2cAcquireBus(&HW_I2C_DEV);

		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);

		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);

		palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		palSetPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);

		chThdSleep(1);

		for (int i = 0; i < 16; i++)
		{
			palClearPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
			chThdSleep(1);
			palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
			chThdSleep(1);
		}

		// Generate start then stop condition
		palClearPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);
		chThdSleep(1);
		palClearPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		chThdSleep(1);
		palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		chThdSleep(1);
		palSetPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);

		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
						  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);

		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
						  PAL_STM32_OTYPE_OPENDRAIN |
						  PAL_STM32_OSPEED_MID1 |
						  PAL_STM32_PUDR_PULLUP);

		HW_I2C_DEV.state = I2C_STOP;
		i2cStart(&HW_I2C_DEV, &i2cfg);

		i2cReleaseBus(&HW_I2C_DEV);
	}
}
