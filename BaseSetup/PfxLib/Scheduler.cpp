/*
 *  Scheduler.cpp
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "Scheduler.hpp"

/* Task constructors */
Task::Task(void)
{
	link		= nullptr;      // link to next Task
	execTime	= 0L;			// time to execute Task
	period		= 0;			// duration between Task executions
    beatPeriod  = 1.0;          // duration between executions in beats
	function	= nullptr;      // function to be called by Task
}

/* Scheduler constructor */
Scheduler::Scheduler(int maxTasks)
{
    MM              = 120.0;
    sampleCount     = 0L;
    samplesPerMsec  = samplingRate / 1000.0;
    useBeats        = false;

	taskTableSize   = maxTasks;
	taskTable	    = new Task[maxTasks];
	waitQueueHeads  = new Task*[kWaitQueueSize];
	waitQueueTails  = new Task*[kWaitQueueSize];
	lastWaitingTime = 0L;
	ClearQueues();
}

/* Scheduler deconstructor */
Scheduler::~Scheduler(void)
{
	delete [] waitQueueTails;
	delete [] waitQueueHeads;
	delete [] taskTable;
}

void Scheduler_NoOp(void* p)
{
	#pragma unused(p)
}

/* AbortTask: maintains the task in the queue, but ensures that it will have no effect */
void Scheduler::AbortTask(Task* task)
{
	if (task == nullptr) return;        // check for null task pointer
	task->function = Scheduler_NoOp;	// point to empty function
	task->period   = 0;					// make sure task does not recur
}

/* ClearQueues: empty all scheduler queues */
void Scheduler::ClearQueues(void)
{
	int   i;
	Task* task;

	freeTasks = task = taskTable;
    readyQueueHeads = nullptr;
    readyQueueTails = nullptr;
	for (i=0; i<kWaitQueueSize; i++)
		waitQueueHeads[i] = waitQueueTails[i] = nullptr;
	for (i=1; i<taskTableSize; i++)
    {
		task->link = &taskTable[i];		// link to next task in singly-linked list
		task = task->link;
	}
	task->link      = nullptr;          // last task has null link
    lastWaitingTime = 0L;
}

/* DisposeTask: move freeTask pointer back one task */
void Scheduler::DisposeTask(Task* task)
{
	task->link = freeTasks;
	freeTasks  = task;
}

/* ReadyTask: add the task to the appropriate queue */
void Scheduler::ReadyTask(Task* task)
{
    task->link = nullptr;                   // task at end of queue
	if (readyQueueHeads == nullptr)         // if ready queue is empty insert at head
    {
		readyQueueHeads = readyQueueTails = task;
	} else                                  // otherwise insert at tail
    {
		readyQueueTails->link = task;
		readyQueueTails       = task;
	}
}

/* WaitTask: add task to wait queue */
void Scheduler::WaitTask(Task* task)
{
	if (task == nullptr) return;

	long execTime = task->execTime;
	if (execTime <= lastWaitingTime)        // task is due
    {
		ReadyTask(task);					// run it
		return;
	}

	int   index = execTime%kWaitQueueSize;	// look at this moment in the queue
	Task* head  = waitQueueHeads[index];
	if (head == nullptr)
    {                                       // if empty
		task->link = nullptr;				// add this task
		waitQueueHeads[index] = waitQueueTails[index] = task;
		return;
	}
	
	if (head->execTime >= execTime)         // if first task in queue is later than this
    {
		task->link            = head;       // add task to the head
		waitQueueHeads[index] = task;
		return;
	}
	
	Task* tail = waitQueueTails[index];		// this task is later than head
	if (tail->execTime <= execTime)         // if tail is earlier than this
    {
		task->link = nullptr;				// add to tail
		waitQueueTails[index] = tail->link = task;
		return;
	}
	
	while (true)                            // task is later than head and earlier than tail
    {
		if (head->link == nullptr)          // if at end of head queue
        {
			head->link = task;				// add here
			task->link = nullptr;
			waitQueueTails[index] = task;	// store as the tail
			break;
		}
		else if (head->link->execTime >= execTime)
        {
			task->link = head->link;		// task is earlier than head link
			head->link = task;				// insert
			break;
		}
		else
			head = head->link;				// keep looking
	 }
}

