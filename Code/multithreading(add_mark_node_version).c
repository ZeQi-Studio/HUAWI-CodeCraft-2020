#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//#define MAP_FILENAME  "/data/test_data.txt"
//#define RESULT_FILENAME  "/projects/student/result.txt"
#define MAP_FILENAME  "./Data/58284/test_data.txt"
#define RESULT_FILENAME  "./my_result.txt"

#define MAX_RING_NUMBER 3000000
#define MAX_NODE_NUMBER 560000
#define MAX_PATH_LENGTH 7

// multi-thread
#define THREAD_NUMBER 7

// hash table
#define MAX_HASH_LENGTH 560000
#define HASH_NUMBER 559970


int node_number = 0;
pthread_mutex_t count_lock, dec_lock;

#define MEMORY_TEST_OFF // switch
#define DEBUG_OUTPUT_OFF
#define TIMER_ON
#define TIMER_MIDDLE_ON
#ifdef TIMER_ON

#include <sys/time.h>

#endif

typedef struct ArcNode {
    int adj_vex;    // out bound of vec
    struct ArcNode *next_arc;  // next vec
} ArcNode;

typedef struct Vnode {
    ArcNode *first_out;      // head of out list
    ArcNode *first_in; //head of in list
    int in_degree, out_degree;
    unsigned int ID;               // unused node info
} VNode;

// graph
typedef struct AdjGraph {
    int n;      // count node number
    int e;      // count edge number
    VNode *adj_list;    // node list
} AdjGraph;

// path_info
typedef struct path_info {
    int num;
    unsigned int path[MAX_PATH_LENGTH + 2];
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

// hash
typedef struct node {
    unsigned int ID;
    int ID_index;
    struct node *next;
} hash_node;

typedef struct {
    hash_node *hash_list[MAX_HASH_LENGTH];
    int index_num;
} hash_table;

int min(int a, int b) {
    return a < b ? a : b;
}

int hash_founction(unsigned int number) {
    return number % HASH_NUMBER;
}

int search(unsigned int ID, hash_node *node) {
    while (1) {
        if (node == NULL) {
            return -1;
        }
        if (node->ID == ID) {
            return node->ID_index;
        }

        node = node->next;
    }
}

int HashSearch(unsigned int ID, hash_table *my_table) {
    int index = hash_founction(ID), id_index;
    id_index = search(ID, my_table->hash_list[index]);
    if (id_index == -1) {
        hash_node *p = (hash_node *) malloc(sizeof(hash_node));
        p->ID = ID;
        p->ID_index = my_table->index_num;
        //insert to list
        p->next = my_table->hash_list[index];
        my_table->hash_list[index] = p;
        id_index = my_table->index_num;
        //update table info
        my_table->index_num++;
    }
    return id_index;
}

int *mark_special_node(AdjGraph *G, int index, int *visited);

void DFS(AdjGraph *G, int v, int start, \
        int path_length, int *my_path_length, path_info my_path[], int *temp_path, int visited[]);

AdjGraph *creatAdj(const char *filename);

void write_path(const char *filename, mixed_path_result result);

void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest);

void *multi_thread_dfs(void *msg_raw) {
    thread_info *msg = (struct thread_info *) msg_raw;

    int *visited = (int *) calloc(MAX_NODE_NUMBER, sizeof(int));
    int *temp_path = (int *) malloc((MAX_PATH_LENGTH + 1) * sizeof(int));
    int my_path_length = 0;
    path_info *my_path = (path_info *) malloc(sizeof(path_info) * MAX_RING_NUMBER);
    mixed_path_result *return_temp = (mixed_path_result *) malloc(sizeof(mixed_path_result));
    if (my_path == NULL) {
        printf("Allocate memory failed.\n");
        exit(EXIT_FAILURE);
    }


    int current_node;
    while (1) {
        pthread_mutex_lock(&count_lock);
        current_node = node_number;
        node_number++;
        pthread_mutex_unlock(&count_lock);
//        printf("%d\n", current_node);
        if (node_number < msg->G->n) {
            if (visited[current_node] != -1) {
                DFS(msg->G, current_node, current_node, 0, &my_path_length, my_path, temp_path, visited);
                visited[current_node] = -1;
            } else {
                printf("skip.\n");
                continue;
            }
           // remove node with mark_special_node()
           msg->G->adj_list[current_node].in_degree = 0;
           msg->G->adj_list[current_node].out_degree = 0;
           ArcNode *float_pointer;
           // clear
           float_pointer = msg->G->adj_list[current_node].first_out;
           while (float_pointer != NULL && visited[float_pointer->adj_vex] != -1) {
               msg->G->adj_list[float_pointer->adj_vex].in_degree--;
               float_pointer = float_pointer->next_arc;
           }
           float_pointer = msg->G->adj_list[current_node].first_in;
           while (float_pointer != NULL && visited[float_pointer->adj_vex] != -1) {
               msg->G->adj_list[float_pointer->adj_vex].out_degree--;
               float_pointer = float_pointer->next_arc;
           }
           // special
           float_pointer = msg->G->adj_list[current_node].first_out;
           while (float_pointer != NULL && visited[float_pointer->adj_vex] != -1) {
               //mark_special_node(msg->G, float_pointer->adj_vex, visited);
               float_pointer = float_pointer->next_arc;
           }
           float_pointer = msg->G->adj_list[current_node].first_in;
           while (float_pointer != NULL && visited[float_pointer->adj_vex] != -1) {
               //mark_special_node(msg->G, float_pointer->adj_vex, visited);
               float_pointer = float_pointer->next_arc;
           }
        } else {
            return_temp->num_of_path = my_path_length;
            return_temp->path_list = my_path;
            free(visited);
            free(msg);
            free(temp_path);
            return return_temp;
        }
    }
}

