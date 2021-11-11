#include "graph_editor.h"
#include "imgui_window_sdl.h"
#include "imgui_extras.h"
#include "imfilebrowser.h"
#include "thread.h"
#include "timer.h"
#include <GL/glew.h>
#include <math.h>
#include "processing-lib/ocio_processing.h"
#include "socket_notifier.h"

#include "scope-lib/waveform_monitor.h"
#include "processing-lib/lens_correction.h"
#include "raw-lib/mlv_video.h"
#include "dng_convert.h"

#include "texture2D.h"

class TestWidget : public Widget
{
	waveformMonitor _wfm;
	TextureRGBA16F* _tex;

	int _cf=0;
	float _idt_mat[9];
	Lens_correction *_lc;
	imgui_addons::ImGuiFileBrowser _fbw;
public:
	Mlv_video _video;
	TestWidget(Window_SDL* win) : Widget(win, "TestWidget"), _wfm(256,256), _video("/storage/VIDEO/MISC/MLV/M04-1713.MLV")
	{
		//set_maximized(true);
		set_position(50, 50);
		set_size(500,500);
		//set_resizable(false);
		set_titlebar(false);
		uint32_t size;
		int w = _video.resolution_x();
		int h = _video.resolution_y();

		_tex = new TextureRGBA16F(GL_RGB, GL_UNSIGNED_SHORT, w, h, NULL);

      /* _video.set_lens_params(_video.get_camera_name(), _video.get_lens_name(),
						   _video.get_aperture(), _video.get_focal_dist(),
						   _video.get_focal_length(), false, false);
 */
      _video.get_frame_as_gl_texture(10, *_tex);
	}

   void reload(){
      _video.get_frame_as_gl_texture(10, *_tex);
   }

	~TestWidget(){
	}

	void draw() override {

      if (_video.need_refresh()){
         _video.get_frame_as_gl_texture(10, *_tex);
      }

		ImGui::SetNextItemWidth(1000);
		ImGui::BeginGroup();
		ImVec2 winsize = ImGui::GetWindowSize();
		float ratio = 9.0/16.0;
		//ImGui::SetCursorPos(ImVec2(-500,-500));
		bool open = ImGui::Button("Test", ImVec2(100,20));
		ImGui::Image((void*)(unsigned long)_tex->get_gltex(), ImVec2(winsize.x,int((float)winsize.x*ratio)));
      ImGui::Image((void*)(unsigned long)_video.get_waveform_texture(), ImVec2(512, 256));
		ImGui::EndGroup();
		// Same name for openpopup and showFileDialog....
		if(open)ImGui::OpenPopup("Open File");
		_fbw.showFileDialogPopup("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 300), ".png,.jpg,.bmp");

      ImGui::ShowDemoWindow();
	}
};

class RawInfoWidget : public Widget
{
   Video_base* _video;
   int _sel_chroma  = 0;
   int _sel_camera_model  = 0;
   int _sel_lens_model  = 0;
   std::vector<std::string> _camera_models;
   std::vector<std::string> _lens_models;

public:
   RawInfoWidget(Window_SDL* win, Video_base* video) : Widget(win, "RawInfo"), _video(video)
   {
	   set_size(400,500);
      set_titlebar(false);
      ChildWidget* test = new ChildWidget(this, "Test1", false, 50,0);
      ChildWidget* test2 = new ChildWidget(this, "Test2", true);
      auto_set_lens();
   }

   void auto_set_lens(){
      std::string camera = _video->get_camera_name();
      std::string lens = _video->get_lens_name();

      _camera_models = Lens_correction::get_camera_models();    
      _lens_models = Lens_correction::get_lens_models(camera);
      std::vector<std::string>::iterator cam_it = std::find(_camera_models.begin(), _camera_models.end(), camera);
      std::vector<std::string>::iterator lens_it = std::find(_lens_models.begin(), _lens_models.end(), lens);
      if (cam_it == _camera_models.end()){
         _sel_camera_model = -1;
      } else {
         _sel_camera_model = cam_it - _camera_models.begin();
      }

      if (lens_it == _lens_models.end()){
         _sel_lens_model = -1;
      } else {
         _sel_lens_model =  lens_it - _lens_models.begin();
      }
   }

   std::string get_selected_camera(){
      if (_sel_camera_model >= _camera_models.size()){
         return "";
      }
      return _camera_models[_sel_camera_model];
   }

   std::string get_selected_lens(){
      if (_sel_lens_model >= _lens_models.size()){
         return "";
      }
      return _lens_models[_sel_lens_model];
   }

