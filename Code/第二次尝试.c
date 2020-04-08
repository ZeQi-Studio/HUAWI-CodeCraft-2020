#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXV 6500
#define max_lenth 7

typedef struct ANode
{     int adjvex;			//该边的终点编号
      struct ANode *nextarc;	//指向下一条边的指针
}  ArcNode;

typedef struct Vnode
{    int data;			//顶点信息
     ArcNode *firstarc;		//指向第一条边
}  VNode;

typedef struct 
{     
	int n,e; 
	VNode adjlist[MAXV] ;	//邻接表
} AdjGraph;

typedef struct
{
	int num;
	int path[max_lenth];
}path_info;

int visited[MAXV]={0};
int start=0;
path_info my_path[MAXV];
int temp_path[max_lenth];
int path_lenth = 0;
int my_path_lenth = 0;

void DFS(AdjGraph *G,int v)  
{ 
	ArcNode *p; int w;
    visited[v]=1; 		//置已访问标记
    p=G->adjlist[v].firstarc;     	//p指向顶点v的第一条边的边头结点
    temp_path[path_lenth] = v;
    path_lenth++;
	while (p!=NULL){
	   	w=p->adjvex;
	   	if(w == start&&path_lenth>2)
	   	{
	   		memcpy(my_path[my_path_lenth].path,temp_path,sizeof(int)*path_lenth);
	   		my_path[my_path_lenth].num=path_lenth;
	   		my_path_lenth++;
	   		printf("acc\n");
		   }
		if(visited[w]==0&&w>temp_path[0]&&path_lenth<7)
	       DFS(G,w);   	//若w顶点未访问，递归访问它
		p=p->nextarc;      //p指向顶点v的下一条边的边头结点
    }
    visited[v]=0;
    temp_path[path_lenth] = 0;
    path_lenth--;
}

AdjGraph* creatAdj(const char*filename){
	AdjGraph* G;
    FILE* map_file;
    map_file = fopen(filename,"r");
    int i, j,k;
    ArcNode* p;
    G = (AdjGraph*)malloc(sizeof(AdjGraph));
    G->e=0;
    G->n=0;
    int a;
    for(a = 0;a < MAXV;a++){
        G->adjlist[a].firstarc=NULL;
    }
    while(!feof(map_file)){
        fscanf(map_file,"%d,%d,%d\n",&i,&j,&k);
        p = (ArcNode*)malloc(sizeof(ArcNode));
        p->adjvex = j;
        p->nextarc = G->adjlist[i].firstarc;
        G->adjlist[i].firstarc = p;
        G->e++;
        if(i>G->n){
        	G->n=i;
		}
    }
	return G;
}

void DispAdj(AdjGraph *G)	//输出邻接表G
{      int i;
       ArcNode *p;
       for (i=0;i<=G->n;i++)
       {	p=G->adjlist[i].firstarc;
	printf("%d: ",i);
	while (p!=NULL)
	{       printf("%3d→",p->adjvex);
	         p=p->nextarc;
	}
	printf("end\n");
       }
}
void print_path()
{
	int i;
	for(i = 0;i<my_path_lenth;i++){
		int j;
		for(j=0;j<my_path[i].num;j++){
			printf("%d->",my_path[i].path[j]);
		}
		printf("\n");
	}
}

void write_path(const char*filename)
{
    FILE* path_file;
    path_file = fopen(filename,"w");
	fprintf(path_file,"%d\n",my_path_lenth)	;
	int i,j,k;
	for(k=3;k<=max_lenth;k++){
		for(i=0;i<my_path_lenth;i++){
			if(my_path[i].num==k){	
				for(j=0;j<my_path[i].num;j++){
					if(j==0){
						fprintf(path_file,"%d",my_path[i].path[j]);
					}
					else{
						fprintf(path_file,",%d",my_path[i].path[j]);
					}
				}
			fprintf(path_file,"\n");
			}
		}
	}
}
void main()
{
	AdjGraph *G;
	G = (AdjGraph*)malloc(sizeof(AdjGraph));
    const char* map_filename = "C:\\Users\\10372\\Desktop\\初赛\\test_data.txt" ;
    const char* result_filename = "my_result.txt";
    G = creatAdj(map_filename);
    int i;
    for(i=0;i<=G->n;i++)
    {
    	start = i;
    	DFS(G,i);
		int j;
		for(j = 0;j<MAXV;j++){
			visited[j]=0;
		}	
	}
//	start = 1;
//	DFS(G,1);
    write_path(result_filename);
}

