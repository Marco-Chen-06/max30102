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
