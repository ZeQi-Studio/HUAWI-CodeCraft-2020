#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAP_FILENAME  "./Data/3738/test_data.txt"
#define RESULT_FILENAME  "./Output/my_result.txt"
#define MAXV 1000000
#define MAX_PATH_LENGTH 7
#define THREAD_NUMBER 8

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

// path
typedef struct path_info {
    int num;
    int path[MAX_PATH_LENGTH];
} path_info;


void DFS(AdjGraph *G, int v, int start);

AdjGraph *creatAdj(const char *filename);

void DispAdj(AdjGraph *G);

void print_path();

void write_path(const char *filename);


path_info my_path[MAXV];
int temp_path[MAX_PATH_LENGTH];
int path_lenth = 0;
int my_path_lenth = 0;

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

    for (int i = 0; i <= G->n; i++) {
        //printf("Searching node %d\n", i);
        DFS(G, i, i);
    }
//    start = 1;
//    DFS(G, 1);
    write_path(RESULT_FILENAME);

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


// mark visited
int visited[MAXV] = {0};
void DFS(AdjGraph *G, int v, const int start) {

    ArcNode *p;
    int w;
    visited[v] = 1;        //置已访问标记
    p = G->adj_list[v].first_arc;        //p指向顶点v的第一条边的边头结点
    temp_path[path_lenth] = v;
    path_lenth++;

    while (p != NULL) {
        w = p->adj_vex; // outbound index number

        // find a ring
        if (w == start && path_lenth > 2) {
            memcpy(my_path[my_path_lenth].path, temp_path, sizeof(int) * path_lenth);
            my_path[my_path_lenth].num = path_lenth;
            my_path_lenth++;
            printf("acc\n");
        }
        if (visited[w] == 0 && w > temp_path[0] && path_lenth < 7)
            DFS(G, w, start);    //若w顶点未访问，递归访问它
        p = p->next_arc;      //p指向顶点v的下一条边的边头结点
    }

    visited[v] = 0;
    temp_path[path_lenth] = 0;
    path_lenth--;
}


void DispAdj(AdjGraph *G)    //输出邻接表G
{
    int i;
    ArcNode *p;
    for (i = 0; i <= G->n; i++) {
        p = G->adj_list[i].first_arc;
        printf("%d: ", i);
        while (p != NULL) {
            printf("%3d→", p->adj_vex);
            p = p->next_arc;
        }
        printf("end\n");
    }
}

void print_path() {
    int i;
    for (i = 0; i < my_path_lenth; i++) {
        int j;
        for (j = 0; j < my_path[i].num; j++) {
            printf("%d->", my_path[i].path[j]);
        }
        printf("\n");
    }
}

void write_path(const char *filename) {
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
                fprintf(path_file,  "\n");
            }
        }
    }
}