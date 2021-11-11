#include "video.h"
#include "../processing-lib/lens_correction.h"

Video_base::Video_base()
{
    _lens_op = NULL;
    _need_refresh = false;

    _input_processor = new OCIO_processing("ACES - ACES2065-1", "ACES - ACEScg");
    _output_processor = new OCIO_processing("ACEScg", "Output - sRGB");
    _waveform_monitor = new waveformMonitor(512, 256);
    _wf_tex = 0;
}

Video_base::~Video_base()
{
    if (_lens_op){
        delete _lens_op;
        _lens_op = NULL;
    }
    delete _input_processor;
    delete _output_processor;
    delete _waveform_monitor;
}

void Video_base::get_frame_as_gl_texture(uint32_t frame, Texture2D& texture)
{
    uint16_t* framebuffer = get_raw_buffer(frame, _idt);
    texture.update_buffer(0, 0, resolution_x(), resolution_y(), GL_RGB, GL_UNSIGNED_SHORT, framebuffer);
    free_buffer();

    if (_lens_op){
        _lens_op->apply_correction(texture);
    }

    _input_processor->process(texture, _idt);

    _wf_tex = _waveform_monitor->compute(texture).get_gltex();
}

void Video_base::set_lens_params(const std::string camera, const std::string lens, float aperture, float focus_distance, float focus_length, bool do_expo, bool do_distort)
{
    if (_lens_op){
        delete _lens_op;
        _lens_op = NULL;
    }

    if (!do_expo && !do_distort){
        return;
    }

    _lens_op = new Lens_correction(camera, lens, resolution_x(), resolution_y(), aperture, focus_distance, focus_length, do_expo, do_distort);
    if(!_lens_op->valid()){
        delete _lens_op;
        _lens_op = NULL;
    }
    _need_refresh = true;
}
