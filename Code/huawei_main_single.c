#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include "time.h"

#define MAP_FILENAME  "/data/test_data.txt"
#define RESULT_FILENAME  "/projects/student/result.txt"

#define Bool int
#define True 1
#define  False 0
#define  P 560000 //56万附近的大素数
#define MAXV 560000



typedef struct node     //表结点
{   unsigned int ID;
    int ID_index;
    Bool sign;
    struct node * nextarc;
}EdgeNode;

typedef struct Link_node       //链表头结点
{   int ID;
    Bool sign;
    struct node *firstarc;
    struct Link_node *nextlink;
}LNode;

typedef struct Vnode    //头结点
{    int ID;
     Bool sign;
    EdgeNode *firstarc;
    LNode *firstlink;
}VNode;

typedef struct AdjGraph {
    int n;      // count node number
    int e;      // count edge number
    VNode adj_list[MAXV];    // node list
} AdjGraph;

typedef struct Loop_path{
    int path[7];
    int length;
    struct Loop_path *nextlp;
}Loop_path;


AdjGraph *creatAdj(const char *filename) {
    FILE *map_file = fopen(filename, "r");  // data file
    AdjGraph *G = (AdjGraph *) malloc(sizeof(AdjGraph));    // head pointer of graph

    // init graph info
    G->e = 0;
    G->n = 0;
    int a;
    for (a = 0; a < MAXV; a++) {
        G->adj_list[a].ID=-1;
        G->adj_list[a].sign = False;
        G->adj_list[a].firstarc = NULL;
        G->adj_list[a].firstlink = NULL;
    }

    int i,j,k,index;
    EdgeNode *e,*t;

    while (EOF != fscanf(map_file, "%d,%d,%d\n", &i, &j, &k))
    {   //存储表结点信息
        G->n++;
        e = (EdgeNode *)malloc(sizeof(EdgeNode));
        e->ID = j;
        e->sign = False;
        e->ID_index = (j+1)%P;
        e->nextarc = NULL;

        index = (i+1)%P;        //将ID号映射成数组下标号
        //此时index下标的数组头结点存在三种状态
        if( G->adj_list[index].firstarc == NULL){    //下标index的数组头结点全新，未占用
            G->adj_list[index].ID = i;      //填充头结点ID信息
            G->adj_list[index].firstarc = e;    //插入表结点
            G->e++;
        }
        else if (G->adj_list[index].ID == i){       //下标index的数组头结点就是该ID号的头结点
            e->nextarc = G->adj_list[index].firstarc;   //头插法插入该表结点
            G->adj_list[index].firstarc = e;
        }
        else{   //下标index的数组头结点不是该ID号的头结点，且存在链表头结点
            LNode *r,*pr;       //pr是r的前驱指针
            r = G->adj_list[index].firstlink;
            if (r==NULL){       //r为空，表明该数组头结点下面无链表头结点
                LNode *first_new = (LNode *)malloc(sizeof(LNode));  //新建第一个链表头结点
                first_new->ID = i;
                first_new->sign = False;
                first_new->nextlink = NULL;
                first_new->firstarc = e;
                G->adj_list[index].firstlink = first_new;
            }
            else{
                //遍历该数组头结点连接的所有链表头结点，判断该ID号是否已经建立链表头结点，有则插入表结点，无则建立链表头结点
                while(r!=NULL){
                    if (r->ID == i){
                        e->nextarc = r->firstarc;       //头插法插入表结点
                        r->firstarc = e;
                        break;      //跳出while循环
                    }
                    pr = r;
                    r = r->nextlink;
                }
                if (r==NULL){   //遍历结束仍未发现该ID号的链表头结点
                    G->e++;
                    LNode *new = (LNode *)malloc(sizeof(LNode));        //创建新的链表头结点
                    pr->nextlink = new;     //连接新的链表结点
                    new->ID = i;        //插入表结点
                    new->sign = False;
                    new->nextlink = NULL;
                    new->firstarc = e;
                }
            }

        }
    }
    fclose(map_file);
    return G;
}

void initLoop_path(Loop_path **Loopth);

