#ifndef SOCKET_NOTIFIER_H
#define SOCKET_NOTIFIER_H

#include "callback.h"
#include "imgui_window_sdl.h"
#include <vector>

class SocketNotifier : public Event {
	std::vector<int> m_fds;
public:
	SocketNotifier(){
		App_SDL::get()->register_fd_event(this);
	}

	~SocketNotifier(){
		stop();
	}

	void add_fd(int fd){
		m_fds.push_back(fd);
	}

	void data_available(int which){
		if (m_callback)
			m_callback(this, (void*)(unsigned long)which, m_callback_data);
	}

	const std::vector<int>& get_fds(){
		return m_fds;
	}

	void stop(){
		App_SDL::get()->unregister_fd_event(this);
		m_callback = NULL;
		m_callback_data = NULL;
	}
};


#endif
