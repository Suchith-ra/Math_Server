#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>

#define MAX_POINTS 4096
#define MAX_CLUSTERS 32
#define NUM_THREADS 4 // Number of threads to use (adjust as needed)

typedef struct point
{
    float x;
    float y;
    int cluster;
} point;

int N;
int k = 9;
point data[MAX_POINTS];
point cluster[MAX_CLUSTERS];

pthread_mutex_t mutex;

int get_closest_centroid(int i, int k)
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++) {
        // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        if (dist <= min_dist) {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    return nearest_cluster;
}

void update_cluster_centers()
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = {0}; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = {0.0};

    for (int i = 0; i < N; i++) {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < k; i++) {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
}

void read_data(int argc, char* argv[])
{
    N = 1797;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[i + 1]);
            break;
        }
    }
    
    FILE* fp = fopen("kmeans-data.txt", "r");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }

    // Initialize points from the data file
    float temp;
    for (int i = 0; i < N; i++) {
        fscanf(fp, "%f %f", &data[i].x, &data[i].y);
        data[i].cluster = -1; // Initialize the cluster number to -1
    }
    printf("Read the problem data!\n");

    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++) {
        int r = rand() % N;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}


void* assign_clusters_and_update_centers(void* thread_id)
{
    int id = (int)(long)thread_id;

    int start = id * (N / NUM_THREADS);
    int end = (id == (NUM_THREADS - 1)) ? N : start + (N / NUM_THREADS);

    bool something_changed;
    int old_cluster, new_cluster;

    do {
        something_changed = false;
        for (int i = start; i < end; i++) {
            old_cluster = data[i].cluster;
            new_cluster = get_closest_centroid(i, k);
            data[i].cluster = new_cluster;
            if (old_cluster != new_cluster) {
                something_changed = true;
            }
        }
        pthread_mutex_lock(&mutex);
        if (something_changed) {
            update_cluster_centers();
        }
        pthread_mutex_unlock(&mutex);
    } while (something_changed);

    pthread_exit(NULL);
}

bool assign_clusters_to_points()
{
    bool something_changed = false;
    int old_cluster = -1, new_cluster = -1;
    for (int i = 0; i < N; i++)
    { // For each data point
        old_cluster = data[i].cluster;
        new_cluster = get_closest_centroid(i, k);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed = true;
        }
    }
    return something_changed;
}

int kmeans(int k)
{
    bool somechange;
    int iter = 0;
    do {
        iter++; // Keep track of the number of iterations
        somechange = assign_clusters_to_points();
        update_cluster_centers();
    } while (somechange);
    printf("Number of iterations taken = %d\n", iter);
    printf("Computed cluster numbers successfully!\n");
}

void write_results()
{
    FILE* fp = fopen("kmeans-results.txt", "w");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < N; i++) {
            fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
        }
    }
    printf("Wrote the results to a file!\n");
}


int main(int argc, char* argv[])
{
    read_data(argc, argv);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, assign_clusters_and_update_centers, (void*)(long)i);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    kmeans(k);
    write_results();
    
    pthread_mutex_destroy(&mutex);
    
    return 0;
    
}
