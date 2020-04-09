#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAP_FILENAME  "./Data/54/test_data.txt"
#define RESULT_FILENAME  "my_result.txt"
#define MAXV 1000000
#define MAX_PATH_LENGTH 7
#define THREAD_NUMBER 100

typedef struct ArcNode {
    int adj_vex;    // out bound of vec
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

typedef struct thread_msg {
    AdjGraph *G;
    int v;
    int start;
    int path_length;
    int *my_path_length;
    path_info *my_path;
    int *temp_path;
    int *visited;
} thread_msg;

void *DFS(void *msg);

AdjGraph *creatAdj(const char *filename);

void write_path(const char *filename, mixed_path_result result);

void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest);

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


    AdjGraph *G;
    // G = (AdjGraph *) malloc(sizeof(AdjGraph));   // useless `malloc`
    G = creatAdj(MAP_FILENAME);

    mixed_path_result all_ring = {
            0,
            (path_info *) malloc(sizeof(path_info) * MAXV)
    };
    pthread_t thread_list[MAXV];
    mixed_path_result *ring_of_current_node;
    for (int i = 0; i <= G->n; i++) {
        //printf("Searching node %d\n", i);
        thread_msg *current_msg = (thread_msg *) malloc(sizeof(thread_msg));
        current_msg->G = G;
        current_msg->v = i;
        current_msg->start = i;
        current_msg->path_length = 0;
        current_msg->my_path_length = NULL;
        current_msg->my_path = NULL;
        current_msg->temp_path = NULL;
        current_msg->visited = NULL;

        pthread_create(thread_list + i, NULL, DFS, current_msg);

        if (i >= THREAD_NUMBER && i % THREAD_NUMBER == 0) {
            for (int ii = 0; ii < THREAD_NUMBER; ii++) {
                void *ring_of_current_node_row;
                pthread_join(thread_list[i - THREAD_NUMBER + ii], &ring_of_current_node_row);
                ring_of_current_node = (mixed_path_result *) (ring_of_current_node_row);
                merge_mixed_path_result(ring_of_current_node, &all_ring);
            }
        }
    }

    write_path(RESULT_FILENAME, all_ring);

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

void *DFS(void *msg_row) {
    thread_msg *msg = msg_row;

    AdjGraph *G = msg->G;

    // TODO: free all these located memory, especially `my_path`
    if (msg->path_length == 0) {
        // store all the result
        msg->my_path_length = (int *) malloc(sizeof(int));
        *(msg->my_path_length) = 0;
        msg->my_path = (path_info *) malloc(sizeof(path_info) * MAXV);

        msg->temp_path = (int *) calloc(MAX_PATH_LENGTH, sizeof(int));
        msg->visited = (int *) calloc(MAXV, sizeof(int));
    }

    int v = msg->v;
    const int start = msg->start;
    int path_length = msg->path_length;
    int *my_path_length = msg->my_path_length;
    path_info *my_path = msg->my_path;
    int *temp_path = msg->temp_path;
    int *visited = msg->visited;

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

            printf("acc\n");
        }

        // TODO: could change temp_path[0] -> start ?
        if (visited[w] == 0 && w > temp_path[0] && path_length < 7) {
            msg->v = w;
            msg->path_length = path_length;
            DFS(msg);    // recursion
        }
        p = p->next_arc;      // next
    }

    // exit recursion
    visited[v] = 0; // clear mark
    temp_path[path_length] = 0;  // clear path

    path_length--;   // decrease path length

    // generate the return info
    if (path_length == 0) {
        // TODO: free the memory located for mixed_path_result
        mixed_path_result *temp = (mixed_path_result *) malloc(sizeof(mixed_path_result));
        temp->num_of_path = *my_path_length;
        temp->path_list = my_path;

        // free memory
        free(visited);
        free(temp_path);
        free(my_path_length);

        free(msg_row);

        return temp;
    } else
        return NULL;
}

void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest) {
    int base = dest->num_of_path;
    memcpy(dest->path_list + base, src->path_list, src->num_of_path * sizeof(path_info));
    dest->num_of_path += src->num_of_path;
    free(src->path_list);
    free(src);
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