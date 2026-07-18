#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SYSY_EMBEDDED_INPUT
extern const unsigned char __sysy_input_start[];
extern const unsigned char __sysy_input_end[];
static const unsigned char *input_cursor = __sysy_input_start;

int getint(void) {
  char *end;
  long value = strtol((const char *)input_cursor, &end, 10);
  if (end == (const char *)input_cursor) return 0;
  input_cursor = (const unsigned char *)end;
  return (int)value;
}

int getch(void) {
  if (input_cursor == __sysy_input_end) return EOF;
  return *input_cursor++;
}

float getfloat(void) {
  char *end;
  float value = strtof((const char *)input_cursor, &end);
  if (end == (const char *)input_cursor) return 0.0f;
  input_cursor = (const unsigned char *)end;
  return value;
}
#else
int getint(void) {
  int value;
  return scanf("%d", &value) == 1 ? value : 0;
}

int getch(void) { return getchar(); }

float getfloat(void) {
  float value;
  return scanf("%a", &value) == 1 ? value : 0.0f;
}
#endif

int getarray(int values[]) {
  int count = getint();
  for (int i = 0; i < count; ++i) values[i] = getint();
  return count;
}

int getfarray(float values[]) {
  int count = getint();
  for (int i = 0; i < count; ++i) values[i] = getfloat();
  return count;
}

void putint(int value) { printf("%d", value); }
void putch(int value) { putchar(value); }
void putfloat(float value) { printf("%a", value); }

void putarray(int count, int values[]) {
  printf("%d:", count);
  for (int i = 0; i < count; ++i) printf(" %d", values[i]);
  putchar('\n');
}

void putfarray(int count, float values[]) {
  printf("%d:", count);
  for (int i = 0; i < count; ++i) printf(" %a", values[i]);
  putchar('\n');
}

void putf(char format[], ...) {
  va_list arguments;
  va_start(arguments, format);
  vprintf(format, arguments);
  va_end(arguments);
}

void _sysy_starttime(int line) { (void)line; }
void _sysy_stoptime(int line) { (void)line; }
