// Copyright (c) 2021 Michael Stoops. All rights reserved.
// Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
// following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
//    disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
//    products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// SPDX-License-Identifier: BSD-3-Clause
//
// Example of an SPI bus master using the PL022 SPI interface

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "spi_master.h"

void spi_read(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, uint8_t *read_buf, size_t write_len, size_t read_len) {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, write_buf, write_len);
    spi_read_blocking(spi, 0x00, read_buf, read_len);
    gpio_put(cs_pin, 1);
}

void spi_write(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, size_t len) {
    gpio_put(cs_pin, 0);
    spi_write_blocking(spi, write_buf, len);
    gpio_put(cs_pin, 1);
}

void read_device_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf) {
    uint8_t write_buf[ID_CODE_LEN] = {READ_DEVICE_ID_CODE, 0xff, 0xff, 0xff};
    spi_read(spi, cs_pin, write_buf, read_buf, 4, 1);
}

void read_mf_dev_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf) {
    uint8_t write_buf[ID_CODE_LEN] = {READ_MF_DEV_ID_CODE, 0xff, 0xff, 0x00};
    spi_read(spi, cs_pin, write_buf, read_buf, 4, 2);
}

void read_unique_id_code(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf) {
    uint8_t write_buf[5] = {READ_UNIQUE_ID_CODE, 0xff, 0xff, 0xff, 0xff};
    spi_read(spi, cs_pin, write_buf, read_buf, 5, 8);
}

void read_status_reg(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf, uint8_t status_reg) {
    uint8_t write_buf[1] = {status_reg};
    spi_read(spi, cs_pin, write_buf, read_buf, 1, 1);
}

void read_data(spi_inst_t *spi, uint cs_pin, uint8_t *read_buf, size_t read_len, uint8_t *addr) {
    uint8_t write_buf[4] = {READ_DATA, addr[0], addr[1], addr[2]};
    spi_read(spi, cs_pin, write_buf, read_buf, 4, read_len);
}

void write_data(spi_inst_t *spi, uint cs_pin, uint8_t *write_buf, size_t write_len, uint8_t *addr) {
    // Calculate full length => page_program command + 3 address bytes + write_buf length
    size_t full_write_len = 4 + write_len;
    uint8_t full_write_buf[full_write_len];

    for (int i = 0; i < full_write_len; i++) {
        if (i == 0) {
            full_write_buf[i] = PAGE_PROGRAM;
        } else if (i >= 1 && i <= 3) {
            full_write_buf[i] = addr[i-1];
        } else {
            full_write_buf[i] = write_buf[i-4];
        }
    }

    uint8_t current_status[1];

    // Ensure write length is smaller than a page (256 bytes)
    if (write_len < 256) {
        // Send write enable command
        uint8_t we[1] = {WRITE_ENABLE};
        spi_write(spi, cs_pin, we, 1);

        // Erase the sector
        read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        if (current_status[0] & WRITE_ENABLE_LATCH) {
            uint8_t sector_erase_buf[4] = {SECTOR_ERASE, addr[0], addr[1], addr[2]};
            spi_write(spi, cs_pin, sector_erase_buf, 4);
        }

        // Wait until write in progress is low
        read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        while (current_status[0] & WRITE_IN_PROGRESS) {
            read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        }

        // Re-enable the write enable
        spi_write(spi, cs_pin, we, 1);

        // Read status register 1 to ensure write enable is high and write in progress is low
        read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        if (current_status[0] & WRITE_ENABLE_LATCH) {
            spi_write(spi, cs_pin, full_write_buf, full_write_len);
        }

        // Wait until write in progress is low
        read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        while (current_status[0] & WRITE_IN_PROGRESS) {
            read_status_reg(spi, cs_pin, current_status, READ_STAT_REG_1);
        }

        // Disable any further writes
        we[0] = WRITE_DISABLE;
        spi_write(spi, cs_pin, we, 1);
    }
}