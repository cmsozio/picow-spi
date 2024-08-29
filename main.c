#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "spi_master.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1
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

static int data_rec = 0;
uint8_t prog_data[256];
uint8_t prog_addr[3];
uint8_t operation;

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);

        switch (operation) {
            case 0: 
                if (ch > 0 && ch < 2) {
                    operation = ch;
                }
                break;
            case 1:
                if (data_rec < 256) {
                    prog_data[data_rec] = ch;
                    data_rec++;
                } else if (data_rec >= 256 & data_rec < 259) {
                    prog_addr[data_rec-256] = ch;
                    data_rec++;
                } else {

                }
                break;
        }
        /*
        // Can we send it back?
        if (uart_is_writable(UART_ID)) {
            // Change it slightly first!
            //ch++;
            uart_putc(UART_ID, ch);
        }
        if (chars_rxed < 256) {
            data[chars_rxed] = ch;
        }
        chars_rxed++;
        if (chars_rxed >= 256) {
            int count = 0;
            while (count < 256) {
                uart_putc(UART_ID, data[count]);
                count++;
                printf("\n");
            }
            chars_rxed = 0;
        }
        }
        */
    }
}

void uart_full_init() {
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);
}

int main() {
    // Enable UART so we can print
    stdio_init_all();
    uart_full_init();
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

    while (1) {

    }

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
    sector_erase(spi_default, PICO_DEFAULT_SPI_CSN_PIN, read_address);
    uint8_t data_read_after_erase[4];
    read_data(spi_default, PICO_DEFAULT_SPI_CSN_PIN, data_read_after_erase, 4, read_address);

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
    printf("Read Data: ");
    printbuf(data_read_after_erase, 4);
    sleep_ms(10*1000);
    
    while(1) {
       tight_loop_contents(); 
    }
#endif
}