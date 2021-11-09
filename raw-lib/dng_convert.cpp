#include "dng_convert.h"
#include "idt/dng_idt.h"
#include <libraw/libraw.h>
#include <sys/stat.h>

struct Dng_processor::dngc_impl{
	LibRaw* libraw;
	libraw_processed_image_t* image;
};

Dng_processor::Dng_processor() : _buffer(NULL)
{
	_imp = new dngc_impl;
	_imp->libraw = new LibRaw;
	_imp->image = NULL;
	_w = _h = 0;
}

Dng_processor::~Dng_processor()
{
	if (_imp->image){
		free(_imp->image);
		_imp->image = NULL;
	}
	delete _imp->libraw;
	delete _imp;
}

void Dng_processor::free_buffer()
{
	if (_imp->image){
		free(_imp->image);
		_imp->image = NULL;
	}
}

uint16_t* Dng_processor::get_aces_from_file(std::string dng_filename)
{
	FILE* file = fopen(dng_filename.c_str(), "rb");
	if (!file){
		printf("Dng_processor : Cannot open file %s\n", dng_filename.c_str());
		return NULL;
	}

	struct stat sb;
	if (stat(dng_filename.c_str(), &sb) == -1) {
		printf("Dng_processor : Cannot open file (stat) %s\n", dng_filename.c_str());
		return NULL;
	}

	size_t file_size =  sb.st_size;

	uint8_t* dng_memory = (uint8_t*)malloc(file_size);
	fread(dng_memory, file_size, 1, file);

	fclose(file);

	uint16_t* result = get_aces(dng_memory, file_size);
	free(dng_memory);

	return result;
}

uint16_t* Dng_processor::get_aces(uint8_t* buffer, size_t buffersize)
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
	int obret = _imp->libraw->open_buffer(buffer, buffersize);

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

	_w = _imp->image->width;
	_h = _imp->image->height;

	DNGIdt idt = DNGIdt(_imp->libraw->imgdata.rawdata);
	idt.getDNGIDTMatrix2(_idt.matrix);

	_imp->libraw->recycle();

	return (uint16_t*)&_imp->image->data;
}

Dng_processor::Idt Dng_processor::get_idt_matrix()
{
	return _idt;
}
