#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <unistd.h>
#include <functional>
#include "callback.h"

class ThreadMutex {
	pthread_mutex_t m_mutex;
public:
	ThreadMutex(){
		pthread_mutex_init(&m_mutex, NULL);
	}
	~ThreadMutex(){
		pthread_mutex_destroy(&m_mutex);
	}
	bool lock(){
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	bool unlock(){
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
};

class Thread {
protected:
	pthread_t m_thread_id;

	volatile bool m_running;
	volatile bool m_pause;
	bool m_loop;
	UserEvent m_thread_exit_event;
	void* run();
public:
	Thread(bool loop);
	virtual ~Thread();
	void start();
	void* join();

	void usleep(unsigned long us);

	/*
	 * Main thread code
	 * If the code needs to loop, set loop=true in the constructor
	 */
	virtual void entry() = 0;

	virtual void on_exit_event(Event* sender_object, void* data);

	void stop(){
		m_running = false;
	}

	bool is_running(){
		return m_running;
	}

	DECLARE_STATIC_CALLBACK_METHOD(on_exit_event)
};

class ASyncTask : public Thread
{
public:
	ASyncTask() : Thread(false) {};
	virtual ~ASyncTask(){}
};

class ASyncLoopTask : public Thread
{
public:
	ASyncLoopTask() : Thread(true) {};
	virtual ~ASyncLoopTask(){}
	void pause(bool p){
		m_pause = p;
	}
	bool is_paused(){
		return m_pause;
	}
};

#endif
