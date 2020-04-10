#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAP_FILENAME  "/data/test_data.txt"
#define RESULT_FILENAME  "/projects/student/result.txt"
#define MAXV 560000
#define MAX_PATH_LENGTH 7
#define THREAD_NUMBER 8
#define MAX_HASH_LENGTH 56000
#define HASH_NUMBER 55997

typedef struct ArcNode {
    unsigned int adj_vex;    // out bound of vec
    struct ArcNode *next_arc;  // next vec
}/*__attribute__((packed))*/ ArcNode;

typedef struct Vnode {
    unsigned int ID;               // unused node info
    ArcNode *first_arc;      // head of list
} VNode;

// graph
typedef struct AdjGraph {
    int n;      // count node number
    int e;      // count edge number
    VNode adj_list[MAXV];    // node list
} AdjGraph;

// path
typedef struct path {
    unsigned int path[MAX_PATH_LENGTH];
} path;

typedef struct path_info{
    path path[MAXV];
    int length;		//record the path(The number of nodes in the path is constant) length
}path_info;
typedef struct node{
    unsigned int ID;
    int ID_index;
    struct node *next;
}hash_node;

typedef struct{
    hash_node *hash_list[MAX_HASH_LENGTH];
    int index_num;
}hash_table;

int min(int a, int b) {
    return a < b ? a : b;
}

int hash_founction(unsigned int number){
    return number%HASH_NUMBER;
}

//void init_hash_table(hash_table *my_table){
//    my_table = (hash_table*)malloc(sizeof(hash_table));
//    my_table->index_num=0;
//    printf("%d\n",my_table->index_num);
//    int i;
//    for(i=0;i<MAX_HASH_LENGTH;i++){
//        my_table->hash_list[i]=NULL;
//    }
//}

int search(unsigned int ID,hash_node *node){
    while(1){
        if(node==NULL){
            return -1;
        }
        if(node->ID==ID){
            return node->ID_index;
        }

        node = node->next;
    }
}

int HashSearch(unsigned int ID,hash_table* my_table){
    int index = hash_founction(ID),id_index;
    id_index = search(ID,my_table->hash_list[index]);
    if(id_index==-1){
        hash_node *p = (hash_node*)malloc(sizeof(hash_node));
        p->ID=ID;
        p->ID_index=my_table->index_num;
        //insert to list
        p->next= my_table->hash_list[index];
        my_table->hash_list[index] = p;
        id_index = my_table->index_num;
        //update table info
        my_table->index_num++;
    }
    return id_index;
}
void DFS(AdjGraph *G, int v, int start);

AdjGraph *creatAdj(const char *filename);

void DispAdj(AdjGraph *G);

void print_path();

void write_path(const char *filename);

void bubble_sort_path();

int compile(int a[],int b[],int i);

path_info my_path[MAX_PATH_LENGTH-2];
unsigned int temp_path[MAX_PATH_LENGTH];
int path_lenth = 0;

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
    int i ;
    for (i= 0; i < 280000; i++) {
        //printf("%d",start_pointer->adj_vex);
        start_pointer = start_pointer->next_arc;
    }
    printf("Memory test done.\n");
    //exit(EXIT_SUCCESS);
#endif


    //init path info
    int j;
    for (j = 0; j < MAX_PATH_LENGTH-3; j++) {
        my_path[j].length=0;
    }
    AdjGraph *G;
    // G = (AdjGraph *) malloc(sizeof(AdjGraph));   // useless `malloc`
    G = creatAdj(MAP_FILENAME);
    int i;
    for (i = 0; i <= G->n; i++) {
        //printf("Searching node %d\n", i);
        DFS(G, i, i);
    }
    bubble_sort_path();
    write_path(RESULT_FILENAME);
    return 0;
}


