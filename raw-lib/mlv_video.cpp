
extern "C"{
	#include "video_mlv.h"
	#include "dng/dng.h"
	#include "llrawproc/llrawproc.h"
}
#include <string.h>
#include "mlv_video.h"
#include <iostream>
#include "lens_id.h"
#include "dng_convert.h"

struct mlv_imp
{
	mlvObject_t* mlv_object;
	Dng_processor *dngc;
	dngObject_t* dng_object;
	mlv_wbal_hdr_t original_wbal;
};

std::string get_map_name(mlvObject_t* mvl_object)
{
    char name[1024];
    snprintf(name, 1024, "%x_%ix%i.fpm", mvl_object->IDNT.cameraModel, mvl_object->RAWI.raw_info.width, mvl_object->RAWI.raw_info.height);
    return name;
}

Mlv_video::Mlv_video(std::string filename) : Video_base()
{
	_imp = new mlv_imp;

	int err;
	char err_mess[512];

	_imp->mlv_object = initMlvObjectWithClip(filename.c_str(), MLV_OPEN_FULL, &err, err_mess);

	int par[4] = {1,1,1,1};
	_imp->dng_object = initDngObject(_imp->mlv_object, UNCOMPRESSED_RAW, getMlvFramerateOrig(_imp->mlv_object), par);

	_imp->dngc = new Dng_processor;

	if (err){
		std::cout << "MLV open problem : " << err_mess << std::endl;
		return;
	}
	memcpy(&_imp->original_wbal, &_imp->mlv_object->WBAL, sizeof(mlv_wbal_hdr_t));
	_rawinfo.crop_factor = crop_factor();
	_rawinfo.focal_length = focal_length();
}

Mlv_video::~Mlv_video()
{
	freeMlvObject(_imp->mlv_object);
	delete _imp->dngc;
	delete _imp;
}

uint32_t Mlv_video::raw_resolution_x()
{
	return getMlvWidth(_imp->mlv_object);
}

uint32_t Mlv_video::raw_resolution_y()
{
	return getMlvHeight(_imp->mlv_object);
}

float Mlv_video::fps()
{
	return getMlvFramerateOrig(_imp->mlv_object);
}

uint32_t Mlv_video::black_level()
{
	return _imp->mlv_object->RAWI.raw_info.black_level;
}

uint32_t Mlv_video::white_level()
{
	return _imp->mlv_object->RAWI.raw_info.white_level;

}

void Mlv_video::free_buffer()
{
	_imp->dngc->free_buffer();
}

uint16_t* Mlv_video::raw_buffer(uint32_t frame, float idt_matrix[9], const RawInfo& ri)
{
	if (frame >= _imp->mlv_object->frames){
		frame = _imp->mlv_object->frames - 1;
	}

	memcpy(&_imp->mlv_object->WBAL, &_imp->original_wbal, sizeof(mlv_wbal_hdr_t));

	if (ri.temperature != -1){
		_imp->mlv_object->WBAL.wb_mode = WB_KELVIN;
		_imp->mlv_object->WBAL.kelvin = ri.temperature;
	}

	llrpSetFixRawMode(_imp->mlv_object, 1);
	llrpSetChromaSmoothMode(_imp->mlv_object, raw_settings().chroma_smooth+1);
	llrpResetDngBWLevels(_imp->mlv_object);

	if (ri.fix_focuspixels){
		int focusDetect = llrpDetectFocusDotFixMode(_imp->mlv_object);
		if( focusDetect != 0 ){
			llrpSetFocusPixelMode(_imp->mlv_object, focusDetect);
		}
	} else {
		llrpSetFocusPixelMode(_imp->mlv_object, FP_OFF);
	}

	uint8_t *buffer = getDngFrameBuffer(_imp->mlv_object, _imp->dng_object, frame);
	size_t size = _imp->dng_object->image_size + _imp->dng_object->header_size;
	// Set debayer type
	_imp->dngc->set_interpolation(_rawinfo.interpolation);
	_imp->dngc->set_highlight(_rawinfo.highlight);
	// Get ACES AP0 color space buffer
	uint16_t* rawbuffer = _imp->dngc->get_aces(buffer, size);
	Dng_processor::Idt idt = _imp->dngc->get_idt_matrix();
	memcpy((void*)idt_matrix, (void*)idt.matrix, 9*sizeof(float));
	return rawbuffer;
}

std::string Mlv_video::camera_name()
{
	return std::string((const char*)_imp->mlv_object->IDNT.cameraName);
}

std::string Mlv_video::lens_name()
{
	// std::vector<std::string> lenses = get_lens_by_type(_imp->mlv_object->LENS.lensID);
	// if (!lenses.empty()){
	// 	return lenses[0];
	// }
	std::string lensname = ((char*)_imp->mlv_object->LENS.lensName);
	return lensname;
}

std::string Mlv_video::lens_name_by_id()
{
	std::vector<std::string> lenses = get_lens_by_type(_imp->mlv_object->LENS.lensID);
	if (!lenses.empty()){
		return lenses[0];
	}
	return ((char*)_imp->mlv_object->LENS.lensName);;
}

float Mlv_video::focal_length()
{
	return _imp->mlv_object->LENS.focalLength;
}

float Mlv_video::focal_dist()
{
	return (float)_imp->mlv_object->LENS.focalDist / 100.0f;
}

void Mlv_video::sensor_resolulion(int& x, int& y)
{
	x = _imp->mlv_object->RAWC.sensor_res_x;
	y = _imp->mlv_object->RAWC.sensor_res_y;
}

float Mlv_video::aperture()
{
	return _imp->mlv_object->LENS.aperture / 100.0f;
}

uint32_t Mlv_video::frame_count()
{
	return _imp->mlv_object->frames;
}

float Mlv_video::crop_factor()
{
	return float(_imp->mlv_object->RAWC.sensor_crop) / 100.f;
}

float Mlv_video::final_crop_factor()
{
	const mlv_rawc_hdr_t& m = _imp->mlv_object->RAWC;
	float ratio = ((float)m.sensor_res_x / ((float)sampling_factor_x() * (float)raw_resolution_x())) * crop_factor();
	if(ratio < crop_factor()){
		ratio = crop_factor();
	}
	return ratio;
}

uint32_t Mlv_video::iso()
{
	return _imp->mlv_object->EXPO.isoValue;
}

int Mlv_video::bpp()
{
	return _imp->mlv_object->RAWI.raw_info.bits_per_pixel;
}

uint32_t Mlv_video::shutter_speed()
{
	return _imp->mlv_object->EXPO.shutterValue / uint64_t(1000);
}

int Mlv_video::pixel_binning_x()
{
	return _imp->mlv_object->RAWC.binning_x;
}

int Mlv_video::pixel_binning_y()
{
	return _imp->mlv_object->RAWC.binning_y;
}

int Mlv_video::sampling_factor_x()
{
	return _imp->mlv_object->RAWC.binning_x + _imp->mlv_object->RAWC.skipping_x;
}

int Mlv_video::sampling_factor_y()
{
	return _imp->mlv_object->RAWC.binning_y + _imp->mlv_object->RAWC.skipping_y;
}