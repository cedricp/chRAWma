#include "imgui_window_sdl.h"
#include "imfilebrowser.h"
#include "graph_editor.h"
#include "thread.h"
#include "timer.h"
#include <GL/glew.h>
#include <math.h>
#include "processing-lib/ocio_processing.h"
#include "socket_notifier.h"

#include "scope-lib/waveform_monitor.h"
#include "processing-lib/lens_correction.h"
#include "raw-lib/libmlv.h"

#include "texture2D.h"

template <typename T, std::size_t N>
struct Array
{
   T data[N];
   const size_t size() const { return N; }

   const T operator [] (size_t index) const { return data[index]; }
   operator T* () {
      T* p = new T[N];
      memcpy(p, data, sizeof(data));
      return p;
   }
};

template <typename T, typename ... U> Array(T, U...)->Array<T, 1 + sizeof...(U)>;


struct GraphEditorDelegate : public GraphEditor::Delegate
{
   bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override
   {
      return true;
   }

   void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override
   {
      mNodes[nodeIndex].mSelected = selected;
   }

   void MoveSelectedNodes(const ImVec2 delta) override
   {
      for (auto& node : mNodes)
      {
         if (!node.mSelected)
         {
            continue;
         }
         node.x += delta.x;
         node.y += delta.y;
      }
   }

   virtual void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override
   {
   }

   void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override
   {
      mLinks.push_back({ inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex });
   }

   void DelLink(GraphEditor::LinkIndex linkIndex) override
   {
      mLinks.erase(mLinks.begin() + linkIndex);
   }

   void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override
   {
      drawList->AddLine(rectangle.Min, rectangle.Max, IM_COL32(0, 0, 0, 255));
      drawList->AddText(rectangle.Min, IM_COL32(255, 128, 64, 255), "Draw");
   }

   const size_t GetTemplateCount() override
   {
      return sizeof(mTemplates) / sizeof(GraphEditor::Template);
   }

   const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override
   {
      return mTemplates[index];
   }

   const size_t GetNodeCount() override
   {
      return mNodes.size();
   }

   const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override
   {
      const auto& myNode = mNodes[index];
      return GraphEditor::Node
      {
          myNode.name,
          myNode.templateIndex,
          ImRect(ImVec2(myNode.x, myNode.y), ImVec2(myNode.x + 200, myNode.y + 200)),
          myNode.mSelected
      };
   }

   const size_t GetLinkCount() override
   {
      return mLinks.size();
   }

   const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override
   {
      return mLinks[index];
   }

   // Graph datas
   static const inline GraphEditor::Template mTemplates[] = {
       {
           IM_COL32(160, 160, 180, 255),
           IM_COL32(100, 100, 140, 255),
           IM_COL32(110, 110, 150, 255),
           1,
           Array{"MyInput"},
           nullptr,
           2,
           Array{"MyOutput0", "MyOuput1"},
           nullptr
       },

       {
           IM_COL32(180, 160, 160, 255),
           IM_COL32(140, 100, 100, 255),
           IM_COL32(150, 110, 110, 255),
           3,
           nullptr,
           Array{ IM_COL32(200,100,100,255), IM_COL32(100,200,100,255), IM_COL32(100,100,200,255) },
           1,
           Array{"MyOutput0"},
           Array{ IM_COL32(200,200,200,255)}
       }
   };

   struct Node
   {
      const char* name;
      GraphEditor::TemplateIndex templateIndex;
      float x, y;
      bool mSelected;
   };

   std::vector<Node> mNodes = {
       {
           "My Node 0",
           0,
           0, 0,
           false
       },

       {
           "My Node 1",
           0,
           400, 0,
           false
       },

       {
           "My Node 2",
           1,
           400, 400,
           false
       }
   };

   std::vector<GraphEditor::Link> mLinks = { {0, 0, 1, 0} };
};

class TestWidget : public Widget
{
	waveformMonitor _wfm;
	TextureRGBA16F* _tex;

	int _cf=0;
	float _idt_mat[9];
	Mlv_video _video;
	OCIO_processing _aces_proc;
	OCIO_processing _aces_proc2;
	Lens_correction *_lc;
	imgui_addons::ImGuiFileBrowser _fbw;
public:
	TestWidget(Window_SDL* win) : Widget(win, "TestWidget"), _wfm(256,256), _video("/storage/VIDEO/MISC/MLV/M04-1713.MLV"),
								_aces_proc("ACES - ACEScg", "Output - sRGB"), _aces_proc2("ACES - ACES2065-1", "ACES - ACEScg")
	{
		//set_maximized(true);
		set_position(50, 50);
		set_size(500,500);
		//set_resizable(false);
		set_titlebar(false);

		uint32_t size;
		uint16_t *frame = _video.get_raw_processed_buffer(_cf++, _idt_mat);
		int w = _video.resolution_x();
		int h = _video.resolution_y();

		_tex = new TextureRGBA16F(GL_RGB, GL_UNSIGNED_SHORT, _video.resolution_x(), _video.resolution_y(), frame);

		_lc = new Lens_correction(_video.get_camera_name(), _video.get_lens_name(),
						   _video.resolution_x(), _video.resolution_y(),
						   _video.get_aperture(), _video.get_focal_dist(),
						   _video.get_focal_length());

		_video.free_buffer();

		_lc->apply_correction(_tex->get_gltex());

		_aces_proc2.process(_tex->get_gltex(), _video.resolution_x(), _video.resolution_y(), _idt_mat);
		_aces_proc.process(_tex->get_gltex(), _video.resolution_x(), _video.resolution_y());

	}

	~TestWidget(){
	}

	void draw() override {

		ImGui::SetNextItemWidth(1000);
		ImGui::BeginGroup();
		bool open = ImGui::Button("Test", ImVec2(100,20));
		ImGui::Image((void*)(unsigned long)_tex->get_gltex(), ImVec2(1000,600));
		ImGui::EndGroup();
		// Same name for openpopup and showFileDialog....
		if(open)ImGui::OpenPopup("Open File");
		_fbw.showFileDialogPopup("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(600, 300), ".png,.jpg,.bmp");

		static GraphEditor::Options options;
		static GraphEditorDelegate delegate;
		static GraphEditor::ViewState viewState;
	    static GraphEditor::FitOnScreen fit = GraphEditor::Fit_None;

        ImGui::Begin("Graph Editor", NULL, 0);
        if (ImGui::Button("Fit all nodes"))
        {
           fit = GraphEditor::Fit_AllNodes;
        }
        ImGui::SameLine();
        if (ImGui::Button("Fit selected nodes"))
        {
           fit = GraphEditor::Fit_SelectedNodes;
        }
        GraphEditor::Show(delegate, options, viewState, true, &fit);

        ImGui::End();

        //ImPlot::ShowDemoWindow();
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
	window2->set_monitor_lut("/home/cedric/Documents/ACES-OCIO/luts/Z27_sRGB.cube");
	window2->set_minimum_window_size(800,600);
	TestWidget* testwid = new TestWidget(window2);
	ImFont* font = app->load_font("/home/cedric/Documents/FONTS/DroidSans.ttf", 16.f);
	ImFont* font2 = app->load_font("/home/cedric/Documents/FONTS/DroidSans-Bold.ttf", 16.f);
	//Window_SDL* window = app->create_new_window("demo", 900, 1000);
	app->run();
    return 0;
}
