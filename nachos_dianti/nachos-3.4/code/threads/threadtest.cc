// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"
#include "synchlist.h"

// testnum is set in main.cc
int testnum = 1;
int cnt = 0;

#define N 10 //缓冲区个数
Semaphore* mutex = new Semaphore((char *)"mutex", 1); //互斥信号量，控制对临界区的访问
Semaphore* empty = new Semaphore((char *)"empty", N); //空缓冲区个数
Semaphore* full = new Semaphore((char *)"full", 0);//满缓冲区个数
List* buffer;//缓冲区

Condition* notempty = new Condition((char *)"notempty");
Condition* notfull = new Condition((char *)"notfull");
SynchList* cbuffer;
Lock* mesa = new Lock((char *)"mesa");

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** tid %d priority %d looped %d times\n",
     currentThread->gettid(), currentThread->getpriority(), num);
        //currentThread->Yield();
    }
}

void producer(int order) {
    empty->P();//看是否已满
    mutex->P();//临界区保护
    buffer->Append((void *)order);
    printf("%d produced by %d\n", order, currentThread->gettid());
    mutex->V();
    full->V();
}

void consumer() { 
    full->P();//看是否为空
    mutex->P();//临界区保护
    int item = buffer->Remove();
    printf("%d consumed by %d\n", item, currentThread->gettid());
    mutex->V();
    empty->V();
}
void test_producer()
{
    for (int i = 0;i < 5;i++)
    {
        Thread *t = new Thread("producer forked");
        t->Fork(producer, (void*)i);
    }
}
void test_consumer()
{
    for (int i = 0;i < 10;i++)
    {
        Thread *t = new Thread("consumer forked");
        t->Fork(consumer, (void*)1);
    }
}



void append (int x)
{
    mesa->Acquire();
    while (cnt == N) notfull->Wait(mesa);
    cbuffer->Append((void *)x);
    printf("%d appended by %d\n", x, currentThread->gettid());
    cnt++;
    notempty->Signal(mesa);
    mesa->Release();
}
void take ()
{
    mesa->Acquire();
    while (cnt == 0) notempty->Wait(mesa);
    int x = cbuffer->Remove();
    printf("%d took by %d\n", x, currentThread->gettid());
    cnt--;
    notfull->Signal(mesa);
    mesa->Release();
}
void monitor_producer()
{
    for (int i = 0;i < 5;i++)
    {
        Thread *t = new Thread("producer forked");
        t->Fork(append, (void*)i);
    }
}
void monitor_consumer()
{
    for (int i = 0;i < 10;i++)
    {
        Thread *t = new Thread("consumer forked");
        t->Fork(take, (void*)1);
    }
}
//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    /*switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
    for(int i = 0;i < testnum;i++)
        ThreadTest2(i);
    printf("testnum%d\n",testnum);
	//printf("No test specified.\n");
	break;
    }*/
    buffer = new List();
    cbuffer = new SynchList();
    for (int i = 0;i < 2;i++)
    {
        /*
        test_producer();
        test_consumer();
        */
        monitor_producer();
        monitor_consumer();
    }
}

