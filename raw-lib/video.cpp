#include "video.h"
#include <string.h>

Video_base::Video_base()
{
    _lens_op = NULL;
    _need_refresh = false;

    _input_processor = new OCIO_processor("ACES - ACES2065-1", "ACES - ACEScg");
    _output_processor = new OCIO_processor("ACEScg", "Output - sRGB");
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
    clear_cache();
}

void Video_base::clear_cache()
{
     for (int i = 0; i < _raw_buffers.size(); ++i){
        free(_raw_buffers[i]);
        _raw_buffers[i] = 0;
    }   
}

void Video_base::get_frame_as_gl_texture(uint32_t frame, Texture2D& texture)
{
    if (_raw_buffers.empty()){
        _raw_buffers.resize(frame_count(), NULL);
    }

    uint16_t* framebuffer = NULL;
    if (_raw_buffers[frame] == NULL){
        framebuffer = raw_buffer(frame, _idt, _rawinfo);
        size_t buffersize = raw_resolution_x() * raw_resolution_y() * 3 * sizeof(uint16_t);
        _raw_buffers[frame] = (uint16_t*)malloc(buffersize);
        memcpy(_raw_buffers[frame], framebuffer, buffersize);
        free_buffer();  
    }
    
    framebuffer = _raw_buffers[frame];

    if (sampling_factor_y() != sampling_factor_x()){
        TextureRGBA16F original_texture_buffer(GL_RGB, GL_UNSIGNED_SHORT, raw_resolution_x(), raw_resolution_y(), framebuffer);
        texture.init(GL_RGB, GL_HALF_FLOAT, resolution_x(), resolution_y());
        _scale_processor.process(original_texture_buffer, texture);
    } else {
        texture.update_buffer(0, 0, raw_resolution_x(), raw_resolution_y(), GL_RGB, GL_UNSIGNED_SHORT, framebuffer);
    }

    if (_lens_op){
        _lens_op->apply_correction(texture);
    }

    // Apply headroom
    float idt[9];
    for (int i = 0; i < 9; ++i){
        idt[i] = _idt[i] * _rawinfo.headroom;
    }
    
    TextureRGBA16F blur_texture(GL_RGB, GL_UNSIGNED_SHORT, resolution_x(), resolution_y());
    _blur_processor.process(texture, blur_texture, _blur);
    texture.swap(blur_texture);

    _input_processor->process(texture, idt);
    _output_processor->process(texture);

    _wf_tex = _waveform_monitor->compute(texture).gl_texture();
}

void Video_base::set_lens_params(const std::string camera, const std::string lens, float crop_factor, float aperture, float focus_distance, float focus_length, bool do_expo, bool do_distort)
{
    if (_lens_op){
        delete _lens_op;
        _lens_op = NULL;
    }

    if (!do_expo && !do_distort){
         _need_refresh = true;
        return;
    }
    int x,y;
    sensor_resolulion(x,y);
    float sensor_ratio = (float)x / float(y);
    _lens_op = new Lens_correction(camera, lens, resolution_x(), resolution_y(),
                                   crop_factor, aperture, focus_distance, focus_length,
                                   sensor_ratio, do_expo, do_distort);
    if(!_lens_op->valid()){
        delete _lens_op;
        _lens_op = NULL;
    }
    _need_refresh = true;
}

uint32_t Video_base::resolution_x()
{
    if (sampling_factor_x() == sampling_factor_y()){
        return raw_resolution_x();
    }
    float raw_ratio = (float)sampling_factor_x() / (float)sampling_factor_y();
    return (float)raw_resolution_x() * raw_ratio;
}

uint32_t Video_base::resolution_y()
{
    return raw_resolution_y();
}