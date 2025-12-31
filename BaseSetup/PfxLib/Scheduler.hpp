/*
 *  Scheduler.h
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#pragma		once

#include "Unit.hpp"

typedef void (*voidfun)(void* args);

class Task
{
private:
	Task*         link;
	unsigned long execTime;
	int           period;
    double        beatPeriod;
	voidfun       function;
public:
	void*         arguments;

public:
	Task(void);

	friend class Scheduler;
};

class Scheduler : public Unit
{
public:
    double        MM;

private:
	enum Sizes	{ kWaitQueueSize = 16384 };

	int			taskTableSize;
	long		lastWaitingTime;
	Task*		taskTable;
	Task**		waitQueueHeads;
	Task**		waitQueueTails;
	Task*     	readyQueueHeads;
	Task*     	readyQueueTails;
	Task*		freeTasks;

    unsigned long sampleCount;
    double        samplesPerMsec;
    bool          useBeats;

public:
				Scheduler(int maxTasks = 16384);
				~Scheduler(void);
	void		AbortTask(Task* task);
    unsigned long CurrentTime(void) { return sampleCount; }
    double      CurrentTimeMS(void) { return sampleCount / samplesPerMsec; }
    Task*       ScheduleTask       (long time, int per, void (*fun)(void* empty));
	Task*		ScheduleTask       (long time, int per, void (*fun)(void* args), void* args);
    Task*       ScheduleBeatTask   (double beatPart, double per, void (*fun)(void* empty));
    Task*       ScheduleBeatTask   (double beatPart, double per, void (*fun)(void* args), void* args);
    Task*		ScheduleTaskSamples(long time, int per, void (*fun)(void* args), void* args);
    void        SetMM(double newMM);
	void		Tick(long now);
    void        Sample(int sNo);

private:
	void		ClearQueues(void);
	void		DisposeTask(Task* task);
	void		ExecuteTask(Task* task);
	Task* 		NextReadyTask(void);
	Task* 		NextWaitingTask(long time);
	void		ReadyTask(Task* task);
	void		RescheduleTask(Task* task);
	void		WaitTask(Task* task);
};
