#pragma once

#include "imgui_window_sdl.h"
#include "callback.h"

struct fb_impl;

class FileBrowserWidget : public Widget
{
	fb_impl* _impl;
	UserEvent _file_selected_event;
	bool _show;
	std::string _file_types;
public:
	FileBrowserWidget(Window_SDL* win, std::string name, std::string file_types = "*.MLV");
	virtual ~FileBrowserWidget();

	UserEvent& get_selected_event(){return _file_selected_event;}
	std::string get_filename();
	virtual void draw() override;
};
