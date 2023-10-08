#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#define MAX_SIZE 4096
#define NUM_THREADS 4

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;           
int maxnum;       
char* Init;       
int PRINT;        
matrix A;         
matrix I = {0.0};

pthread_barrier_t barrier;
pthread_mutex_t mutex;

typedef struct {
    int thread_id;
} ThreadArgs;

void find_inverse_parallel(int start_row, int end_row)
{
    int row, col, p;
    double pivalue;

    for (p = 0; p < N; p++) {
        pthread_mutex_lock(&mutex);
        pivalue = A[p][p];
        for (col = 0; col < N; col++) {
            A[p][col] = A[p][col] / pivalue;
            I[p][col] = I[p][col] / pivalue;
        }
        pthread_mutex_unlock(&mutex);
        assert(A[p][p] == 1.0);

        double multiplier;
        for (row = start_row; row < end_row; row++) {
            multiplier = A[row][p];
            if (row != p) {
                pthread_mutex_lock(&mutex);
                for (col = 0; col < N; col++) {
                    A[row][col] = A[row][col] - A[p][col] * multiplier;
                    I[row][col] = I[row][col] - I[p][col] * multiplier;
                }
                pthread_mutex_unlock(&mutex);
                assert(A[row][p] == 0.0);
            }
        }
        pthread_barrier_wait(&barrier);
    }
}

void* ThreadWork(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    int thread_id = thread_args->thread_id;

    int rows_per_thread = N / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id == NUM_THREADS - 1) ? N : (start_row + rows_per_thread);

    find_inverse_parallel(start_row, end_row);

    return NULL;
}

void Init_Matrix()
{
    int row, col;

    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (row == col)
                I[row][col] = 1.0;
        }
    }

    if (strcmp(Init, "rand") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) 
                    A[row][col] = (double)(rand() % maxnum) + 5.0;
                else
                    A[row][col] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) 
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }

}

void Write_Matrix(matrix M, FILE* file) {
    int row, col;
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            fprintf(file, "%5.2f ", M[row][col]);
        }
        fprintf(file, "\n");
    }
}

void Init_Default()
{
    N = 5;
    Init = "fast";
    maxnum = 15.0;
    PRINT = 1;
}

int Read_Options(int argc, char** argv)
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
    return 0;
}

int main(int argc, char** argv)
{

    Init_Default();
    Read_Options(argc, argv);
    Init_Matrix();
    
    FILE* outputFile = fopen("output.txt", "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        return 1;
    }
    fprintf(outputFile, "Matrix Inverse...\n");
    fprintf(outputFile, "size      = %dx%d\n", N, N);
    fprintf(outputFile, "maxnum    = %d\n", maxnum);
    fprintf(outputFile, "Init      = %s\n", Init);
    fprintf(outputFile, "Initializing matrix...done\n");

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, ThreadWork, &thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
	
    Write_Matrix(I, outputFile);

    fclose(outputFile);

    return 0;
}

