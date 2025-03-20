#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>

/* 
  Rows and cols must be in 1:4 ratio, or we'll get 'illegal instruction' error.
  Stride marks the end of a column and a start of a new row. 
  You can set stride to be x2 cols if you want to skip alternate rows, for example.a64l
  The maximum number of rows and columns is 16 and 64 respectively.
  The maximum number of elements in a tile is 1024, but we are using 1:4 ratio.
  Don't change the other definitions.
*/
#define MAX_ROWS 1     // maximum 16         
#define MAX_COLS 4     // maximum 64         
#define MAX MAX_ROWS * MAX_COLS     //maximum 1024          
#define STRIDE MAX_COLS              
#define ARCH_GET_XCOMP_PERM     0x1022
#define ARCH_REQ_XCOMP_PERM     0x1023
#define XFEATURE_XTILECFG       17
#define XFEATURE_XTILEDATA      18

/*
  Define tile config data structure 
  DO NOT CHANGE THIS AT ALL!!
*/
typedef struct __tile_config
{
  uint8_t palette_id;
  uint8_t start_row;
  uint8_t reserved_0[14];
  uint16_t colsb[16]; 
  uint8_t rows[16]; 
} __tilecfg;

/* 
  Initialize tile config 
*/
static void init_tile_config (__tilecfg *tileinfo)
{
  tileinfo->palette_id = 1;
  tileinfo->start_row = 0;

  // tile for res
  tileinfo->colsb[0] = MAX_ROWS;
  tileinfo->rows[0] =  MAX_ROWS;
  // tile for src1
  tileinfo->colsb[1] = MAX_COLS;
  tileinfo->rows[1] =  MAX_ROWS;
  // tile for src2
  tileinfo->colsb[2] = MAX_COLS;
  tileinfo->rows[2] =  MAX_ROWS;
  // tile for intermediate results //
  tileinfo->colsb[3] = MAX_COLS;
  tileinfo->rows[3] =  MAX_ROWS;

  _tile_loadconfig (tileinfo);
}
/* 
  Invoke syscall to set ARCH_SET_STATE_USE 
*/
static bool set_tiledata_use()
{
   if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)) 
   {
      printf("\n Fail to do XFEATURE_XTILEDATA \n\n");
      return false;
   }
   else
   {
      printf("\n TILE DATA USE SET - OK \n\n");
      return true;
   }

   return true;
}

/* 
  Initialize src buffer 
*/
static void init_src (int8_t *buf, int8_t value)
{
  for (int i = 0; i < MAX_ROWS; i++)
    for (int j = 0; j < MAX_COLS; j++)
    {
        buf[i * MAX_COLS + j] = value;
    }
}
/* 
  Print src buffer 
*/
static void print_src(int8_t* buf) 
{
   for (int i = 0; i < MAX_ROWS; i++) {
     for (int j = 0; j < (MAX_COLS); j++)
     {
         printf("%d ", buf[i * MAX_COLS + j]);
     }
     printf("\n");
   }
   printf("\n");
}

/* 
  Initialize res buffer 
*/
static void init_res (int32_t *buf, int32_t value)
{
  int col = MAX_ROWS;
  int rows = MAX_ROWS;

  for (int i = 0; i < rows; i++)
    for (int j = 0; j < (col); j++) //MAX_COLS/4
    {
        buf[i * col + j] = value;
    }
}
/* 
  Print res buffer 
*/
static void print_res(int32_t* buf)
{
  int col = MAX_COLS / 4;
  int rows = MAX_ROWS;

  for (int i = 0; i < rows; i++) {
     for (int j = 0; j < (col); j++)
     {
         printf("%d ", buf[i * col + j]);
     }
     printf("\n");
   }
  printf("\n");
}

int main(){
  // Request permission to linux kernel to run AMX 
  if (!set_tiledata_use())
    exit(-1);

  // Load tile configuration 
  __tilecfg tile_data = {0};
  init_tile_config (&tile_data);

  // Init dst matrix buffers with data
  int32_t res[MAX/4];        //MAX/4
  init_res (res, 0);
  print_res(res);
  // Init src matrix buffers with data
  int8_t src1[MAX];
  init_src (src1, 2);
  print_src(src1);
 
  int8_t src2[MAX];
  init_src (src2, 2);
  print_src(src2);

  // Load tile rows from memory
  _tile_loadd (1, res, STRIDE);
  _tile_loadd (2, src1, STRIDE);
  _tile_loadd (3, src2, STRIDE);
  
  // Compute & store dot-product of src1 & src2 in res
  _tile_dpbssd (1, 2, 3);
  _tile_stored (1, res, STRIDE);

  //Print the result
  printf("after operation 1:\n\n");
  print_res(res);

  // repeated operations are possible too
  init_res (res, 10);
  print_res(res);

  int8_t src3[MAX];
  init_src (src3, 1);
  print_src(src3);

  int8_t src4[MAX];
  init_src (src4, 1);
  print_src(src4);

  _tile_loadd (1, res, STRIDE);
  _tile_loadd (2, src3, STRIDE);
  _tile_loadd (3, src4, STRIDE);
  _tile_dpbssd (1, 2, 3);
  _tile_stored (1, res, STRIDE);

  printf("after operation 2:\n\n");
  print_res(res);

  // Release the tile configuration to return to the init state, 
  // which releases all storage it currently holds
  _tile_release ();
}

/*
  Run this file like this :
  gcc -O2 -march=native -fno-strict-aliasing main.c -o test && ./test
*/
