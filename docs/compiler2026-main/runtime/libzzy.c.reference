#include <stdarg.h>
#include <stdint.h>
#include <reent.h>

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

void delay(volatile unsigned int count) {
    while (count--) {
        __asm__ volatile("nop");
    }
}

uint64_t totalwrite = 0;
static void kputc(char ch) {
    struct uart_regs * regs = (struct uart_regs *)0x60010000;
    while (regs->status & SR_TX_FIFO_FULL) {}
    regs->tx_fifo = ch & 0xff;
    totalwrite++;
    //delay(100000);
}

static void kputs(const char * s) {
    while (*s) kputc(*s++);
    kputc('\r');
    kputc('\n');
}

void kprintf(const char * fmt, ...) {
    va_list vl;
    int is_format = 0;
    int is_long = 0;
    int is_char = 0;
    char c;

    va_start(vl, fmt);
    while ((c = *fmt++) != '\0') {
        if (is_format) {
            switch (c) {
            case 'l':
                is_long = 1;
                continue;
            case 'h':
                is_char = 1;
                continue;
            case 'x': {
                unsigned long n;
                long i;
                if (is_long) {
                    n = va_arg(vl, unsigned long);
                    i = (sizeof(unsigned long) << 3) - 4;
                }
                else {
                    n = va_arg(vl, unsigned int);
                    i = is_char ? 4 : (sizeof(unsigned int) << 3) - 4;
                }
                for (; i >= 0; i -= 4) {
                    long d;
                    d = (n >> i) & 0xF;
                    kputc(d < 10 ? '0' + d : 'a' + d - 10);
                }
                break;
            }
            case 'd': {
                char buf[32];
                long n;
                long i = sizeof(buf);
                if (is_long) {
                    n = va_arg(vl, long);
                }
                else {
                    n = va_arg(vl, int);
                }
                if (n < 0) {
                    kputc('-');
                    n = -n;
                }
                while (i > 0) {
                    buf[--i] = n % 10 + '0';
                    n = n / 10;
                    if (n == 0) break;
                }
                while (i < sizeof(buf)) kputc(buf[i++]);
                break;
            }
            case 's': {
                const char * s = va_arg(vl, const char *);
                while (*s) kputc(*s++);
                break;
            }
            case 'c':
                kputc(va_arg(vl, int));
                break;
            }
            is_format = 0;
            is_long = 0;
            is_char = 0;
        }
        else if (c == '%') {
            is_format = 1;
        }
        else if (c == '\n') {
            kputc('\r');
            kputc('\n');
        }
        else {
            kputc(c);
        }
    }
    va_end(vl);
}


static inline void w_mtvec(void * x)
{
 	asm volatile("csrw mtvec, %0" : : "r" (x));
}

#define STACK_SIZE 2*1024*1024
#define MCAUSE_MASK_INTERRUPT   0x80000000
#define MCAUSE_MASK_ECODE       0x7FFFFFFF

char __attribute__((aligned(16))) task_stack[STACK_SIZE];

unsigned long trap_handler(unsigned long epc, unsigned long cause, unsigned long oa0)
{
	unsigned long return_pc = epc;
        unsigned long cause_code = cause & MCAUSE_MASK_ECODE;

        if (cause & MCAUSE_MASK_INTERRUPT) {
                /* Asynchronous trap - interrupt */
                switch (cause_code) {
		case 3:
		  kprintf("software interruption! %lx\n", return_pc);
		  break;
                case 7:
		  kprintf("timer interruption!\n");
		  break;
                case 11:
		  kprintf("external interruption! return pc %lx\n", return_pc);
		  break;
                default:
		  kprintf("exception! Code = %lx return pc %lx\n", cause_code, return_pc);
		  break;
		}
	} else {
          /* Synchronous trap - exception */
	  int i = 1;
	  uint64_t * saved_registers = 	(uint64_t *)task_stack;  
	  kprintf("Sync exceptions! Code = %lx return pc %lx\n", cause_code, return_pc);
	  kprintf("saved registers:\n");
	  for (i = 0; i <= 29; i++) {
	    if (i == 29)
	      kprintf("%d: %lx\n", i, saved_registers[i]);
	    else
	      kprintf("%d: %lx, ", i, saved_registers[i]);
	  }
	  return_pc += 4;
	}
	kprintf("Current A0: %lx\n", oa0);
        return return_pc;
}



