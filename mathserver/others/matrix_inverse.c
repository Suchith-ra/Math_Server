#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#define MAX_SIZE 4096
#define NUM_THREADS 4 // You can adjust the number of threads as needed

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;         /* matrix size        */
int maxnum;    /* max number of element */
char* Init;    /* matrix init type    */
int PRINT;     /* print switch        */
matrix A;      /* matrix A            */
matrix I = {0.0};  /* The A inverse matrix, which will be initialized to the identity matrix */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronization
int current_row = 0; // Global variable to keep track of the current row being processed

/* Forward declarations */
void find_inverse(void);
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);
void Init_Default(void);
int Read_Options(int, char**);
void* thread_inverse(void* arg);

int main(int argc, char** argv)
{
    printf("Matrix Inverse\n");
    int i, timestart, timeend, iter;

    Init_Default();     /* Init default values */
    Read_Options(argc, argv);   /* Read arguments */
    Init_Matrix();      /* Init the matrix */

    pthread_t threads[NUM_THREADS];
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_inverse, NULL);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    if (PRINT == 1)
    {
        //Print_Matrix(A, "End: Input");
        Print_Matrix(I, "Inversed");
    }
}

void find_inverse()
{
    int row, col, p; // 'p' stands for pivot (numbered from 0 to N-1)
    double pivalue; // pivot value

    while (1) {
        pthread_mutex_lock(&mutex);
        int row_to_process = current_row++;
        pthread_mutex_unlock(&mutex);

        if (row_to_process >= N) {
            break; // All rows have been processed
        }

        p = row_to_process;

        /* Bringing the matrix A to the identity form */
        pivalue = A[p][p];
        for (col = 0; col < N; col++)
        {
            A[p][col] = A[p][col] / pivalue; /* Division step on A */
            I[p][col] = I[p][col] / pivalue; /* Division step on I */
        }
        assert(A[p][p] == 1.0);

        double multiplier;
        for (row = 0; row < N; row++) {
            multiplier = A[row][p];
            if (row != p) // Perform elimination on all except the current pivot row
            {
                for (col = 0; col < N; col++)
                {
                    A[row][col] = A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
                    I[row][col] = I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
                }
                assert(A[row][p] == 0.0);
            }
        }
    }
}

void* thread_inverse(void* arg)
{
    find_inverse();
    return NULL;
}

void
Init_Matrix()
{
    int row, col;

    // Set the diagonal elements of the inverse matrix to 1.0
    // So that you get an identity matrix to begin with
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (row == col)
                I[row][col] = 1.0;
        }
    }

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init, "rand") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = (double)(rand() % maxnum) + 5.0;
                else
                    A[row][col] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }

    printf("done \n\n");
    if (PRINT == 1)
    {
        //Print_Matrix(A, "Begin: Input");
        //Print_Matrix(I, "Begin: Inverse");
    }
}

void
Print_Matrix(matrix M, char name[])
{
    int row, col;

    printf("%s Matrix:\n", name);
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++)
            printf(" %5.2f", M[row][col]);
        printf("\n");
    }
    printf("\n\n");
}

void
Init_Default()
{
    N = 5;
    Init = "fast";
    maxnum = 15.0;
    PRINT = 1;
}

int
Read_Options(int argc, char** argv)
{
    char* prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++ * argv) {
            case 'n':
                --argc;
                N = atoi(*++argv);
                break;
            case 'h':
                printf("\nHELP: try matinv -u \n\n");
                exit(0);
                break;
            case 'u':
                printf("\nUsage: matinv [-n problemsize]\n");
                printf("           [-D] show default values \n");
                printf("           [-h] help \n");
                printf("           [-I init_type] fast/rand \n");
                printf("           [-m maxnum] max random no \n");
                printf("           [-P print_switch] 0/1 \n");
                exit(0);
                break;
            case 'D':
                printf("\nDefault:  n         = %d ", N);
                printf("\n          Init      = rand");
                printf("\n          maxnum    = 5 ");
                printf("\n          P         = 0 \n\n");
                exit(0);
                break;
            case 'I':
                --argc;
                Init = *++argv;
                break;
            case 'm':
                --argc;
                maxnum = atoi(*++argv);
                break;
            case 'P':
                --argc;
                PRINT = atoi(*++argv);
                break;
            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -u \n\n", prog);
                break;
            }
}

