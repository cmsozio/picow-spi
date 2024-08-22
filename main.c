#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "spi_master.h"

void printbuf(uint8_t buf[], size_t len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

int main() {
    // Enable UART so we can print
    stdio_init_all();
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/spi_master example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    printf("SPI master example\n");

    // Enable SPI 0 at 1 MHz and connect to GPIOs
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    //gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

    gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);

    // Make the SPI pins available to picotool
    bi_decl(bi_4pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI));

    uint8_t in_buf[1];
    read_device_id_code(spi_default, PICO_DEFAULT_SPI_CSN_PIN, in_buf);

    uint8_t mf_dev_buf[2];
    read_mf_dev_id_code(spi_default, PICO_DEFAULT_SPI_CSN_PIN, mf_dev_buf);

    uint8_t unique[8];
    read_unique_id_code(spi_default, PICO_DEFAULT_SPI_CSN_PIN, unique);

    uint8_t stat_reg1[1];
    read_status_reg(spi_default, PICO_DEFAULT_SPI_CSN_PIN, stat_reg1, READ_STAT_REG_1);
    uint8_t stat_reg2[1];
    read_status_reg(spi_default, PICO_DEFAULT_SPI_CSN_PIN, stat_reg2, READ_STAT_REG_2);
    uint8_t stat_reg3[1];
    read_status_reg(spi_default, PICO_DEFAULT_SPI_CSN_PIN, stat_reg3, READ_STAT_REG_3);
    uint8_t data_read[4];
    uint8_t read_address[3] = {0x00, 0x00, 0x00};
    read_data(spi_default, PICO_DEFAULT_SPI_CSN_PIN, data_read, 4, read_address);
    uint8_t data_write[4] = {0x89, 0xab, 0xcd, 0xef};
    write_data(spi_default, PICO_DEFAULT_SPI_CSN_PIN, data_write, 4, read_address);
    uint8_t data_read_after[4];
    read_data(spi_default, PICO_DEFAULT_SPI_CSN_PIN, data_read_after, 4, read_address);


    /* UART Transactions */
    printf("SPI Device ID Code: ");
    printbuf(in_buf, 1);
    printf("SPI Manufacturer and Device ID Code: ");
    printbuf(mf_dev_buf, 2);
    printf("SPI Unique ID Code: ");
    printbuf(unique, 8);
    printf("SPI Status Register 1: ");
    printbuf(stat_reg1, 1);
    printf("SPI Status Register 2: ");
    printbuf(stat_reg2, 1);
    printf("SPI Status Register 3: ");
    printbuf(stat_reg3, 1);
    printf("Read Data: ");
    printbuf(data_read, 4);
    printf("Write Data: ");
    printbuf(data_write, 4);
    printf("Read Data: ");
    printbuf(data_read_after, 4);
    sleep_ms(10*1000);
    
#endif
}