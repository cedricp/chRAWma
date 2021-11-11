#pragma once

#include <string>
#include <vector>
#include "../processing-lib/ocio_processing.h"
#include "../scope-lib/waveform_monitor.h"

class Lens_correction;

class Video_base
{
    float _idt[9];
    Lens_correction* _lens_op;
    bool _need_refresh;
    OCIO_processing* _input_processor;
    OCIO_processing* _output_processor;
    waveformMonitor* _waveform_monitor;
    GLuint _wf_tex;
protected:
    virtual uint16_t* get_raw_buffer(uint32_t frame, float idt[9]) = 0;
    virtual void free_buffer(){};
public:
    Video_base();
    virtual ~Video_base();

    void get_frame_as_gl_texture(uint32_t frame, Texture2D& texture);

	virtual GLenum gl_pixel_type() = 0;
	virtual GLenum gl_pixel_format() = 0;
    virtual std::string get_camera_name() = 0;
	virtual std::string get_lens_name() = 0;

	virtual float get_focal_length() = 0;
	virtual float get_focal_dist() = 0;
	virtual float get_aperture() = 0;
    virtual uint32_t get_num_frames() = 0;
    virtual	uint32_t resolution_x() = 0;
	virtual uint32_t resolution_y() = 0;

    void set_lens_params(const std::string camera, const std::string lens, float aperture, float focus_distance, float focus_length, bool do_expo, bool do_distort);
    void enable_distortion_correction(bool e);
    void enable_exposition_correction(bool e);

    GLuint get_waveform_texture(){return _wf_tex;}

    float* get_idt(){return _idt;}

    bool need_refresh(){bool status =  _need_refresh;_need_refresh = false; return status;}

    virtual float fps() = 0;
};