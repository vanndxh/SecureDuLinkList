/*
* Operation System Curriculum Design
* Created by Vanndxh on 2021/5
* Copyright © 2021 Vanndxh. All rights reserved.
*/


#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <process.h>
using namespace std;

#define random(x)(rand() %x)
#define insert 1
#define erase 0
volatile int readcount = 0;
const int lsarea=1000;
const int earea=1000;
const int sum=15;
int th=0;
int th_find = 1;
int th_insert=1;
int th_erase=1;
HANDLE rmutex;//控制读者数量readcount的互斥访问量
HANDLE mutex;//控制读写互斥，写写互斥的信号量
typedef int ElemType; 
typedef struct DuLNode* PNode;

//定义结点结构体
typedef struct DuLNode {
    ElemType data; //定义数据域
    PNode prior;//定义前驱指针
    PNode next;//定义后继指针
} DuLNode,*DLN;

//定义双向链表结构体
typedef struct DuLinkList {
    DLN head; // 定义头结点
    int Length;
} DuLinkList,*DLL;

//定义读者传参结构体
struct Readarg{
    DLL List;
    ElemType e;//定义查找元素
} ;

//定义写者传参结构体
struct Writearg{
    DLL List;
    int index;
    ElemType e;
    int flag;
};



//1、InitList函数
void InitList(DLL L){
    int c, i, e;
    DLN p;
    L->head = 0;
    L->Length = 0;
    printf("双向链表初始化完毕\n");

    srand((int)time(0));
    c = random(lsarea);
    p = (DuLNode*)malloc(sizeof(DuLNode));
    e = random(earea);
    p->data = e;
    p->next = p->prior = p;
    L->head = p;
    L->Length++;
    //下面循环插入后续结点操作
    for (i = 1; i < c; i++) {
        p = (DuLNode*)malloc(sizeof(DuLNode));
        e = random(earea);
        p->data = e;
        p->next = L->head;
        p->prior = L->head->prior;
        L->head->prior->next = p;
        L->head->prior = p;
        L->Length++;
    }
}

//2、Insert函数
void Insert(DLL L, int i, ElemType e) {
    DLN p, q;
    int j;
    if (i<1 || i>L->Length + 1) {
        printf("插入的位置超过了链表范围! \n\n");
    }
    else {
        printf("插入成功! \n\n");
        p = (DuLNode*)malloc(sizeof(DuLNode));
        if (!p) {
            printf("结点p动态分配内存失败! \n");
            exit(0);
        }
        q = L->head;
        p->data = e;
        if (!L && i == 1) {//判断链表为空并且插入第一个元素的情况
            p->next = p->prior = p;
            L->head = p;
            L->Length++;
        }
        else {
            if (i == 1 || i == L->Length + 1) {//判断链表不为空,在当前表头或表尾插入结点
                p->next = q;
                p->prior = q->prior;
                q->prior->next = p;
                q->prior = p;
                L->Length++;
                if (i == 1)//判断插入位置是否是头结点
                    L->head = p;
            }
            else {
                for (j = 1; j < i; j++) {//循环定位插入位置
                    q = q->next;
                }
                q->prior->next = p;
                p->prior = q->prior;
                q->prior = p;
                p->next = q;
                L->Length++;  
            }
        }
    }
}

//3、Erase函数
void Erase(DLL L, int i) {
    DLN q;
    q = L->head;
    if (i<1 || i>(L->Length) || L->head == NULL) {
        printf("删除的位置超过了链表范围或者链表为空! \n\n");
    }
    else {
        printf("删除成功! \n\n");
        for (int j = 1; j < i; j++) {
            q = q->next;
        }
        q->prior->next = q->next;
        q->next->prior = q->prior;
        L->Length--;
        if (i == 1) {
            L->head = q->next;
        }
        if (L->Length == 0) {
            L->head = NULL;
        }
        free(q);
    }
}

//4、Clear函数
void Clear(DLL L) {
    DLN p, temp;
    int i,j;
    i=L->Length;
    temp = p = L->head;
    for(j=0 ; j<i ; j++){
        p = p->next;
        free(temp); 
        temp = p;
        L->Length--;
    }
    L->head = NULL;
    printf("链表已清空! ");
}

//5、Find函数
DLN Find(DLL L, ElemType e) {
    DLN p;
    p = L->head;
    if (!p) {
        printf("链表为空!找不到元素%d\n\n", e);
        return 0;
    }
    else {
        while (p->data != e && p->next != L->head) {
            p = p->next;
        }
        if (p->data == e) {
            printf("元素%d已找到!指针已返回! \n\n", e); 
            return p;
        }
        else {
            printf("元素%d未找到!返回空\n\n", e); 
            return NULL;
        }
    }
}

//6、Print函数
void Print(DLL L) {
    DLN p;
    if (L->head == NULL) {
        printf("链表为空!");
    }
    else {
        p = L->head;
        printf("%4d", p->data);
        p = p->next;
        while (p != L->head) {
            printf("%4d", p->data); 
            p = p->next;
        }
    }
    printf("\n");
    printf("长度为: %d\n", L->Length);
    printf("打印完毕! \n\n");
}


