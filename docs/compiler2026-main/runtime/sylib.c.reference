#include<stdio.h>
#include<stdarg.h>
#include<stdint.h>
#include<inttypes.h>
#include<string.h>
#include<time.h>
#include"sylib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Timing function implementation */
static uint64_t _sysy_start[_SYSY_N], _sysy_end[_SYSY_N];
static int _sysy_l1[_SYSY_N],_sysy_l2[_SYSY_N];
static int _sysy_idx;

static uint64_t curtime(void) {
#if 0
	struct timespec mytime;
	clock_gettime(CLOCK_MONOTONIC, &mytime);
	return mytime.tv_sec * 1000000000ULL + mytime.tv_nsec;
#else
	uint64_t cycles;
	asm volatile ("rdcycle %0": "=r"(cycles));
	return cycles;
#endif
}

/* Input & output functions */
int getint(){int t; scanf("%d",&t); return t; }
int getch(){char c; scanf("%c",&c); return (int)c; }
float getfloat(){
    float n;
    scanf("%a", &n);
    return n;
}

int getarray(int a[]){
  int n;
  scanf("%d",&n);
  for(int i=0;i<n;i++)scanf("%d",&a[i]);
  return n;
}

int getfarray(float a[]) {
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        scanf("%a", &a[i]);
    }
    return n;
}
void putint(int a){ printf("%d",a);}
void putch(int a){ printf("%c",a); }
void putarray(int n,int a[]){
  printf("%d:",n);
  for(int i=0;i<n;i++)printf(" %d",a[i]);
  printf("\n");
}
void putfloat(float a) {
  printf("%a", a);
}
void putfarray(int n, float a[]) {
    printf("%d:", n);
    for (int i = 0; i < n; i++) {
        printf(" %a", a[i]);
    }
    printf("\n");
}

void putf(char a[], ...) {
    va_list args;
    va_start(args, a);
    vfprintf(stdout, a, args);
    va_end(args);
}

/* Timing function implementation */
void before_main(){
  memset(_sysy_start, sizeof(_sysy_start), 0);
  memset(_sysy_end, sizeof(_sysy_end), 0);
  _sysy_idx=1;
  printf("---Cut Start---\n");
}
extern int g_write_count;
extern char *g_output_base_ptr;
void after_main(uint64_t retcode){
  uint64_t total = 0;
  fflush(stdout);
  if (g_write_count == 0)
    printf("%ld\n", retcode%256);
  else if (g_output_base_ptr[g_write_count-1] != 0xa)
    printf("\n%ld\n", retcode%256);
  else
    printf("%ld\n", retcode%256);

  printf("---Cut Stop---\n");
  for(int i=1;i<_sysy_idx;i++) {
    uint64_t elapsed = _sysy_end[i] - _sysy_start[i];
    printf("Timer@%04d-%04d: %15" PRIu64 " \n", _sysy_l1[i], _sysy_l2[i], elapsed);
    total += elapsed;
  }
  printf("TOTAL: %15" PRIu64 "\n", total);
  printf("---Cut End---\n");
}  
void _sysy_starttime(int lineno){
  _sysy_l1[_sysy_idx] = lineno;
  _sysy_start[_sysy_idx] = curtime();
}
void _sysy_stoptime(int lineno){
  _sysy_end[_sysy_idx] = curtime();
  _sysy_l2[_sysy_idx] = lineno;
  _sysy_idx ++;
}

#ifdef __cplusplus
}
#endif