Loop_path *DFS(AdjGraph **G,struct Loop_path *Loopth,struct Loop_path *T,EdgeNode *p,int ID){
    while (p!=NULL){
        if((*G)->adj_list[p->ID_index].ID == -1 ){
            p=p->nextarc;
            continue;
        }

        //复制来自上层DFS的路径遍历记录
        Loop_path *t=(Loop_path *)malloc(sizeof(Loop_path));
        t->length = T->length;
        t->nextlp = T->nextlp;
        int i=0;
        while(i<7){
            t->path[i] = T->path[i];
            i++;
        }

        if ((*G)->adj_list[p->ID_index].ID == p->ID && (*G)->adj_list[p->ID_index].sign == False){    //找到表结点对应的未访问的数组头结点
            if ((*G)->adj_list[p->ID_index].ID == ID ){//找到环
                if(t->length>1){
                    t->nextlp = Loopth->nextlp;
                    Loopth->nextlp = t;
                }
                p=p->nextarc;
                continue;
            }
            t->length++;
            if (t->length > 6){
                free(t);
                p=p->nextarc;
                continue;
            }
            (*G)->adj_list[p->ID_index].sign = True;
            t->path[t->length]=p->ID;
            EdgeNode *e = (*G)->adj_list[p->ID_index].firstarc;
            Loopth = DFS(G,Loopth,t,e,ID);
            (*G)->adj_list[p->ID_index].sign = False;
        }
        else if ((*G)->adj_list[p->ID_index].ID != p->ID){
            LNode *ln = NULL;
            ln = (*G)->adj_list[p->ID_index].firstlink;
            while(ln != NULL && ln->ID != p->ID ){
                ln = ln->nextlink;
            }
            //ln = NULL,遍历了所有链表头结点，结束并重置表结点标志位
            if(ln == NULL || ln->sign==True){
                free(t);
                p=p->nextarc;
                continue;
            }
            if (ln->ID == ID ){//找到环
                if(t->length>1){
                    t->nextlp = Loopth->nextlp;
                    Loopth->nextlp = t;
                }
                p=p->nextarc;
                continue;
            }
            else if(ln->sign == False){

                t->length++;
                if (t->length >6){
                    free(t);
                    p=p->nextarc;
                    continue;
                }
                ln->sign = True;
                t->path[t->length]=ln->ID;
                EdgeNode *le = ln->firstarc;
                Loopth = DFS(G,Loopth,t,le,ID);
                ln->sign = False;
            }

        }
        p=p->nextarc;
    }
    return Loopth;
}

Loop_path *FindLoops(AdjGraph **G,Loop_path *Loopth){
    int i=0;
    initLoop_path(&Loopth);
    Loop_path *t;

    while(i<MAXV){
        if((*G)->adj_list[i].ID != -1){
            initLoop_path(&t);
            t->path[0]=(*G)->adj_list[i].ID;
            Loopth = DFS(G,Loopth,t,(*G)->adj_list[i].firstarc,(*G)->adj_list[i].ID);
            (*G)->adj_list[i].sign = True;
            LNode *LN = (*G)->adj_list[i].firstlink;
            while(LN != NULL){
                t->path[0]=LN->ID;
                Loopth = DFS(G,Loopth,t,LN->firstarc,LN->ID);
                LN->sign = True;
                LN = LN->nextlink;
            }
        }
        i++;
    }
    return Loopth;
}

void initLoop_path(Loop_path **Loopth){
    *Loopth = (Loop_path *)malloc(sizeof(Loop_path));
    for(int i=0;i<7;i++){
        (*Loopth)->path[i] = -1;
    }
    (*Loopth)->length = 0;
    (*Loopth)->nextlp = NULL;
}

void PrintGraph(AdjGraph *G);

Bool Bigger(Loop_path*m,Loop_path*n){//路径长度大、ID号依次序较的链表判为大
    if(m->length>n->length){
        return True;
    }
    else if (m->length<n->length){
        return False;
    }
    else{
        int i=0;
        while (i<=m->length){
            if(m->path[i]>n->path[i]){
                return True;
            } else if (m->path[i]<n->path[i]){
                return False;
            }
             i++;
        }
    }
}