extern void trap_vector(void);

static inline void w_mscratch(void *x)
{
        asm volatile("csrw mscratch, %0" : : "r" (x));
}

int init_zzy() {
  unsigned int flags;
  w_mscratch(task_stack);
  kputs("ZZY begin Let's study interrupt\n");
  w_mtvec((void *)trap_vector);
  kputs("ZZY handler installed\n");
  asm volatile ("frflags %0" : "=r" (flags));
  kputs("return from trap\n");

  return 0;
}

int _exit(int sig) {
  kputs("Successful ended\n");
  for(;;);
  return 0;
}

extern int putchar(int ch);

#define outbyte(x) putchar(x)
#define inbyte() 0

#include "glue.h"
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef DEBUG_API
#define kputs(x)
#endif

/*
 * close -- We don't need to do anything, but pretend we did.
 */
int
_close  (int fd)
{
  kputs("_close");
  return (0);
}

/*
 * fstat -- Since we have no file system, we just return an error.
 */
int
_fstat (int fd,
       struct stat *buf)
{
  kputs("_fstat");
  buf->st_mode = S_IFCHR;	/* Always pretend to be a tty */
  buf->st_blksize = 0;

  return (0);
}

/*
 * getpid -- only one process, so just return 1.
 */
int
_getpid (void)
{
  kputs("_getpid");
  return __MYPID;
}


/*
 * isatty -- returns 1 if connected to a terminal device,
 *           returns 0 if not. Since we're hooked up to a
 *           serial port, we'll say yes, return a 1.
 */
int
_isatty (int fd)
{
  kputs("_isatty");
  return (1);
}

/*
 * kill -- go out via exit...
 */
int
_kill (int pid,
        int sig)
{
  kputs("_kill");
  if(pid == __MYPID)
    _exit(sig);
  return 0;
}

/*
 * lseek --  Since a serial port is non-seekable, we return an error.
 */
off_t
_lseek (int fd,
       off_t offset,
       int whence)
{
  kputs("_lseek");
  errno = ESPIPE;
  return ((off_t)-1);
}

/*
 * open -- open a file descriptor. We don't have a filesystem, so
 *         we return an error.
 */
int
_open (const char *buf,
       int flags,
       int mode)
{
  kputs("_open");
  errno = EIO;
  return (-1);
}

char *heap_ptr;

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
char *
_sbrk (nbytes)
     int nbytes;
{
  kputs("_sbrk");
  char        *base;

  if (!heap_ptr)
    heap_ptr = (char *)&_end;
  base = heap_ptr;
  heap_ptr += nbytes;
#ifdef DEBUG_API
  kprintf("Alloced %d bytes at %lx\n", nbytes, base);
#endif 
  return base;
}

/*
 * stat -- Since we have no file system, we just return an error.
 */
int
_stat (const char *path,
       struct stat *buf)
{
  kputs("_stat");
  errno = EIO;
  return (-1);
}

/*
 * unlink -- since we have no file system, 
 *           we just return an error.
 */
int
_unlink (char * path)
{
  kputs("_unlink");
  return (-1);
}

/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
char *g_output_base_ptr = (char *)0x80000000ULL;
volatile int g_write_count = 0ULL;

int
_write (int fd,
       char *buf,
       int nbytes)
{
  kputs("_write");
  int i;

  for (i = 0; i < nbytes; i++) {
    outbyte (buf[i]);
    g_output_base_ptr[g_write_count++] = buf[i];
  }
  return (nbytes);
}

char *g_input_base_ptr = (char*)0xC0000000ULL;
volatile int g_read_count = 0ULL;

int _read(int fd, char* buf, int nbytes) {
  int i = 0;
  for (i = 0; i < nbytes; i++) {
    if (g_input_base_ptr[g_read_count] == 0)
      break;
    buf[i] = g_input_base_ptr[g_read_count++];
  }
  return i?i:-1;
}

