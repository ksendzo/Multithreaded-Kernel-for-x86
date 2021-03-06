/*
 * Thread.cpp
 *
 *  Created on: Apr 16, 2021
 *      Author: OS1
 */

#include "Thread.h"
#include "PCB.h"
#include <dos.h>
#include <iostream.h>
#include "SCHEDULE.H"
#include <stdio.h>
#include "System.h"

Thread::Thread(StackSize stackSizeParam, Time timeSlice) {
	myPCB = new PCB(
				((stackSizeParam<maxStackSize)?stackSizeParam:maxStackSize) / sizeof(unsigned),
				timeSlice,
				this);
}

Thread::~Thread() {
	myPCB->waitToComplete();
	if(myPCB != 0){
		delete myPCB;
	}
}


void Thread::start(){
	if(myPCB != 0)
		((PCB*)myPCB)->start();
}


void Thread::waitToComplete(){
	myPCB->waitToComplete();
}


ID Thread::getId(){
	if(myPCB)
		return myPCB->id;
	else
		return -1;
}


ID Thread::getRunningId(){
	return System::running->id;
}

Thread* Thread::getThreadById(ID id){
	return PCB::getThreadById(id);
}


volatile PCB* Thread::getMyPCB() volatile {
	return (volatile PCB*)myPCB;
}








