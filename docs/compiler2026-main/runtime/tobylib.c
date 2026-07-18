#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "tobylib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SR_RX_FIFO_VALID_DATA   (1 << 0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL         (1 << 1) /* receive FIFO full */
#define SR_TX_FIFO_EMPTY        (1 << 2) /* transmit FIFO empty */
#define SR_TX_FIFO_FULL         (1 << 3) /* transmit FIFO full */

struct uart_regs {
    volatile uint32_t rx_fifo;
    volatile uint32_t tx_fifo;
    volatile uint32_t status;
    volatile uint32_t control;
};

int putchar(int ch) {
    struct uart_regs * regs = (struct uart_regs *)0x60010000;
    while (regs->status & SR_TX_FIFO_FULL) {}
    regs->tx_fifo = ch & 0xff;
    return ch & 0xff;
}

int puts(const char * s) {
    while (*s) putchar(*s++);
    putchar('\r');
    putchar('\n');
    return 0;
}

#ifdef __cplusplus
}
#endif
