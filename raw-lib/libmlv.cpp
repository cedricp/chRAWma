
extern "C"{
	#include "video_mlv.h"
	#include "dng/dng.h"
	#include "llrawproc/llrawproc.h"
}
#include <string.h>
#include "libmlv.h"
#include <iostream>
#include "lens_id.h"
#include "dng_convert.h"

struct mlv_imp
{
	mlvObject_t* mlv_object;
	Dng_processor *dngc;
	dngObject_t* dng_object;
};

std::string get_map_name(mlvObject_t* mvl_object)
{
    char name[1024];
    snprintf(name, 1024, "%x_%ix%i.fpm", mvl_object->IDNT.cameraModel, mvl_object->RAWI.raw_info.width, mvl_object->RAWI.raw_info.height);
    return name;
}

Mlv_video::Mlv_video(std::string filename)
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

	llrpSetFixRawMode(_imp->mlv_object, 1);
	llrpSetChromaSmoothMode(_imp->mlv_object, CS_2x2);
	llrpResetDngBWLevels(_imp->mlv_object);

    int focusDetect = llrpDetectFocusDotFixMode(_imp->mlv_object);
    if( focusDetect != 0 ){
		llrpSetFocusPixelMode(_imp->mlv_object, focusDetect);
    }
}

Mlv_video::~Mlv_video()
{
	freeMlvObject(_imp->mlv_object);
	delete _imp->dngc;
	delete _imp;
}

uint32_t Mlv_video::frame_count()
{
	return _imp->mlv_object->frames;
}

uint16_t* Mlv_video::get_raw_frame(uint32_t frame)
{
	mlvObject_t* video = _imp->mlv_object;
    int pixels_count = video->RAWI.xRes * video->RAWI.yRes;
    size_t unpacked_frame_size = pixels_count * sizeof(uint16_t);
    uint16_t * unpacked_frame = (uint16_t *)malloc( unpacked_frame_size );

    getMlvRawFrameUint16(_imp->mlv_object, frame, unpacked_frame);

    return unpacked_frame;
}

uint32_t Mlv_video::resolution_x()
{
	return getMlvWidth(_imp->mlv_object);
}

uint32_t Mlv_video::resolution_y()
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
	return _imp->mlv_object->RAWI.raw_info.black_level;

}

void Mlv_video::free_buffer()
{
	_imp->dngc->free_buffer();
}

unsigned short* Mlv_video::get_raw_processed_buffer(uint32_t frame, float idt_matrix[9])
{
	uint8_t *buffer = getDngFrameBuffer(_imp->mlv_object, _imp->dng_object, frame);
	size_t size = _imp->dng_object->image_size + _imp->dng_object->header_size;

	uint16_t* rawbuffer = _imp->dngc->get_aces(buffer, size);
	Dng_processor::Idt idt = _imp->dngc->get_idt_matrix();
	memcpy((void*)idt_matrix, (void*)idt.matrix, 9*sizeof(float));
	return rawbuffer;
}

std::string Mlv_video::get_camera_name()
{
	return std::string((const char*)_imp->mlv_object->IDNT.cameraName);
}

std::string Mlv_video::get_lens_name()
{
	std::vector<std::string> lenses = get_lens_by_type(_imp->mlv_object->LENS.lensID);
	if (!lenses.empty()){
		return lenses[0];
	}
	return std::string((char*)_imp->mlv_object->LENS.lensName);
}

float Mlv_video::get_focal_length()
{
	return _imp->mlv_object->LENS.focalLength;
}

float Mlv_video::get_focal_dist()
{
	return _imp->mlv_object->LENS.focalDist / 1000.0f;
}

float Mlv_video::get_aperture()
{
	return _imp->mlv_object->LENS.aperture / 100.0f;
}
