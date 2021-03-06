/*
 * PCB.cpp
 *
 *  Created on: Apr 16, 2021
 *      Author: OS1
 */

#include "PCB.h"
#include "System.h"
#include <dos.h>
#include "Thread.h"
#include "PCBStack.h"
#include "SCHEDULE.H"

void dispatch();

ID PCB::idCnt = 0;

void PCB::wrapper(){
	if(System::running != 0 && System::running->id != 0){

		(System::running)->myThread->run();
		(System::running)->state = PCB::FINISHED;

		System::lock();
		while(!System::running->waitingForMeStack->isEmpty()){
			volatile PCB* temp = System::running->waitingForMeStack->pop();
			temp->state = READY;
			Scheduler::put((PCB*)temp);
		}
		System::unlock();
	}

	dispatch();
}

PCB::PCB(StackSize stackSize, Time timeSlice, Thread* thread) {
	System::lock();
	this->id = idCnt++;
	this->myThread = thread;
	this->stackSize = stackSize;
	this->timeSlice = timeSlice;
	this->waitingForMeStack = new PCBStack;
	this->isWaitingForSem = 0;
	this->semWaitTimeLeft = 0;
	this->unblockedBySignal = 0;
	if(thread != 0){
		this->stack = new unsigned[stackSize];
	#ifndef BCC_BLOCK_IGNORE
		this->stack[stackSize - 1] = 0x200;

		this->stack[stackSize - 2] = FP_SEG(&(PCB::wrapper));
		this->stack[stackSize - 3] = FP_OFF(&(PCB::wrapper));

		this->ss = FP_SEG(stack + stackSize - 12);
		this->sp = FP_OFF(stack + stackSize - 12);
		this->bp = FP_OFF(stack + stackSize - 12);
	#endif
		this->state = CREATED;
	}
	else {
		this->stack = 0;
		this->state = READY;
	}
	System::listOfPCB->push(this);
	System::unlock();
}

PCB::~PCB() {
	waitToComplete();
	System::lock();
	System::listOfPCB->removeMe(this);

	if(stack != 0)
		delete [] ((unsigned*)stack);
	System::unlock();

}


void PCB::start(){
	state = READY;
	Scheduler::put(this);
}


void PCB::waitToComplete(){
	System::lock();
	if(this->state != FINISHED){
		System::running->state = BLOCKED;
		this->waitingForMeStack->push(System::running);
		System::unlock();
		dispatch();
	}
	else
		System::unlock();
}


Thread* PCB::getThreadById(ID id){
	System::lock();
	volatile PCB *ret = System::listOfPCB->find(id);
	System::unlock();

	return ret->myThread;
}





int PCB::tick() volatile {
	if(this->state == BLOCKED && isWaitingForSem && semWaitTimeLeft > 0){
		semWaitTimeLeft--;
		if(semWaitTimeLeft == 0){
			state = READY;
			unblockedBySignal = 0;
			Scheduler::put((PCB*)this);
			return 1;
		}
	}
	return 0;
}