   void draw() override {
      static bool param1 = true;
      
      if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
      {
         if (ImGui::BeginTabItem("Raw info"))
         {
            ImGui::BeginTable("RawTable", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV);
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,   0.0f, 0);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,   0.0f, 1);
            ImGui::TableNextColumn();ImGui::Text("Camera");
            ImGui::TableNextColumn();ImGui::Text(_video->get_camera_name().c_str());
            ImGui::TableNextColumn();ImGui::Text("Lens");
            ImGui::TableNextColumn();ImGui::Text(_video->get_lens_name().c_str());
            ImGui::TableNextColumn();ImGui::Text("Focal length");
            ImGui::TableNextColumn();ImGui::Text("%.1f mm", _video->get_focal_length());
            ImGui::TableNextColumn();ImGui::Text("Focal distance");
            ImGui::TableNextColumn();ImGui::Text("%.1f m", _video->get_focal_dist());
            ImGui::TableNextColumn();ImGui::Text("Aperture");
            ImGui::TableNextColumn();ImGui::Text("f/%.1f", _video->get_aperture());
            ImGui::TableNextColumn();ImGui::Text("Frame #");
            ImGui::TableNextColumn();ImGui::Text("%i", _video->get_num_frames());
            ImGui::TableNextColumn();ImGui::Text("Frame rate");
            ImGui::TableNextColumn();ImGui::Text("%.2f/s", _video->fps());
            ImGui::TableNextColumn();ImGui::Text("Resolution");
            ImGui::TableNextColumn();ImGui::Text("%ix%i px", _video->resolution_x(), _video->resolution_y());
            ImGui::EndTable();
            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Raw tweaks"))
         {
            bool raw_changed = false;
            ImGui::BeginTable("Tweak table", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV);

            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,   0.0f, 0);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,   0.0f, 1);

            ImGui::TableNextColumn();ImGui::Text("Fix focus pixels");
            ImGui::TableNextColumn();raw_changed |= ImGui::Checkbox("##focuspixel", &param1);

            ImGui::TableNextColumn();ImGui::Text("Chroma smooth");
            ImGui::TableNextColumn();raw_changed |= ImGui::Combo("##chromasmooth", &_sel_chroma, "[None]\0[2 x 2]\0[3 x 3]\0[4 x 4]\0[5 x 5]\0");
            ImGui::EndTable();
            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Lens"))
         {
            static bool correct_expo = false, correct_distort = false;
            bool lens_changed = false;
            if (ImGui::Button("Auto set")){
               auto_set_lens();
            }
            ImGui::BeginTable("Lens table", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV);

            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed,   0.0f, 0);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch,   0.0f, 1);

            ImGui::TableNextColumn();ImGui::Text("Exposition correct");
            ImGui::TableNextColumn();bool lens_expo_changed = ImGui::Checkbox("##expocorrect", &correct_expo);

            ImGui::TableNextColumn();ImGui::Text("Distortion correct");
            ImGui::TableNextColumn();bool lens_distort_changed = ImGui::Checkbox("##distortcorrect", &correct_distort);

            std::vector<std::string> camera_models = Lens_correction::get_camera_models();
            ImGui::TableNextColumn();ImGui::Text("Camera model");
            ImGui::TableNextColumn();lens_changed |= ImGui::Combo("##cameramodel", &_sel_camera_model, vector_getter, static_cast<void*>(&camera_models), camera_models.size());

            std::vector<std::string> lens_models = Lens_correction::get_lens_models(camera_models[_sel_camera_model]);
            
            ImGui::TableNextColumn();ImGui::Text("Lens model");
            ImGui::TableNextColumn();lens_changed |= ImGui::Combo("##lensmodel", &_sel_lens_model, vector_getter, static_cast<void*>(&lens_models), lens_models.size());

            ImGui::EndTable();
            ImGui::EndTabItem();

            if (lens_changed || lens_expo_changed || lens_distort_changed){
               _video->set_lens_params(get_selected_camera(), get_selected_lens(), _video->get_aperture(), _video->get_focal_dist(), _video->get_focal_length(), correct_expo, correct_distort);
            }
         }
         
         ImGui::EndTabBar();
      }
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
	//window2->set_monitor_lut("/home/cedric/Documents/ACES-OCIO/luts/Z27_sRGB.cube");
	window2->set_minimum_window_size(800,600);
	TestWidget* testwid = new TestWidget(window2);
   RawInfoWidget* rawinfo = new RawInfoWidget(window2, &testwid->_video);
	ImFont* font = app->load_font("/home/cedric/Documents/FONTS/DroidSans.ttf", 16.f);
	//ImFont* font2 = app->load_font("/home/cedric/Documents/FONTS/DroidSans-Bold.ttf", 16.f);
	//Window_SDL* window = app->create_new_window("demo", 900, 1000);
	app->run();
    return 0;
}
