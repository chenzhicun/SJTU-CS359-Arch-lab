/*
 *  Author:Zhicun Chen
 *  Student ID:518030910173
 */

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    
    if(M==32&&N==32){
        int i,j,k,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
        for(j=0;j<32;j+=8){
            for(i=0;i<32;i+=8){
                for(k=j;k<j+8;k++){
                    tmp0=A[k][i];
                    tmp1=A[k][i+1];
                    tmp2=A[k][i+2];
                    tmp3=A[k][i+3];
                    tmp4=A[k][i+4];
                    tmp5=A[k][i+5];
                    tmp6=A[k][i+6];
                    tmp7=A[k][i+7];
                    B[i][k]=tmp0;
                    B[i+1][k]=tmp1;
                    B[i+2][k]=tmp2;
                    B[i+3][k]=tmp3;
                    B[i+4][k]=tmp4;
                    B[i+5][k]=tmp5;
                    B[i+6][k]=tmp6;
                    B[i+7][k]=tmp7;
                }
            }
        }
    }
    
    if(M==64&&N==64){
        int i,j,k,tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
        for(j=0;j<64;j+=8){
            for(i=0;i<64;i+=8){
                //using B[i~i+3][k+4] as buffer
                for(k=j;k<j+4;k++){
                    tmp0=A[k][i];
                    tmp1=A[k][i+1];
                    tmp2=A[k][i+2];
                    tmp3=A[k][i+3];
                    tmp4=A[k][i+4];
                    tmp5=A[k][i+5];
                    tmp6=A[k][i+6];
                    tmp7=A[k][i+7];
                    B[i][k]=tmp0;
                    B[i+1][k]=tmp1;
                    B[i+2][k]=tmp2;
                    B[i+3][k]=tmp3;
                    B[i][k+4]=tmp4;
                    B[i+1][k+4]=tmp5;
                    B[i+2][k+4]=tmp6;
                    B[i+3][k+4]=tmp7;
                }
                //put the elements in the buffer into the right place, and
                //read some data from A, and put them into right place.
                for(k=i;k<i+4;k++){
                    tmp0=B[k][j+4];
                    tmp1=B[k][j+5];
                    tmp2=B[k][j+6];
                    tmp3=B[k][j+7];
                    tmp4=A[j+4][k];
                    tmp5=A[j+5][k];
                    tmp6=A[j+6][k];
                    tmp7=A[j+7][k];
                    B[k][j+4]=tmp4;
                    B[k][j+5]=tmp5;
                    B[k][j+6]=tmp6;
                    B[k][j+7]=tmp7;
                    B[k+4][j]=tmp0;
                    B[k+4][j+1]=tmp1;
                    B[k+4][j+2]=tmp2;
                    B[k+4][j+3]=tmp3;
                }
                //put the rest of A into the right place.
                //one loop operate 2 rows.
                for(k=j+4;k<j+8;k+=2){
                    tmp0=A[k][i+4];
                    tmp1=A[k][i+5];
                    tmp2=A[k][i+6];
                    tmp3=A[k][i+7];
                    tmp4=A[k+1][i+4];
                    tmp5=A[k+1][i+5];
                    tmp6=A[k+1][i+6];
                    tmp7=A[k+1][i+7];
                    B[i+4][k]=tmp0;
                    B[i+5][k]=tmp1;
                    B[i+6][k]=tmp2;
                    B[i+7][k]=tmp3;
                    B[i+4][k+1]=tmp4;
                    B[i+5][k+1]=tmp5;
                    B[i+6][k+1]=tmp6;
                    B[i+7][k+1]=tmp7;
                }
            }
        }
    }

    if(M==61&&N==67){
        int i,j,k,l,tmp;
        for(i=0;i<61;i+=8){
            for(j=0;j<67;j+=8){
                for(k=j;(k<j+8)&&(k<67);k++){
                    for(l=i;(l<i+8)&&(l<61);l++){
                        tmp=A[k][l];
                        B[l][k]=tmp;
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

