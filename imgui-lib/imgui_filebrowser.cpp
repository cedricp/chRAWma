#include "imgui_filebrowser.h"
#include <imfilebrowser.h>

struct fb_impl{
	imgui_addons::ImGuiFileBrowser fileDialog;
};

FileBrowserWidget::FileBrowserWidget(Window_SDL* win, std::string name, std::string file_types) :
									Widget(win, name, false), _show(true), _file_types(file_types){
	_impl = new fb_impl;
	set_size(500, 400);
	set_modal(true);
}

FileBrowserWidget::~FileBrowserWidget() {
	delete _impl;
}

void FileBrowserWidget::draw() {
	if (!_show)
		return;
	bool ok = _impl->fileDialog.showFileDialog("open", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(_sizex, _sizey), _file_types );
	if (ok){
		_file_selected_event.push();
		_show = false;
	}
}


std::string
FileBrowserWidget::get_filename()
{
	return _impl->fileDialog.selected_path;
}
