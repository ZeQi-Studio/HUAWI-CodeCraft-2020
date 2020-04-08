#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXV 8
#define max_lenth 7

typedef struct ANode
{     int adjvex;			//�ñߵ��յ���
      struct ANode *nextarc;	//ָ����һ���ߵ�ָ��
}  ArcNode;

typedef struct Vnode
{    int data;			//������Ϣ
     ArcNode *firstarc;		//ָ���һ����
}  VNode;

typedef struct 
{     
	int n,e; 
	VNode adjlist[MAXV] ;	//�ڽӱ�
} AdjGraph;

typedef struct
{
	int num;
	int path[max_lenth];
}path_info;

int visited[MAXV]={0};
int start;
path_info my_path[MAXV];
int temp_path[max_lenth];
int path_lenth = 0;
int my_path_lenth = 0;

void DFS(AdjGraph *G,int v)  
{
	ArcNode *p; int w;
    visited[v]=1; 		//���ѷ��ʱ��
    printf("%d  ",v); 		//��������ʶ���ı��
    p=G->adjlist[v].firstarc;     	//pָ�򶥵�v�ĵ�һ���ߵı�ͷ���
    temp_path[path_lenth] = v;
    path_lenth++;
	while (p!=NULL){
	   	w=p->adjvex;
	   	if(w == start)
	   	{
	   		memcpy(my_path[my_path_lenth].path,temp_path,sizeof(int)*path_lenth);
	   		my_path[my_path_lenth].num=path_lenth;
	   		my_path_lenth++;
	   		printf("acc\n");
		   }
		if(visited[w]==0)
	       DFS(G,w);   	//��w����δ���ʣ��ݹ������
		p=p->nextarc;      //pָ�򶥵�v����һ���ߵı�ͷ���
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

void DispAdj(AdjGraph *G)	//����ڽӱ�G
{      int i;
       ArcNode *p;
       for (i=0;i<=G->n;i++)
       {	p=G->adjlist[i].firstarc;
	printf("%d: ",i);
	while (p!=NULL)
	{       printf("%3d��",p->adjvex);
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
void main()
{
	AdjGraph *G;
	G = (AdjGraph*)malloc(sizeof(AdjGraph));
    const char* filename = "C:\\Users\\10372\\Desktop\\����\\myTest_data.txt" ;
    G = creatAdj(filename);
    int i;
//    for(i=0;i<=G->n;i++)
//    {
//    	printf("[%d]",i);
//    	start = i;
//    	DFS(G,i);
//		int j;
//		for(j = 0;j<MAXV;j++){
//			visited[j]=0;
//		}	
//	}
//	start = 1;
//	DFS(G,1);
    print_path();
}

