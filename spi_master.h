#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Commands
#define READ_DEVICE_ID_CODE 0xAB
#define READ_MF_DEV_ID_CODE 0x90
#define READ_UNIQUE_ID_CODE 0x4B
#define READ_STAT_REG_1 0x05
#define READ_STAT_REG_2 0x35
#define READ_STAT_REG_3 0x15
#define READ_DATA 0x03
#define WRITE_ENABLE 0x06
#define WRITE_DISABLE 0x04
#define PAGE_PROGRAM 0x02
#define SECTOR_ERASE 0x20

// Status Register 1 Bits
#define WRITE_IN_PROGRESS 0x1
#define WRITE_ENABLE_LATCH 0x2

// Helpful definitions
#define ID_CODE_LEN 0x04

// Functions
void spi_read(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, uint8_t *read_buf, size_t write_len, size_t read_len);
void spi_write(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, size_t len);
void read_device_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf);
void read_mf_dev_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf);
void read_unique_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf);
void read_status_reg(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf, uint8_t status_reg);
void read_data(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf, size_t read_len, uint8_t *addr);
void write_data(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, size_t write_len, uint8_t *addr);