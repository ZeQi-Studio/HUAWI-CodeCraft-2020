#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zconf.h>
#include <sys/time.h>

#define MAP_FILENAME  "./Data/280000/test_data.txt"
#define RESULT_FILENAME  "my_result.txt"
#define MAXV 1000000
#define MAX_PATH_LENGTH 7
#define THREAD_NUMBER 32
#define SLICE ((int)(G->n / THREAD_NUMBER) + 1)

typedef struct ArcNode {
    unsigned int adj_vex;    // out bound of vec
    struct ArcNode *next_arc;  // next vec
}/*__attribute__((packed))*/ ArcNode;

typedef struct Vnode {
    int data;               // unused node info
    ArcNode *first_arc;      // head of list
} VNode;

// graph
typedef struct AdjGraph {
    int n;      // count node number
    int e;      // count edge number
    VNode adj_list[MAXV];    // node list
} AdjGraph;

// path_info
typedef struct path_info {
    int num;
    int path[MAX_PATH_LENGTH];
} path_info;

// path info + length
typedef struct mixed_path_result {
    int num_of_path;
    path_info *path_list;
} mixed_path_result;

typedef struct thread_info {
    AdjGraph *G;
    int start;
    int end;
} thread_info;

int min(int a, int b) {
    return a < b ? a : b;
}

mixed_path_result *DFS(AdjGraph *G, int v, int start, \
        int path_length, int *my_path_length, path_info my_path[], int *temp_path, int visited[]);

AdjGraph *creatAdj(const char *filename);

void write_path(const char *filename, mixed_path_result result);

void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest);


void *multi_thread_dfs(void *msg_raw) {
    thread_info *msg = (struct thread_info *) msg_raw;

    //mixed_path_result all_ring;
    //all_ring.num_of_path = 0;
    //all_ring.path_list = (path_info *) malloc(sizeof(path_info) * MAXV);
    int visited[MAXV] = {0};
    int *temp_path = (int *) malloc((MAX_PATH_LENGTH + 100) * sizeof(int));
    int my_path_length = 0;
    path_info *my_path = (path_info *) malloc(sizeof(path_info) * MAXV);
    mixed_path_result *return_temp = (mixed_path_result *) malloc(sizeof(mixed_path_result));
    if (my_path == NULL) {
        printf("Allocate memory failed.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = msg->start; i < msg->end; i++) {
        //printf("Searching node %d\n", i);
//        mixed_path_result *ring_of_current_node =
        DFS(msg->G, i, i,
            0, &my_path_length, my_path, temp_path, visited);
        //merge_mixed_path_result(ring_of_current_node, &all_ring);
    }


//    if (return_temp == NULL) {
//        printf("Allocate memory failed.\n");
//        exit(EXIT_FAILURE);
//    }
    return_temp->num_of_path = my_path_length;
    return_temp->path_list = my_path;
    free(msg_raw);
    return return_temp;
}

int main(void) {
    // memory test area
#define MEMORY_TEST_OFF // switch
#ifdef MEMORY_TEST_ON
#define ARC_NUMBER  280000
    printf("size of ArcNode: %zu\n B", sizeof(ArcNode));
    printf("take memory: %f MB\n", (float) sizeof(ArcNode) * ARC_NUMBER / 1024 / 1024);

    ArcNode *start_pointer;
    ArcNode *p = start_pointer = (ArcNode *) malloc(sizeof(ArcNode));
    for (int i = 0; i < 280000; i++) {
        p->next_arc = (ArcNode *) malloc(sizeof(ArcNode));
        p->adj_vex = i;
        p = p->next_arc;
    }
    for (int i = 0; i < 280000; i++) {
        //printf("%d",start_pointer->adj_vex);
        start_pointer = start_pointer->next_arc;
    }
    printf("Memory test done.\n");
    //exit(EXIT_SUCCESS);
#endif

    // timer start
    struct timeval start_time, end_time;
    gettimeofday(&start_time, 0);

    AdjGraph *G;
    // G = (AdjGraph *) malloc(sizeof(AdjGraph));   // useless `malloc`
    G = creatAdj(MAP_FILENAME);
    pthread_t thread_list[THREAD_NUMBER];

    for (int i = 0; i < THREAD_NUMBER; i++) {
        thread_info *current_msg = (thread_info *) malloc(sizeof(thread_info));
        current_msg->start = i * SLICE;
        current_msg->end = min(G->n, (i + 1) * SLICE);
        current_msg->G = G;

        // preview the start and end of slice
        printf("%d %d\n", current_msg->start, current_msg->end);

        pthread_create(thread_list + i, NULL, multi_thread_dfs, current_msg);
    }

    mixed_path_result all_ring;
    all_ring.num_of_path = 0;
    all_ring.path_list = (path_info *) malloc(sizeof(path_info) * MAXV);
    for (int i = 0; i < THREAD_NUMBER; i++) {
        void *return_raw;
        pthread_join(thread_list[i], &return_raw);
        mixed_path_result *ring_of_current_node = (mixed_path_result *) return_raw;
        merge_mixed_path_result(ring_of_current_node, &all_ring);
    }


    write_path(RESULT_FILENAME, all_ring);

    gettimeofday(&end_time, 0);
    double time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);

    return 0;
}