//给所有环的链表排序，返回值为指针Lp，Lp->nextlp指向排好序的最小环,Lp->length指示环的个数
Loop_path *sort_path(Loop_path *Loopth){
    Loop_path *l= Loopth->nextlp,*Lp,*max_lp;
    //Lp作为头结点指针，指向长度、ID号相对最小的ID链表
    initLoop_path(&Lp);
    initLoop_path(&max_lp);
    max_lp->length=8;
    Lp->nextlp=max_lp;
    int count=0;
    do{
        unsigned int i,minID_index=0,length=l->length+1,temp=l->path[0];
        for (i=1;i<length;i++){//遍历查找环最小有效ID及其索引号j
            if(l->path[i]<temp){
                temp=l->path[i];
                minID_index=i;
            }
        }
        if(minID_index!=0){//给环路径排序，最小ID在前
            unsigned int j=minID_index,a[7],k=0;
            for(i=0;i<minID_index;i++){
                a[i] = l->path[i];
            }
            for(i=0;i<(length-minID_index);i++){
                l->path[i++]=l->path[j++];
            }
            for(i;i<length;i++){//此时j=数组长度,k=数组长度-最小ID索引号
                l->path[i]=a[k++];
            }
        }
        Loop_path *s,*l_copy=l;
        l=l->nextlp;
        s=Lp;
        while(s!=NULL){
            if(Bigger(s,l_copy)==False && Bigger(s->nextlp,l_copy)==True){//插入处理好的链表
                l_copy->nextlp=s->nextlp;
                s->nextlp=l_copy;
                break;
            }
            s=s->nextlp;
        }

        count++;

    }while(l!=NULL);

    Lp->length =count;

    return Lp;

}


void Write_file(Loop_path *Loopth, const char *filename){
    FILE *save_file;
    Loop_path *L=Loopth;
    save_file = fopen(filename, "w");

    fprintf(save_file, "%d\n", L->length);
    while(L->nextlp->nextlp!=NULL){
        for(int i=0;i<L->nextlp->length;i++){
            fprintf(save_file, "%d,", L->nextlp->path[i]);
        }
        fprintf(save_file, "%d\n", L->nextlp->path[L->nextlp->length]);
        L=L->nextlp;
    }
    fclose(save_file);
}




//测试图散列邻接表的构建
int main(void){
//    printf("dddddd\n");
//    clock_t start,end;
//    start = clock();
    AdjGraph *G;
    //G =(AdjGraph *)malloc(sizeof(AdjGraph));
    //建图
    G = creatAdj(MAP_FILENAME);
    //PrintGraph(G);
    //遍历查找所有环
    Loop_path *Loopth,*L;
    initLoop_path(&Loopth);
    Loopth=FindLoops(&G,Loopth);
    Loopth=sort_path(Loopth);
    Write_file(Loopth,RESULT_FILENAME);

//    end = clock();
//    printf("time=%f s\n",(double)(end - start)/CLK_TCK);
    /*
    L=Loopth->nextlp;
    printf("Sum:  %d \n",Loopth->length);
    int count=0;
    while(L!=NULL){
        printf("%d:   ",L->length+1);
        for(int i=0;i<6;i++){
            printf("%d,",L->path[i]);
        }
        printf("%d\n",L->path[6]);
        L=L->nextlp;
    }
    */



}



void PrintGraph(AdjGraph *G){
    int i=0;
    for(i;i<MAXV;i++){
        if (G->adj_list[i].firstarc!=NULL){     //证明该数组头结点存在表结点
            EdgeNode *p = G->adj_list[i].firstarc;
            while (p!=NULL){
                printf("%d,%d\n",G->adj_list[i].ID,p->ID);
                p = p->nextarc;
            }
            if (G->adj_list[i].firstlink!=NULL){    //该数组头结点连接的有链表头结点
                LNode *s = G->adj_list[i].firstlink;
                while (s!=NULL){
                    EdgeNode *p = s->firstarc;
                    while (p!=NULL){
                        printf("->%d,%d\n",s->ID,p->ID);
                        p = p->nextarc;
                    }
                    s=s->nextlink;
                }
            }

        }
    }
}