int compare_function(const void *a, const void *b) {
    path_info *c = (path_info *) a;
    path_info *d = (path_info *) b;

    if (c->num < d->num)
        return -1;
    else if (c->num > d->num)
        return 1;
    else {
        for (int i = 0; i < c->num; i++) {
            if (c->path[i] > d->path[i])
                return 1;
            else if (c->path[i] < d->path[i])
                return -1;
        }
    }

}

int main(void) {
    // memory test area
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

#ifdef TIMER_ON
    // timer start
    struct timeval start_time, end_time;
    double time_use;
    gettimeofday(&start_time, 0);
#endif


    AdjGraph *G;
    G = creatAdj(MAP_FILENAME);
    int *visited = (int *) calloc(MAX_NODE_NUMBER, sizeof(int));
    //new add ----->mark special node
    for (int i = 0; i < G->n; i++) {
        visited = mark_special_node(G, i, visited);
    }

#ifdef TIMER_MIDDLE_ON
    gettimeofday(&end_time, 0);
    time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);
#endif

    pthread_t thread_list[THREAD_NUMBER];
    pthread_mutex_init(&count_lock, NULL);
    pthread_mutex_init(&dec_lock, NULL);
    for (int i = 0; i < THREAD_NUMBER; i++) {
        thread_info *current_msg = (thread_info *) malloc(sizeof(thread_info));
        current_msg->G = G;

        pthread_create(thread_list + i, NULL, multi_thread_dfs, current_msg);
    }

#ifdef TIMER_MIDDLE_ON
    gettimeofday(&end_time, 0);
    time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);
#endif

    // join the result
    mixed_path_result all_ring;
    all_ring.num_of_path = 0;
    all_ring.path_list = (path_info *) malloc(sizeof(path_info) * MAX_RING_NUMBER);
    for (int i = 0; i < THREAD_NUMBER; i++) {
        void *return_raw;
        pthread_join(thread_list[i], &return_raw);
        mixed_path_result *ring_of_current_node = (mixed_path_result *) return_raw;
        merge_mixed_path_result(ring_of_current_node, &all_ring);
    }

#ifdef TIMER_MIDDLE_ON
    gettimeofday(&end_time, 0);
    time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);
#endif

    qsort(all_ring.path_list, all_ring.num_of_path, sizeof(path_info), compare_function);

#ifdef TIMER_MIDDLE_ON
    gettimeofday(&end_time, 0);
    time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);
#endif

    write_path(RESULT_FILENAME, all_ring);

#ifdef TIMER_ON
    gettimeofday(&end_time, 0);
    time_use = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;
    printf("Time: %lf s\n", (double) (time_use) / 1000000);
#endif

    return 0;
}

inline int *mark_special_node(AdjGraph *G, int index, int *visited) {
    if (G->adj_list[index].in_degree == 0 && visited[index] != -1) {
        printf("marked %d\n", index);
        visited[index] = -1;
        G->adj_list[index].out_degree = 0;
        ArcNode *float_pointer = G->adj_list[index].first_out;
        while (float_pointer != NULL) {
            G->adj_list[index].in_degree--;
            float_pointer = float_pointer->next_arc;
        }
        float_pointer = G->adj_list[index].first_out;
        while (float_pointer != NULL) {
            index = float_pointer->adj_vex;
            pthread_mutex_lock(&dec_lock);
            G->adj_list[index].in_degree--;
            pthread_mutex_unlock(&dec_lock);
            mark_special_node(G, float_pointer->adj_vex, visited);
            float_pointer = float_pointer->next_arc;
        }
    } else if (G->adj_list[index].out_degree == 0 && visited[index] != -1) {
        printf("marked %d\n", index);
        visited[index] = -1;
        G->adj_list[index].in_degree = 0;
        ArcNode *float_pointer = G->adj_list[index].first_in;
        while (float_pointer != NULL) {
            G->adj_list[index].out_degree--;
            float_pointer = float_pointer->next_arc;
        }
        float_pointer = G->adj_list[index].first_in;
        while (float_pointer != NULL) {
            index = float_pointer->adj_vex;
            pthread_mutex_lock(&dec_lock);
            G->adj_list[index].out_degree--;
            pthread_mutex_unlock(&dec_lock);
            mark_special_node(G, float_pointer->adj_vex, visited);
            float_pointer = float_pointer->next_arc;
        }
    }
    return visited;
}

