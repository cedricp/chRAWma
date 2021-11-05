#include "imgui_window_sdl.h"
#include "imgui_filebrowser.h"
#include "imfilebrowser.h"
#include "thread.h"
#include "timer.h"
#include <GL/glew.h>
#include <math.h>
#include "processing-lib/ocio_processing.h"
#include "socket_notifier.h"

#include "scope-lib/waveform_monitor.h"
#include "processing-lib/lens_correction.h"
#include "raw-lib/libmlv.h"

class TestWidget : public Widget
{
	waveformMonitor _wfm;
	GLuint _tex = 0;
	GLuint _uv_tex = 0;
	int _cf=0;
	float _idt_mat[9];
	Mlv_video _video;
	OCIO_processing _aces_proc;
	OCIO_processing _aces_proc2;
	Lens_correction *_lc;
	imgui_addons::ImGuiFileBrowser _fbw;
	//GLuint _inbuff, _outbuff;
public:
	TestWidget(Window_SDL* win) : Widget(win, "TestWidget"), _wfm(256,256), _video("/storage/VIDEO/MISC/MLV/M04-1713.MLV"),
								_aces_proc("ACES - ACEScg", "Output - sRGB"), _aces_proc2("ACES - ACES2065-1", "ACES - ACEScg")
	{
		//set_maximized(true);
		set_position(50, 50);
		set_size(500,500);
		set_resizable(false);
		set_titlebar(false);

		glGenTextures(1, &_tex);
		glBindTexture(GL_TEXTURE_2D, _tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _video.resolution_x(), _video.resolution_y(), 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		uint32_t size;
		uint16_t *frame = _video.get_raw_processed_buffer(_cf++, _idt_mat);
		int w = _video.resolution_x();
		int h = _video.resolution_y();

		glBindTexture(GL_TEXTURE_2D, _tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _video.resolution_x(), _video.resolution_y(), GL_RGB, GL_UNSIGNED_SHORT, frame);
		glBindTexture(GL_TEXTURE_2D, 0);


		_lc = new Lens_correction(_video.get_camera_name(), _video.get_lens_name(),
						   _video.resolution_x(), _video.resolution_y(),
						   _video.get_aperture(), _video.get_focal_dist(),
						   _video.get_focal_length());

		_video.free_buffer();

		_lc->apply_correction(_tex);

		_aces_proc2.process(_tex, _video.resolution_x(), _video.resolution_y(), _idt_mat);
		_aces_proc.process(_tex, _video.resolution_x(), _video.resolution_y());

	}

	~TestWidget(){
	}

	void draw() override {

		ImGui::SetNextItemWidth(200);
		ImGui::BeginGroup();
		bool open = ImGui::Button("Test", ImVec2(100,20));
		ImGui::Image((void*)_tex, ImVec2(300,100));
		ImGui::EndGroup();
		// Same name for openpopup and showFileDialog....
		if(open)ImGui::OpenPopup("Open File");
		_fbw.showFileDialogPopup("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 300), ".png,.jpg,.bmp");
	}
};

// Main code
int main(int, char**)
{
	App_SDL* app = App_SDL::get();
	std::string fpm_path = app->get_str_config("APP_PATH") + "/../pixel_maps";
	std::string ocio_path = app->get_str_config("APP_PATH") + "/../aces-config/config.ocio";
	setenv("OCIO", ocio_path.c_str(), 1);
	setenv("PIXELMAPSDIR", fpm_path.c_str(), 1);
	Window_SDL* window2 = app->create_new_window("OSC", 1200, 1000);
	window2->set_minimum_window_size(800,600);
	TestWidget* testwid = new TestWidget(window2);
	ImFont* font = app->load_font("/home/cedric/Documents/FONTS/DroidSans.ttf", 16.f);
	ImFont* font2 = app->load_font("/home/cedric/Documents/FONTS/DroidSans-Bold.ttf", 16.f);
	//Window_SDL* window = app->create_new_window("demo", 900, 1000);
	app->run();
    return 0;
}