AdjGraph *creatAdj(const char *filename) {
    FILE *map_file = fopen(filename, "r");  // data file
    AdjGraph *G = (AdjGraph *) malloc(sizeof(AdjGraph));    // head pointer of graph
    ArcNode *p;     // temp pointer for arc
    hash_table my_table;
    my_table.index_num=0;
    int a;
    for(a=0;a<MAX_HASH_LENGTH;a++){
        my_table.hash_list[a]=NULL;
    }
    unsigned int i, j, k;
    int index_i,index_j;

    // init graph info
    G->e = 0;
    G->n = 0;
    for (a = 0; a < MAXV; a++) {
        G->adj_list[a].first_arc = NULL;
    }

    while (EOF != fscanf(map_file, "%d,%d,%d\n", &i, &j, &k)) {
        index_i = HashSearch(i,&my_table);
        index_j = HashSearch(j,&my_table);
        p = (ArcNode *) malloc(sizeof(ArcNode));
        //p->adj_vex = j;
        p->adj_vex = index_j;
        // insert to list
        p->next_arc = G->adj_list[index_i].first_arc;
        G->adj_list[index_i].first_arc = p;
        G->adj_list[index_i].ID =i;
        // update graph info
        G->e++;
    }
    G->n = my_table.index_num;
    fclose(map_file);
    return G;
}


// mark visited
int visited[MAXV] = {0};
void DFS(AdjGraph *G, int v, const int start) {
    ArcNode *p;
    int w;
    visited[v] = 1;        //置已访问标记
    p = G->adj_list[v].first_arc;        //p指向顶点v的第一条边的边头结点
    temp_path[path_lenth] = G->adj_list[v].ID;
    path_lenth++;
    //printf("%d,%d\n",v,G->adj_list[v].ID);
    while (p != NULL) {
        w = p->adj_vex; // outbound index number

        // find a ring
        if (w == start && path_lenth > 2) {
            //sort ring
            int min= temp_path[0],index=0,i;
            for(i = 0;i<path_lenth;i++){
                if(min>temp_path[i]){
                    min = temp_path[i];
                    index = i;
                }
            }
            for(i = 0;i<path_lenth;i++){
                if(i+index<path_lenth){
                    my_path[path_lenth-3].path[my_path[path_lenth-3].length].path[i] = temp_path[i+index];
                }
                else{
                    my_path[path_lenth-3].path[my_path[path_lenth-3].length].path[i] = temp_path[i+index-path_lenth];
                }
            }
            //memcpy(my_path[path_lenth-3].path[my_path[path_lenth-3].length].path, temp_path, sizeof(unsigned int) * path_lenth);
            my_path[path_lenth-3].length++;
            printf("%dacc\n",my_path[0].length+my_path[1].length+my_path[2].length+my_path[3].length+my_path[4].length);
        }
        if (visited[w] == 0 && w > start && path_lenth < 7)
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

void write_path(const char *filename) {
    FILE *path_file;
    path_file = fopen(filename, "w");
    int all_path_length=0;
    int j ,i,k;
    for(j= 0;j<MAX_PATH_LENGTH-2;j++){
        all_path_length+=my_path[j].length;
    }
    fprintf(path_file, "%d\n", all_path_length);
    for (i = 0; i < MAX_PATH_LENGTH-2; i++) {
        for (j = 0; j < my_path[i].length; j++) {
            for(k=0;k<i+3;k++){
                if(k==0){
                    fprintf(path_file,"%d",my_path[i].path[j].path[k]);
                }
                else{
                    fprintf(path_file,",%d",my_path[i].path[j].path[k]);
                }
            }
            fprintf(path_file,"\n");
        }
    }
    fclose(path_file);
}
void bubble_sort_path(){
    int i,j,k;
    int temp[MAX_PATH_LENGTH];
    for(i=0;i<MAX_PATH_LENGTH-2;i++){
        for(j=0;j<my_path[i].length;j++){
            for (k = 0;  k<j ; k++) {
                if (compile(my_path[i].path[j].path,my_path[i].path[k].path,0)==1){
                    memcpy(temp,my_path[i].path[j].path, sizeof(unsigned int)*(i+3));
                    memcpy(my_path[i].path[j].path,my_path[i].path[k].path ,sizeof(unsigned int)*(i+3));
                    memcpy(my_path[i].path[k].path,temp, sizeof(unsigned int)*(i+3));
                }
            }
        }
    }
}

int compile(int a[],int b[],int i){
    if(a[i]==b[i]){
        i++;
        compile(a,b,i);
    }
    else if(a[i]>b[i]){
        return -1;
    }
    else if(a[i]<b[i]){
        return 1;
    }
}