AdjGraph *creatAdj(const char *filename) {
    FILE *map_file = fopen(filename, "r");  // data file

    AdjGraph *G = (AdjGraph *) malloc(sizeof(AdjGraph));    // head pointer of graph
    G->adj_list = (VNode *) malloc(sizeof(VNode) * MAX_NODE_NUMBER);

    ArcNode *p, *q;     // temp pointer for arc
    hash_table my_table;
    my_table.index_num = 0;
    for (int a = 0; a < MAX_HASH_LENGTH; a++) {
        my_table.hash_list[a] = NULL;
    }
    unsigned int i, j, k;
    int index_i, index_j;
    // init graph info
    G->e = 0;
    G->n = 0;
    for (int a = 0; a < MAX_NODE_NUMBER; a++) {
        G->adj_list[a].first_out = NULL;
        G->adj_list[a].first_in = NULL;
        G->adj_list[a].in_degree = 0;
        G->adj_list[a].out_degree = 0;
    }

    while (EOF != fscanf(map_file, "%d,%d,%d\n", &i, &j, &k)) {
        index_i = HashSearch(i, &my_table);
        index_j = HashSearch(j, &my_table);
        p = (ArcNode *) malloc(sizeof(ArcNode));
        q = (ArcNode *) malloc(sizeof(ArcNode));

        p->adj_vex = index_j;
        q->adj_vex = index_i;
        // insert to out list
        p->next_arc = G->adj_list[index_i].first_out;
        G->adj_list[index_i].first_out = p;
        G->adj_list[index_i].ID = i;
        //insert to in list
        q->next_arc = G->adj_list[index_j].first_in;
        G->adj_list[index_j].first_in = q;
        G->adj_list[index_j].ID = j;

        G->adj_list[index_j].in_degree++;
        G->adj_list[index_i].out_degree++;
        // update graph info
        G->e++;
    }
    G->n = my_table.index_num;
    return G;
}

void DFS(AdjGraph *G, int v, const int start, \
int path_length, int *my_path_length, path_info my_path[], int *temp_path, int visited[]) {

    if (visited[v] == -1)
        return;

    ArcNode *p; // floating pointer
    int w;  // temp to store the current node index



    visited[v] = 1;        // mark visited
    p = G->adj_list[v].first_out;        // pointer to the edge link list

    temp_path[path_length] = G->adj_list[v].ID;  // write current node to the path
    path_length++;   // increase path length

    while (p != NULL) {
        w = p->adj_vex; // outbound index number

        // find a ring
        if (w == start && path_length > 2) {
            // copy path to the output: dest, src
            int min = temp_path[0], index = 0, i;
            for (i = 0; i < path_length; i++) {
                if (min > temp_path[i]) {
                    min = temp_path[i];
                    index = i;
                }
            }
            for (i = 0; i < path_length; i++) {
                if (i + index < path_length) {
                    my_path[*my_path_length].path[i] = temp_path[i + index];
                } else {
                    my_path[*my_path_length].path[i] = temp_path[i + index - path_length];
                }
            }
            //memcpy(my_path[*my_path_length].path, temp_path, sizeof(int) * path_length);
            my_path[*my_path_length].num = path_length;    // set the path length
            (*my_path_length)++;    // mark the total number of path

            //printf(".");
        }

        // TODO: could change temp_path[0] -> start ?
        if (visited[w] == 0 && w > start && path_length < 7)
            DFS(G, w, start,
                path_length, my_path_length, my_path, temp_path, visited);    // recursion

        p = p->next_arc;      // next
    }

    visited[v] = 0; // clear mark
//    temp_path[path_length] = 0;  // clear path

    return;
}


void merge_mixed_path_result(mixed_path_result *src, mixed_path_result *dest) {
    int base = dest->num_of_path;
    for (int i = 0; i < src->num_of_path; i++) {
        memcpy(dest->path_list + base + i, src->path_list + i, sizeof(path_info));
    }
    free(src->path_list);
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

