#include "max30102.h"
#include "max30102_hw.h"

#define I2C_INT_TIMEOUT_MS 100

// 0 means i2c busy, 1 means i2c complete
static volatile int8_t i2c_done = 0;

// 0 means no error. HAL_I2C_ErrorCallback updates this value on an error
static volatile uint32_t i2c_err = 0;

/*
 * i2c_wait: Intended to be used as a busy wait, but in our driver layer, so that when
 * we make this driver rtos aware, we can deal with it more easily than with polling by
 * just using semaphores.
 * Basic functionality: If the i2c bus is dead for however long the timeout is, we
 * abort the i2c transaction.
 */
static int i2c_wait(I2C_HandleTypeDef *hi2c) {
	uint32_t start = HAL_GetTick();
	while (!i2c_done) {
		if ((HAL_GetTick() - start > I2C_INT_TIMEOUT_MS) || i2c_err != 0) {
			HAL_I2C_Master_Abort_IT(hi2c, (MAX30102_I2C_DEFAULT_ADDR << 1));
			return -1;
		}
	}
	return 0;
}

int max30102_init(I2C_HandleTypeDef *hi2c) {
	uint8_t byte = 0;
	max30102_read_byte(hi2c, MAX30102_REG_PART_ID, &byte);
	if (byte != 0x15) {
		printf("err! wrong value of part_id. byte: %d\r\n", byte);
		return -1;
	}
	printf("byte: %d\r\n", byte);
	return 0;
}

int max30102_reset(I2C_HandleTypeDef *hi2c) {
	max30102_write_byte(hi2c, MAX30102_REG_MODE_CONFIG, 0x40);
	return 0;
}

int max30102_clear_fifo(I2C_HandleTypeDef *hi2c) {
	max30102_write_byte(hi2c, MAX30102_FIFO_WR_PTR, 0x00);
	max30102_write_byte(hi2c, MAX30102_OVF_COUNTER, 0x00);
	max30102_write_byte(hi2c, MAX30102_FIFO_RD_PTR, 0x00);
	return 0;
}

// @ param fifo_a_full Number of empty samples when A_FULL interrupt issued (0 < fifo_a_full < 15)
int max30102_init_fifo(I2C_HandleTypeDef *hi2c, max30102_smp_ave_t smp_ave, uint8_t rollover_en, uint8_t fifo_a_full) {
	uint8_t config = 0;
	config = ((smp_ave & 0x07) << MAX30102_FIFO_CONFIG_SMP_AVE) | ((rollover_en & 0x01) << MAX30102_FIFO_CONFIG_ROLLOVER_EN) | ((fifo_a_full & 0x0F) << MAX30102_FIFO_CONFIG_FIFO_A_FULL);
	max30102_write_byte(hi2c, MAX30102_REG_FIFO_CONFIG, config);
	return 0;
}

int max30102_set_led_pulse_width(I2C_HandleTypeDef *hi2c, max30102_led_pw_t pulse_width) {
	uint8_t config = 0;
	max30102_read_byte(hi2c, MAX30102_REG_SPO2_CONFIG, &config);
	config = (config & 0x7C) | (pulse_width << MAX30102_SPO2_CONFIG_LED_PW);
	max30102_write_byte(hi2c, MAX30102_REG_SPO2_CONFIG, config);
	return 0;
}

int max30102_set_adc_resolution(I2C_HandleTypeDef *hi2c, max30102_adc_t adc_rge) {
	uint8_t config = 0;
	max30102_read_byte(hi2c, MAX30102_REG_SPO2_CONFIG, &config);
	config = (config & 0x1F) | (adc_rge << MAX30102_SPO2_CONFIG_SPO2_ADC_RGE);
	max30102_write_byte(hi2c, MAX30102_REG_SPO2_CONFIG, config);
	return 0;
}

int max30102_set_sampling_rate(I2C_HandleTypeDef *hi2c, max30102_sr_t sample_rate) {
	uint8_t config = 0;
	max30102_read_byte(hi2c, MAX30102_REG_SPO2_CONFIG, &config);
	config = (config & 0x63) | (sample_rate << MAX30102_SPO2_CONFIG_SPO2_SR);
	max30102_write_byte(hi2c, MAX30102_REG_SPO2_CONFIG, config);
	return 0;
}

// Set LED current for the IR led
int max30102_set_led_current_1(I2C_HandleTypeDef *hi2c, float ma) {
	uint8_t config = ma / 0.2;
	max30102_write_byte(hi2c, MAX30102_REG_LED1_PA, config);
	return 0;

}

int max30102_set_mode(I2C_HandleTypeDef *hi2c, max30102_mode_t mode) {
	uint8_t config = 0;
	max30102_read_byte(hi2c, MAX30102_REG_MODE_CONFIG, &config);
	config = (config & 0xF8) | (mode << MAX30102_MODE_CONFIG_MODE);
	max30102_write_byte(hi2c, MAX30102_REG_MODE_CONFIG, config);
	max30102_clear_fifo(hi2c);
	return 0;
}


int max30102_set_a_full(I2C_HandleTypeDef *hi2c, uint8_t enable) {
	uint8_t config = 0;
	max30102_read_byte(hi2c, MAX30102_REG_INT_EN1, &config);
	config &= ~(0x01 << MAX30102_INT_EN1_A_FULL_EN);
	config |= ((enable & 0x01) << MAX30102_INT_EN1_A_FULL_EN);
	max30102_write_byte(hi2c, MAX30102_REG_INT_EN1, config);
	return 0;
}

int max30102_write(I2C_HandleTypeDef *hi2c, uint8_t memAddr, const uint8_t *pData, uint16_t size) {
	i2c_err = 0;
	i2c_done = 0;
	HAL_I2C_Mem_Write_IT(hi2c, MAX30102_I2C_DEFAULT_ADDR << 1, memAddr, I2C_MEMADD_SIZE_8BIT, (uint8_t*) pData, size);
	if (i2c_wait(hi2c) == -1) {
		printf("I2C aborted during max30102_write(). MemAddr: %x. Errcode: %ld \r\n", memAddr, i2c_err);
		return -1;
	}
	return 0;
}

int max30102_read(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *pData, uint16_t size) {
	i2c_err = 0;
	i2c_done = 0;
	HAL_I2C_Mem_Read_IT(hi2c, MAX30102_I2C_DEFAULT_ADDR << 1, memAddr, I2C_MEMADD_SIZE_8BIT, pData, size);
	if (i2c_wait(hi2c) == -1) {
		printf("I2C aborted during max30102_read. MemAddr: %x. Errcode: %ld \r\n", memAddr, i2c_err);
		return -1;
	}
	return 0;
}

int max30102_write_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t byte) {
	if (max30102_write(hi2c, memAddr, &byte, 1) == -1) {
		return -1;
	}
	return 0;
}
int max30102_read_byte(I2C_HandleTypeDef *hi2c, uint8_t memAddr, uint8_t *byte) {
	if (max30102_read(hi2c, memAddr, byte, 1) == -1) {
		return -1;
	}
	return 0;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_done = 1;
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	i2c_done = 1;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
	i2c_err = HAL_I2C_GetError(hi2c);
}
