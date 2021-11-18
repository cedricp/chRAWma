#pragma once

#include <string>
#include <vector>
#include "../processing-lib/ocio_processor.h"
#include "../processing-lib/scale_processor.h"
#include "../processing-lib/lens_correction.h"
#include "../processing-lib/blur_processor.h"
#include "../scope-lib/waveform_monitor.h"

class Lens_correction;

class Video_base
{
public:
    struct RawInfo {
        bool raw_tweak = true;
        bool fix_focuspixels = true;
        int32_t chroma_smooth = 0;
        int32_t temperature = -1;
        float headroom = 4.5;
        int interpolation = 0;
    };
private:
    float _idt[9];
    Lens_correction* _lens_op;
    bool _need_refresh;
    OCIO_processor* _input_processor;
    OCIO_processor* _output_processor;
    Scale_processor _scale_processor;
    Blur_processor _blur_processor;
    waveformMonitor* _waveform_monitor;
    GLuint _wf_tex;
    std::vector<uint16_t*> _raw_buffers;
protected:
    virtual uint16_t* get_raw_buffer(uint32_t frame, float idt[9], const RawInfo& ri) = 0;
    virtual void free_buffer(){};
    RawInfo _rawinfo;
public:
    float _blur = 1.0f;
    Video_base();
    virtual ~Video_base();

    RawInfo& raw_info(){return _rawinfo;}

    void get_frame_as_gl_texture(uint32_t frame, Texture2D& texture);
    void clear_cache();

    uint32_t resolution_x();
	uint32_t resolution_y();

	virtual GLenum gl_pixel_type() = 0;
	virtual GLenum gl_pixel_format() = 0;
    virtual std::string get_camera_name() = 0;
	virtual std::string get_lens_name() = 0;

	virtual float get_focal_length() = 0;
	virtual float get_focal_dist() = 0;
	virtual float get_aperture() = 0;
    virtual uint32_t get_num_frames() = 0;
    virtual	uint32_t raw_resolution_x() = 0;
	virtual uint32_t raw_resolution_y() = 0;
    virtual float get_crop_factor() = 0;
    virtual float get_final_crop_factor() = 0;
    virtual int get_pixel_binning_x() = 0;
    virtual int get_pixel_binning_y() = 0;
    virtual int get_sampling_factor_x() = 0;
    virtual int get_sampling_factor_y() = 0;
    virtual uint32_t get_iso() = 0;
    virtual uint32_t get_shutter() = 0;
    virtual int get_bpp() = 0;

    void set_lens_params(const std::string camera, const std::string lens, float crop_factor, float aperture, float focus_distance, float focus_length, bool do_expo, bool do_distort);
    void enable_distortion_correction(bool e);
    void enable_exposition_correction(bool e);

    GLuint get_waveform_texture(){return _wf_tex;}

    float* get_idt(){return _idt;}

    bool need_refresh(){bool status =  _need_refresh;_need_refresh = false; return status;}
    void set_dirty(){_need_refresh = true;}

    virtual float fps() = 0;
};