//读者线程
unsigned _stdcall ReaderThread(void* arg) {
    Readarg* RA;
    RA = (Readarg*)arg;
    int e;
    WaitForSingleObject(rmutex, INFINITE);//等待互斥量信号
    //printf("Reader wait for rmutex\n");
    readcount++;
    if (readcount == 1) {
        WaitForSingleObject(mutex, INFINITE);//等待信号量信号
        //printf("Reader wait for mutex\n");
    }
    ReleaseMutex(rmutex);//释放互斥量信号
    //printf("Reader release rmutex\n");
    e = RA->e;
    printf("查找操作: 读者%d开始查找%d\n", th_find, e);
    Find(RA->List, e);
    th_find++;
    WaitForSingleObject(rmutex, INFINITE);
    //printf("Reader wait for rmutex\n");
    readcount--;
    if (readcount == 0) {
        ReleaseSemaphore(mutex, 1, NULL);//释放信号量信号
        //printf("Reader release mutex\n");
    }
    ReleaseMutex(rmutex);
    //printf("Reader release rmutex\n");
    return 0;
}

//写者线程
unsigned _stdcall WriterThread(void* arg) {
    Writearg* WA;
    WA = (Writearg*)arg;
    int f, index, e;
    f = WA->flag; 
    index = WA->index; 
    e = WA->e;
    if (f) {
        WaitForSingleObject(mutex,INFINITE);
        //printf("Writer wait for mutex\n");
        printf("插入操作:写者%d开始在第%d位置插入元素%d\n", th_insert, index, e);
        Insert(WA->List, index, e);
        th_insert++;
        Print(WA->List);
        ReleaseSemaphore(mutex, 1, NULL);
        //printf("Writer release mutex\n");
    }
    else {
        WaitForSingleObject(mutex, INFINITE);
        //printf("Writer wait for mutex\n");
        printf("删除操作:写者%d开始删除第%d个位置\n", th_erase, index);
        Erase(WA->List, index);
        th_erase++;
        Print(WA->List);
        ReleaseSemaphore(mutex, 1, NULL);
        //printf("Writer release mutex\n");
    }
    return 0;
}


//主函数
int main()
{
    printf("Operation System Curriculum Design\n");
    printf("************************\n");
    printf("1.测试程序\n");
    printf("2.退出\n");
    printf("3.test_temp\n");
    printf("************************\n");
    printf("Please choose 1 or 2:");
    char choice;
    do {
        choice = (char)getchar();
    } while (choice != '1' && choice != '2' && choice != '3');
    if (choice == '2')
        return 0; 
    else if (choice == '3')
    {
        /*int getsum;
        scanf("%d", &getsum);
        printf("%d\n", getsum);*/
        printf("test begin~\n");
        DLL L;
        L = (DuLinkList*)malloc(sizeof(DuLinkList));
        InitList(L);
        Print(L);
        Clear(L);
        Print(L);
        printf("test over~\n");
    }
    else {
        HANDLE hThread[sum];
        unsigned threadID[sum];
        int i, j;
        DLL L;
        L = (DuLinkList*)malloc(sizeof(DuLinkList)); 
        InitList(L);
        Print(L);
        rmutex = CreateMutex(NULL, FALSE, NULL);//创建互斥量rmutex
        mutex = CreateSemaphore(NULL, 1, 1, NULL);//创建信号量mutex

        srand((int)time(0)); 
        for (i = 0; i < sum; i++) {
            j = random(3);//0为查找，1为插入，2为删除
            if (j == 0) {
                Readarg* RA = new Readarg[1]; 
                RA[0].List = L;
                RA[0].e = random(earea);
                //创建读者线程
                hThread[i] = (HANDLE)_beginthreadex(NULL, 0, ReaderThread, (void*)&RA[0], 0, &threadID[i]);
            }
            else {
                Writearg* WA = new Writearg[2];
                WA[0].List = L;
                WA[0].index = random(lsarea);
                WA[0].e = random(earea);
                WA[0].flag = insert;
                WA[1].List = L;
                WA[1].index = random(lsarea); 
                WA[1].e = 0;
                WA[1].flag = erase;
                if (j == 1) {
                    //创建写者线程（插入）
                    hThread[i] = (HANDLE)_beginthreadex(NULL, 0, WriterThread, (void*)&WA[0], 0, &threadID[i]);
                }else {
                    //创建写者线程（删除)
                    hThread[i] = (HANDLE)_beginthreadex(NULL, 0, WriterThread, (void*)&WA[1], 0, &threadID[i]);
                }
                
            }
        }
        for (i = 0; i < sum; i++) {
            WaitForSingleObject(hThread[i], INFINITE);
            CloseHandle(hThread[i]);
        }
        CloseHandle(rmutex);
        CloseHandle(mutex);
        th = (th_find + th_insert + th_erase) - 3;
        printf("查找读者人数为:%d;插入写者人数为:%d;删除写者人数为: %d; 总人数为: %d \n", th_find - 1, th_insert - 1, th_erase - 1, th);
        Clear(L);
        Print(L);
        printf("test over\n");
        return 0;
    }
}