/* ScheduleTask: Add task to wait queue */
Task* Scheduler::ScheduleTask(long time, int per, void (*fun)(void* empty))
{
    Task* task = freeTasks;                 // get unused task
    if (task == nullptr) return nullptr;    // return if none left
    
    freeTasks        = task->link;
    task->execTime   = time * samplesPerMsec + sampleCount;
    task->period     = per  * samplesPerMsec;
    task->function   = fun;
    task->arguments  = nullptr;
    WaitTask(task);                         // enter in wait queue
    
    return task;                            // return pointer to task
}

Task* Scheduler::ScheduleTask(long time, int per, voidfun fun, void* args)
{
    Task* task = freeTasks;                 // get unused task
	if (task == nullptr) return nullptr;	// return if none left

	freeTasks        = task->link;
	task->execTime   = time * samplesPerMsec + sampleCount;
	task->period     = per  * samplesPerMsec;
	task->function   = fun;
	task->arguments  = args;
	WaitTask(task);							// enter in wait queue

	return task;							// return pointer to task
}

/* ScheduleTask: Add task to wait queue */
Task* Scheduler::ScheduleBeatTask(double beatPart, double per, void (*fun)(void* empty))
{
    Task* task = freeTasks;                 // get unused task
    if (task == nullptr) return nullptr;    // return if none left
    
    double dtime = (60000.0 / MM);
    long   time  = static_cast<long>(dtime*beatPart);
    int    lper  = static_cast<int> (dtime*per);
    
    freeTasks        = task->link;
    task->execTime   = time * samplesPerMsec + sampleCount;
    task->period     = lper * samplesPerMsec;
    task->function   = fun;
    task->arguments  = nullptr;
    WaitTask(task);                         // enter in wait queue
    
    return task;                            // return pointer to task
}

Task* Scheduler::ScheduleBeatTask(double beatPart, double per, voidfun fun, void* args)
{
    Task* task = freeTasks;                 // get unused task
    if (task == nullptr) return nullptr;    // return if none left

    double dtime = (60000.0 / MM);
    long   time  = static_cast<long>(dtime*beatPart);
    int    lper  = static_cast<int> (dtime*per);
    
    freeTasks        = task->link;
    task->execTime   = time * samplesPerMsec + sampleCount;
    task->period     = lper * samplesPerMsec;
    task->function   = fun;
    task->arguments  = args;
    WaitTask(task);                            // enter in wait queue

    return task;                            // return pointer to task
}

Task* Scheduler::ScheduleTaskSamples(long time, int per, voidfun fun, void* args)
{
    Task* task = freeTasks;                 // get unused task
    if (task == nullptr) return nullptr;	// return if none left
    
    freeTasks        = task->link;
    task->execTime   = time;
    task->period     = per;
    task->function   = fun;
    task->arguments  = args;
    WaitTask(task);							// enter in wait queue
    
    return task;							// return pointer to task
}

/* RescheduleTask: Reinsert task in queue after period */
void Scheduler::RescheduleTask(Task* task)
{
	task->execTime += task->period;
	WaitTask(task);							// enter in wait queue
}

/* NextWaitingTask: search wait queue and return next task ready for execution */
Task* Scheduler::NextWaitingTask(long time)
{
	for (long j=lastWaitingTime; j<=time; j++)
    {
		int index  = j%kWaitQueueSize;
		Task* task = waitQueueHeads[index];
		if ((task != nullptr) && (task->execTime <= time))
        {
			if (nullptr == (waitQueueHeads[index] = task->link))
				waitQueueTails[index] = nullptr;
			lastWaitingTime = j;			// remember the last wait time
			return task;
		}
	}
	lastWaitingTime = time+1;				// didn't find anything
	return nullptr;
}

/* NextReadyTask: find next task ready for execution, if there is one */
Task* Scheduler::NextReadyTask(void)
{
    if (readyQueueHeads != nullptr)
    {
        Task* task = readyQueueHeads;
        readyQueueHeads = readyQueueHeads->link;
        return task;
    }
	return nullptr;
}

/* ExecuteTask: execute the task and reschedule or erase */
void Scheduler::ExecuteTask(Task* task)
{
	(*(task->function))(task->arguments);
	if (task->period > 0)
		RescheduleTask(task);				// reschedule at period
	else
		DisposeTask(task);					// erase the task
}

void Scheduler::SetMM(double newMM)
{
    MM = newMM;
}

/* Tick: execute all tasks whose time has come */
void Scheduler::Tick(long now)
{
	Task* task;

	while (nullptr != (task=NextWaitingTask(now)))
		ReadyTask(task);
	while (nullptr != (task=NextReadyTask()))
		ExecuteTask(task);
}

void Scheduler::Sample(int sNo)
{
    Tick(sampleCount++);
}
