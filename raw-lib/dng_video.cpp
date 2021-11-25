#include "dng_video.h"
#include "dng_convert.h"
#include <libraw/libraw.h>
#include <string.h>
#include <filesystem>

namespace fs = std::filesystem;

struct dng_imp{
    Dng_processor* processor = NULL;
    char* buffer = NULL;
    std::vector<std::string> filenames;
};

static uint8_t* file_to_mem(std::string filename, long* size = NULL)
{
    if(size){
        *size = 0;
    }

    FILE* infile = fopen(filename.c_str(), "rb");
    if(!infile){
        return NULL;
    }
    fseek(infile, 0L, SEEK_END);
    long numbytes = ftell(infile);

    fseek(infile, 0L, SEEK_SET);
    uint8_t* buffer = (uint8_t*)malloc(numbytes);
    if (buffer == NULL){
        return NULL;
    }

    fread(buffer, sizeof(char), numbytes, infile);
    fclose(infile);
    if(size){
        *size = numbytes;
    }
    return buffer;
}

Dng_video::Dng_video(std::string filename)
{
    _imp = new dng_imp;
    _imp->processor = new Dng_processor;

    fs::path path(filename);
    fs::path dir(path.parent_path());
    for (auto & filepath : fs::directory_iterator(dir)){
        printf("%s\n", filepath.path().extension().string().c_str());
        if (filepath.path().extension() == ".dng"){
            printf("%s\n", filepath.path().string().c_str());
            _imp->filenames.push_back(filepath.path().string());
        }
    }

    //_imp->filename = filename;

    long size = 0;
    uint8_t* buffer = file_to_mem(filename.c_str(), &size);
    if (!buffer){
        return;
    }
    _imp->processor->unpack(buffer, size);
    free(buffer);
}

Dng_video::~Dng_video()
{
    delete _imp->processor;
    delete _imp;
}

uint16_t* Dng_video::raw_buffer(uint32_t frame, float idt_matrix[9], const Video_base::RawInfo& ri)
{
    long size = 0;
    uint8_t* buffer = file_to_mem(_imp->filenames[frame].c_str(), &size);
    if (!buffer){
        return NULL;
    }
	_imp->processor->set_interpolation(_rawinfo.interpolation);
	_imp->processor->set_highlight(_rawinfo.highlight);
	// Get ACES AP0 color space buffer
	uint16_t* rawbuffer = _imp->processor->get_aces(buffer, size);
	Dng_processor::Idt idt = _imp->processor->get_idt_matrix();
	memcpy((void*)idt_matrix, (void*)idt.matrix, 9*sizeof(float));
    free(buffer);
	return rawbuffer;
}

uint32_t Dng_video::black_level()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->color.dng_levels.dng_black;
}

uint32_t Dng_video::white_level()
{
     libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->color.dng_levels.dng_whitelevel[0];
}

void Dng_video::free_buffer()
{

}

float Dng_video::fps()
{
    return 0.0f;
}

uint32_t Dng_video::frame_count()
{
    return _imp->filenames.size();
}

uint32_t Dng_video::raw_resolution_x()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->sizes.iwidth;
}

uint32_t Dng_video::raw_resolution_y()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->sizes.iheight;
}

std::string Dng_video::camera_name()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return std::string(imgd->color.UniqueCameraModel);
}

std::string Dng_video::lens_name()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return std::string(imgd->lens.Lens);
}

std::string Dng_video::lens_name_by_id()
{
    return "";
}

float Dng_video::focal_length()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->other.focal_len;
}

float Dng_video::focal_dist()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->shootinginfo.AFPoint;
}

float Dng_video::aperture()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->other.aperture;
}

float Dng_video::crop_factor()
{
    return 1.62;
}

float Dng_video::final_crop_factor()
{
    return 1.62;
}

uint32_t Dng_video::iso()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return imgd->other.iso_speed;
}

uint32_t Dng_video::shutter_speed()
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());
    return 1.0f / (float)imgd->other.shutter;
}

int Dng_video::pixel_binning_x()
{
    return 1;
}

int Dng_video::pixel_binning_y()
{
    return 1;
}

int Dng_video::sampling_factor_x()
{
    return 1;
}

int Dng_video::sampling_factor_y()
{
    return 1;
}

int Dng_video::bpp()
{
    return 14;
}

void Dng_video::sensor_resolulion(int& x, int& y)
{
    libraw_data_t* imgd = static_cast<libraw_data_t*>(_imp->processor->imgdata());

    x = imgd->makernotes.canon.SensorWidth;
    y = imgd->makernotes.canon.SensorHeight;
    if (x == 0 && y == 0){
        x = 1500;y = 1000;
    }
}