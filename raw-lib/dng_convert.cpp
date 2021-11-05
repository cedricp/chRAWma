#include "dng_convert.h"
#include "idt/dng_idt.h"
extern "C"{
#include "mlv-lib/dng/dng.h"
}
#include <libraw/libraw.h>

struct Dng_converter::dngc_impl{
	LibRaw* libraw;
	dngObject_t* dng_object;
	mlvObject_t* mlv_object;
	libraw_processed_image_t* image;
};

Dng_converter::Dng_converter(mlvObject_t* mlv_object) : _buffer(NULL)
{
	_imp = new dngc_impl;
	_imp->libraw = new LibRaw;
	_imp->mlv_object = mlv_object;
	_imp->image = NULL;

	int par[4] = {1,1,1,1};
	_imp->dng_object = initDngObject(mlv_object, UNCOMPRESSED_RAW, getMlvFramerateOrig(mlv_object), par);
}

Dng_converter::~Dng_converter()
{
	if (_imp->image){
		free(_imp->image);
		_imp->image = NULL;
	}
	delete _imp->libraw;
	delete _imp;
}

void Dng_converter::free_buffer()
{
	if (_imp->image){
		free(_imp->image);
		_imp->image = NULL;
	}
}

uint16_t* Dng_converter::get_buffer(uint32_t frame)
{
	if (_imp->image){
		free(_imp->image);
		_imp->image = NULL;
	}
	/*
	# - Debayer method
	--+----------------------
	0 - linear interpolation
	1 - VNG interpolation
	2 - PPG interpolation
	3 - AHD interpolation
	4 - DCB interpolation
	11 - DHT intepolation
	12 - Modified AHD intepolation (by Anton Petrusevich)
	*/

	// XYZ colorspace
	_imp->libraw->imgdata.params.use_auto_wb = 0;
	_imp->libraw->imgdata.params.output_color = 5;
	_imp->libraw->imgdata.params.output_bps = 16;
	_imp->libraw->imgdata.params.gamm[0] = 1.0;
	_imp->libraw->imgdata.params.gamm[1] = 1.0;
	_imp->libraw->imgdata.params.user_qual = 0;
	_imp->libraw->imgdata.params.use_camera_matrix = 1;
	_imp->libraw->imgdata.params.use_camera_wb = 1;
	_imp->libraw->imgdata.params.user_mul[0] = _imp->libraw->imgdata.color.cam_mul[0];
	_imp->libraw->imgdata.params.user_mul[1] = _imp->libraw->imgdata.color.cam_mul[1];
	_imp->libraw->imgdata.params.user_mul[2] = _imp->libraw->imgdata.color.cam_mul[2];
	_imp->libraw->imgdata.params.use_rawspeed = 1;
	_imp->libraw->imgdata.params.no_interpolation=0;

	uint32_t size = 0;
	uint8_t* dng_buffer = get_dng_buffer(frame, size);
	int obret = _imp->libraw->open_buffer(dng_buffer, size);

	if (obret != LIBRAW_SUCCESS){
		printf("Open buffer error\n");
		return NULL;
	}

	if(_imp->libraw->unpack() != LIBRAW_SUCCESS){
		printf("Unpack error\n");
		return NULL;
	}

	int err;
	err = _imp->libraw->dcraw_process();
	if(err!= LIBRAW_SUCCESS){
		printf("dcraw process image error\n");
		return NULL;
	}

	_imp->image = _imp->libraw->dcraw_make_mem_image(&err);

	if (err != LIBRAW_SUCCESS){
		printf("make mem image error\n");
		return NULL;
	}

	DNGIdt idt = DNGIdt(_imp->libraw->imgdata.rawdata);
	idt.getDNGIDTMatrix2(_idt.matrix);

	_imp->libraw->recycle();

	return (uint16_t*)&_imp->image->data;
}

Dng_converter::Idt Dng_converter::get_idt_matrix()
{
	return _idt;
}

uint8_t* Dng_converter::get_dng_buffer(uint32_t frame, uint32_t& size)
{
	uint8_t *buffer = getDngFrameBuffer(_imp->mlv_object, _imp->dng_object, frame);
	size = _imp->dng_object->image_size + _imp->dng_object->header_size;
	return buffer;
}