AdjGraph *creatAdj(const char *filename) {
    FILE *map_file = fopen(filename, "r");  // data file
    AdjGraph *G = (AdjGraph *) malloc(sizeof(AdjGraph));    // head pointer of graph
    ArcNode *p;     // temp pointer for arc

    int i, j, k;

    // init graph info
    G->e = 0;
    G->n = 0;
    for (int a = 0; a < MAXV; a++) {
        G->adj_list[a].first_arc = NULL;
    }

    while (EOF != fscanf(map_file, "%d,%d,%d\n", &i, &j, &k)) {

        p = (ArcNode *) malloc(sizeof(ArcNode));
        p->adj_vex = j;
        // insert to list
        p->next_arc = G->adj_list[i].first_arc;
        G->adj_list[i].first_arc = p;
        // update graph info
        G->e++;
        if (i > G->n) {
            G->n = i;
        }
    }
    return G;
}

mixed_path_result *DFS(AdjGraph *G, int v, const int start, \
        int path_length, int *my_path_length, path_info my_path[], int *temp_path, int visited[]) {


    ArcNode *p; // floating pointer
    int w;  // temp to store the current node index

    visited[v] = 1;        // mark visited
    p = G->adj_list[v].first_arc;        // pointer to the edge link list

    temp_path[path_length] = v;  // write current node to the path
    path_length++;   // increase path length

    while (p != NULL) {
        w = p->adj_vex; // outbound index number

        // find a ring
        if (w == start && path_length > 2) {
            // copy path to the output: dest, src
            memcpy(my_path[*my_path_length].path, temp_path, sizeof(int) * path_length);
            my_path[*my_path_length].num = path_length;    // set the path length
            (*my_path_length)++;    // mark the total number of path

            //printf(".");
        }

        // TODO: could change temp_path[0] -> start ?
        if (visited[w] == 0 && w > temp_path[0] && path_length < 7)
            DFS(G, w, start,
                path_length, my_path_length, my_path, temp_path, visited);    // recursion

        p = p->next_arc;      // next
    }

    // exit recursion
    visited[v] = 0; // clear mark
    temp_path[path_length] = 0;  // clear path

    path_length--;   // decrease path length

    // generate the return info
//    if (path_length == 0) {
//        // TODO: free the memory located for mixed_path_result
//        mixed_path_result *temp = (mixed_path_result *) malloc(sizeof(mixed_path_result));
//        temp->num_of_path = *my_path_length;
//        temp->path_list = my_path;
//
//        return temp;
//    } else
    return NULL;
}


void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest) {
    int base = dest->num_of_path;
    for (int i = 0; i < src->num_of_path; i++) {
        memcpy(dest->path_list + base + i, src->path_list + i, sizeof(path_info));
    }
    free(src->path_list);
    //free(src);
    dest->num_of_path += src->num_of_path;
}

void write_path(const char *filename, mixed_path_result result) {
    int my_path_lenth = result.num_of_path;
    path_info *my_path = result.path_list;

    FILE *path_file;
    path_file = fopen(filename, "w");
    fprintf(path_file, "%d\n", my_path_lenth);
    int i, j, k;
    for (k = 3; k <= MAX_PATH_LENGTH; k++) {
        for (i = 0; i < my_path_lenth; i++) {
            if (my_path[i].num == k) {
                for (j = 0; j < my_path[i].num; j++) {
                    if (j == 0) {
                        fprintf(path_file, "%d", my_path[i].path[j]);
                    } else {
                        fprintf(path_file, ",%d", my_path[i].path[j]);
                    }
                }
                fprintf(path_file, "\n");
            }
        }
    }
}