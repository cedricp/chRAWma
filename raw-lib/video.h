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
    enum Raw_type {
        RAW_MLV,
        RAW_DNG,
        RAW_EXR
    };
    struct RawInfo {
        bool raw_tweak = true;
        bool fix_focuspixels = true;
        int32_t chroma_smooth = 0;
        int32_t temperature = -1;
        float headroom = 4.5;
        int interpolation = 4;
        int highlight = 3;
        float crop_factor = 1.0f;
        float focal_length = 35.0f;
        std::string darkframe_file;
        bool darkframe_enable = true;
        bool darkframe_ok = false;
        std::string darkframe_error;
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
    std::vector<uint16_t*> _raw_buffers;
protected:
    virtual uint16_t* raw_buffer(uint32_t frame, float idt[9], const RawInfo& ri) = 0;
    virtual void free_buffer(){};
    RawInfo _rawinfo;
public:
    float _blur = 1.0f;
    float _blur_hv = 0.5f;
    Video_base();
    virtual ~Video_base();

    RawInfo& raw_settings(){return _rawinfo;}

    void get_frame_as_gl_texture(uint32_t frame, Texture2D& texture);
    void clear_cache();

    uint32_t resolution_x();
	uint32_t resolution_y();

    void set_display(std::string display);

    virtual Raw_type file_type() = 0;
	virtual GLenum gl_pixel_type() = 0;
	virtual GLenum gl_pixel_format() = 0;
    virtual std::string camera_name() = 0;
	virtual std::string lens_name() = 0;
    virtual std::string lens_name_by_id() = 0;

	virtual float focal_length() = 0;
	virtual float focal_dist() = 0;
	virtual float aperture() = 0;
    virtual uint32_t frame_count() = 0;
    virtual	uint32_t raw_resolution_x() = 0;
	virtual uint32_t raw_resolution_y() = 0;
    virtual float crop_factor() = 0;
    virtual float final_crop_factor() = 0;
    virtual int pixel_binning_x() = 0;
    virtual int pixel_binning_y() = 0;
    virtual int sampling_factor_x() = 0;
    virtual int sampling_factor_y() = 0;
    virtual uint32_t iso() = 0;
    virtual uint32_t shutter_speed() = 0;
    virtual int bpp() = 0;
    virtual void sensor_resolulion(int& x, int& y) = 0;

    void set_lens_params(const std::string camera, const std::string lens, float crop_factor, float aperture,
                        float focus_distance, float focus_length, float scale, bool do_expo, bool do_distort);
    void enable_distortion_correction(bool e);
    void enable_exposition_correction(bool e);

    float* get_idt(){return _idt;}

    bool need_refresh(){bool status =  _need_refresh;_need_refresh = false; return status;}
    void set_dirty(){_need_refresh = true;}

    virtual float fps() = 0;
};