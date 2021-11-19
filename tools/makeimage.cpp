//Standard headers
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cctype>
#include <unordered_set>
#include "exoquant.h" //Exoquant
#define STBI_FAILURE_USERMSG //More detailed errors from stb_image
//Include stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Image format definitions
#define IMG_FMT_I4 0
#define IMG_FMT_I8 1
#define IMG_FMT_IA4 2
#define IMG_FMT_IA8 3
#define IMG_FMT_IA16 4
#define IMG_FMT_CI4 5
#define IMG_FMT_CI8 6
#define IMG_FMT_RGBA16 7
#define IMG_FMT_RGBA32 8

#define OPACITY_THRESHOLD 192


//Color conversion tables
uint8_t color_8to3[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x07,0x07,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07
};

uint8_t color_8to4[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x01,0x02,0x02, 0x02,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x03, 0x03,0x03,0x03,0x03,
    0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x04,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x05,0x05,0x05,
    0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x06, 0x06,0x06,0x06,0x07,
    0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08, 0x08,0x08,0x08,0x08,
    0x08,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x09,
    0x09,0x09,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0a,
    0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0b,
    0x0b,0x0b,0x0b,0x0b, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0f, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f
};

uint8_t color_8to5[256] =
{
    0x00,0x00,0x00,0x00, 0x00,0x01,0x01,0x01, 0x01,0x01,0x01,0x01, 0x01,0x02,0x02,0x02,
    0x02,0x02,0x02,0x02, 0x02,0x03,0x03,0x03, 0x03,0x03,0x03,0x03, 0x03,0x04,0x04,0x04,
    0x04,0x04,0x04,0x04, 0x04,0x04,0x05,0x05, 0x05,0x05,0x05,0x05, 0x05,0x05,0x06,0x06,
    0x06,0x06,0x06,0x06, 0x06,0x06,0x07,0x07, 0x07,0x07,0x07,0x07, 0x07,0x07,0x08,0x08,
    0x08,0x08,0x08,0x08, 0x08,0x08,0x09,0x09, 0x09,0x09,0x09,0x09, 0x09,0x09,0x09,0x0a,
    0x0a,0x0a,0x0a,0x0a, 0x0a,0x0a,0x0a,0x0b, 0x0b,0x0b,0x0b,0x0b, 0x0b,0x0b,0x0b,0x0c,
    0x0c,0x0c,0x0c,0x0c, 0x0c,0x0c,0x0c,0x0d, 0x0d,0x0d,0x0d,0x0d, 0x0d,0x0d,0x0d,0x0d,
    0x0e,0x0e,0x0e,0x0e, 0x0e,0x0e,0x0e,0x0e, 0x0f,0x0f,0x0f,0x0f, 0x0f,0x0f,0x0f,0x0f,
    0x10,0x10,0x10,0x10, 0x10,0x10,0x10,0x10, 0x11,0x11,0x11,0x11, 0x11,0x11,0x11,0x11,
    0x12,0x12,0x12,0x12, 0x12,0x12,0x12,0x12, 0x12,0x13,0x13,0x13, 0x13,0x13,0x13,0x13,
    0x13,0x14,0x14,0x14, 0x14,0x14,0x14,0x14, 0x14,0x15,0x15,0x15, 0x15,0x15,0x15,0x15,
    0x15,0x16,0x16,0x16, 0x16,0x16,0x16,0x16, 0x16,0x16,0x17,0x17, 0x17,0x17,0x17,0x17,
    0x17,0x17,0x18,0x18, 0x18,0x18,0x18,0x18, 0x18,0x18,0x19,0x19, 0x19,0x19,0x19,0x19,
    0x19,0x19,0x1a,0x1a, 0x1a,0x1a,0x1a,0x1a, 0x1a,0x1a,0x1b,0x1b, 0x1b,0x1b,0x1b,0x1b,
    0x1b,0x1b,0x1b,0x1c, 0x1c,0x1c,0x1c,0x1c, 0x1c,0x1c,0x1c,0x1d, 0x1d,0x1d,0x1d,0x1d,
    0x1d,0x1d,0x1d,0x1e, 0x1e,0x1e,0x1e,0x1e, 0x1e,0x1e,0x1e,0x1f, 0x1f,0x1f,0x1f,0x1f
};

//Case insensitive string compare
int strcmpins(const char *a, const char *b)
{
    while ((*a != 0) && (*b != 0)) {
        int diff = tolower(*a) - tolower(*b);
        if (diff) {
            return diff;
        }
        a++;
        b++;
    }
    return tolower(*a) - tolower(*b);
}

int ImgFormatGet(const char *name)
{
	//Must be in same order as format defines
	const char *format_names[] = { "i4", "i8", "ia4", "ia8", "ia16", "ci4", "ci8", "rgba16", "rgba32" };
	for(int i=0; i<sizeof(format_names)/sizeof(char *); i++) {
		//Search through format names for a case-insensitive match
		if(strcmpins(name, format_names[i]) == 0) {
			return i;
		}
	}
	//Format not found
	return -1;
}

bool IsImageGrayscale(uint8_t *image_data, int image_w, int image_h)
{
	for(int i=0; i<image_w*image_h; i++) {
		//Check for the red green and blue channels to not be identical
		if(image_data[i*4] != image_data[(i*4)+1] || image_data[i*4] != image_data[(i*4)+2]) {
			//Image isn't grayscale
			return false;
		}
	}
	//Image is grayscale
	return true;
}

bool IsImageSemitransparent(uint8_t *image_data, int image_w, int image_h)
{
	for(int i=0; i<image_w*image_h; i++) {
		//Check for alpha to not be 0 or 255
		if(image_data[(i*4)+3] != 0 && image_data[(i*4)+3] != 255) {
			//Image is partially transparent
			return true;
		}
	}
	//Image is not partially transparent
	return false;
}

int CountImageColors(uint8_t *image_data, int image_w, int image_h)
{
	//Write all image pixels to a vector
	std::vector<uint32_t> colors;
	colors.reserve(image_w*image_h);
	for(int i=0; i<image_w*image_h; i++) {
		colors.push_back(image_data[i*4]|(image_data[(i*4)+1] << 8)|(image_data[(i*4)+2] << 16)|(image_data[(i*4)+3] << 24));
	}
	//Calculate number of unique image colors through temporary unordered set
	return std::unordered_set<uint32_t>(colors.begin(), colors.end()).size();
}

int GetBestImageFormat(uint8_t *image_data, int image_w, int image_h)
{
	if(IsImageGrayscale(image_data, image_w, image_h)) {
		//Grayscale format determination
		if(IsImageSemitransparent(image_data, image_w, image_h)) {
			//IA16 is smoothest format for grayscale images with semitransparency
			//while remaining smaller than RGBA32
			return IMG_FMT_IA16;
		} else {
			int colors = CountImageColors(image_data, image_w, image_h);
			if(colors <= 16) {
				//Use CI4 if no more than 16 colors are present
				if(image_w % 2 != 0) {
					//But force CI8 if width is odd
					return IMG_FMT_CI8;
				} else {
					return IMG_FMT_CI4;
				}
			} else if(colors <= 256) {
				//Use CI8 if between 16 and 256 colors are present
				return IMG_FMT_CI8;
			} else {
				//Use more efficient 16-bit format if no semitransparency
				//is present but more than 256 colors are used
				return IMG_FMT_RGBA16;
			}
		}
	} else {
		if(IsImageSemitransparent(image_data, image_w, image_h)) {
			//Unfortunately RGBA32 takes 4 bytes per pixel but it is the only
			//non-grayscale format to allow semitransparency
			return IMG_FMT_RGBA32;
		} else {
			int colors = CountImageColors(image_data, image_w, image_h);
			if(colors <= 16) {
				//Use CI4 if no more than 16 colors are present
				if(image_w % 2 != 0) {
					//But force CI8 if width is odd
					return IMG_FMT_CI8;
				} else {
					return IMG_FMT_CI4;
				}
			} else if(colors <= 256) {
				//Use CI8 if between 16 and 256 colors are present
				return IMG_FMT_CI8;
			} else {
				//Use more efficient 16-bit format if no semitransparency
				//is present but more than 256 colors are used
				return IMG_FMT_RGBA16;
			}
		}
	}
}

void WriteU32(FILE *file, uint32_t value)
{
	uint8_t temp[4];
	//Split value into bytes in big-endian order
    temp[0] = value >> 24;
    temp[1] = (value >> 16) & 0xFF;
    temp[2] = (value >> 8) & 0xFF;
    temp[3] = value & 0xFF;
	//Write it
    fwrite(temp, 4, 1, file);
}

void WriteU16(FILE *file, uint32_t value)
{
	uint8_t temp[2];
	//Split value into bytes in big-endian order
    temp[0] = value >> 8;
    temp[1] = value & 0xFF;
	//Write it
    fwrite(temp, 2, 1, file);
}

void AlignFile(FILE *file, uint32_t alignment)
{
    uint32_t ofs = ftell(file);
    while (ofs % alignment) {
		//Write zero until file is aligned
        uint8_t zero = 0;
        fwrite(&zero, 1, 1, file);
        ofs++;
    }
}

uint32_t GetImageDataSize(int format, int image_w, int image_h)
{
	uint32_t size;
	switch(format) {
		case IMG_FMT_I4:
		case IMG_FMT_IA4:
		case IMG_FMT_CI4:
		//4-bit formats
			size = ((image_w*image_h)+1)/2;
			break;
			
		case IMG_FMT_I8:
		case IMG_FMT_IA8:
		case IMG_FMT_CI8:
		//8-bit formats
			size = image_w*image_h;
			break;
			
		case IMG_FMT_IA16:
		case IMG_FMT_RGBA16:
		//16-bit formats
			size = image_w*image_h*2;
			break;
			
		case IMG_FMT_RGBA32:
		//32-bit format
			size = image_w*image_h*4;
			break;
			
		default:
			size = 0;
			break;
	}
	//Round to nearest 8-byte boundary
	return (size+7) & ~7;
}

void WriteImageI4(FILE *file, uint8_t *data, int w, int h)
{
	uint8_t value;
	int i;
	for(i=0; i<w*h; i++) {
		//RGB to grayscale conversion
		float temp_r = data[i*4]*0.3f;
		float temp_g = data[(i*4)+1]*0.59f;
		float temp_b = data[(i*4)+2]*0.11f;
		uint8_t intensity = temp_r+temp_g+temp_b;
		intensity *= data[(i*4)+3]/255.0f; //Modulate intensity by alpha
		uint8_t pixel_val = color_8to4[intensity]; //Get pixel value
		if(i%2 == 1) {
			value |= pixel_val; //Odd pixels write lower 4 bits
			fwrite(&value, 1, 1, file); //Push pixel pair out to file
		} else {
			value = pixel_val << 4; //Even pixels write upper 4 bits
		}
	}
	//Write remaining pixel pair if number of image pixels is odd
	if(i%2 == 1) {
		fwrite(&value, 1, 1, file);
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImageI8(FILE *file, uint8_t *data, int w, int h)
{
	int i;
	for(i=0; i<w*h; i++) {
		//RGB to grayscale conversion
		float temp_r = data[i*4]*0.3f;
		float temp_g = data[(i*4)+1]*0.59f;
		float temp_b = data[(i*4)+2]*0.11f;
		uint8_t intensity = temp_r+temp_g+temp_b;
		intensity *= data[(i*4)+3]/255.0f; //Modulate intensity by alpha
		fwrite(&intensity, 1, 1, file); //Write intensity as pixel
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImageIA4(FILE *file, uint8_t *data, int w, int h)
{
	uint8_t value;
	int i;
	for(i=0; i<w*h; i++) {
		//RGB to grayscale conversion
		float temp_r = data[i*4]*0.3f;
		float temp_g = data[(i*4)+1]*0.59f;
		float temp_b = data[(i*4)+2]*0.11f;
		uint8_t intensity = temp_r+temp_g+temp_b;
		uint8_t alpha = data[(i*4)+3];
		uint8_t pixel_val = color_8to3[intensity] << 1; //Cram intensity into upper 3 bits
		//Enable opacity if alpha exceeds threshold
		if(alpha >= OPACITY_THRESHOLD) {
			pixel_val |= 1;
		}
		if(i%2 == 1) {
			value |= pixel_val; //Odd pixels write lower 4 bits
			fwrite(&value, 1, 1, file); //Push pixel pair out to file
		} else {
			value = pixel_val << 4; //Even pixels write upper 4 bits
		}
	}
	//Write remaining pixel pair if number of image pixels is odd
	if(i%2 == 1) {
		fwrite(&value, 1, 1, file);
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImageIA8(FILE *file, uint8_t *data, int w, int h)
{
	int i;
	for(i=0; i<w*h; i++) {
		//RGB to grayscale conversion
		float temp_r = data[i*4]*0.3f;
		float temp_g = data[(i*4)+1]*0.59f;
		float temp_b = data[(i*4)+2]*0.11f;
		uint8_t intensity = temp_r+temp_g+temp_b;
		uint8_t alpha = data[(i*4)+3];
		//Cram combined intensity and alpha values to 8 bits
		uint8_t pixel = color_8to4[intensity] << 4;
		pixel |= color_8to4[alpha];
		fwrite(&pixel, 1, 1, file); //Write pixel
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImageIA16(FILE *file, uint8_t *data, int w, int h)
{
	uint8_t value = 0;
	int i;
	for(i=0; i<w*h; i++) {
		//RGB to grayscale conversion
		float temp_r = data[i*4]*0.3f;
		float temp_g = data[(i*4)+1]*0.59f;
		float temp_b = data[(i*4)+2]*0.11f;
		uint8_t intensity = temp_r+temp_g+temp_b;
		uint8_t alpha = data[(i*4)+3];
		fwrite(&intensity, 1, 1, file); //Write intensity
		fwrite(&alpha, 1, 1, file); //Write alpha
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteColorRGBA16(FILE *file, uint8_t *src_color)
{
	//Convert r, g, and b channels to 5 bit
	uint8_t r = color_8to5[src_color[0]];
	uint8_t g = color_8to5[src_color[1]];
	uint8_t b = color_8to5[src_color[2]];
	uint16_t value = (r << 11)|(g << 6)|(b << 1); //Generate 16-bit RGB value
	//Enable opacity if alpha exceeds threshold
	if(src_color[3] >= OPACITY_THRESHOLD) {
		value |= 1;
	}
	WriteU16(file, value); //Write pixel
}

void WriteImageCI(FILE *file, uint8_t *data, int w, int h, bool is_8bpp)
{
	//Calculate number of colors
	int num_colors = 16;
	if(is_8bpp) {
		num_colors = 256;
	}
	//Create data buffers
	uint8_t *data_buf = new uint8_t[w*h](); //8bpp image buffer
	uint8_t *pal_buf = new uint8_t[num_colors*4](); //Palette buffer
	exq_data *exq = exq_init(); //Initialize exoquant
	exq_feed(exq, data, w*h); //Read image into exoquant
	exq_quantize_hq(exq, num_colors); //Main color reduction
	exq_get_palette(exq, pal_buf, num_colors); //Read palette
	exq_map_image_ordered(exq, w, h, data, data_buf); //Read reduced image
	exq_free(exq); //Deinitialize exoquant
	if(is_8bpp) {
		//No need for processing since exoquant outputs exactly like n64 CI8
		fwrite(data_buf, 1, w*h, file);
	} else {
		uint8_t value;
		int i;
		for(i=0; i<w*h; i++) {
			uint8_t pixel_val = data_buf[i] & 0xF; //Mask each pixel to 16 colors
			if(i%2 == 1) {
				value |= pixel_val; //Odd pixels write lower 4 bits
				fwrite(&value, 1, 1, file); //Push pixel pair out to file
			} else {
				value = pixel_val << 4; //Even pixels write upper 4 bits
			}
		}
		//Write remaining pixel pair if number of image pixels is odd
		if(i%2 == 1) {
			fwrite(&value, 1, 1, file);
		}
	}
	//Align to 8 bytes
	AlignFile(file, 8);
	//Write palette
	for(int i=0; i<num_colors; i++) {
		WriteColorRGBA16(file, &pal_buf[i*4]);
	}
	delete[] pal_buf;
	delete[] data_buf;
}

void WriteImageCI4(FILE *file, uint8_t *data, int w, int h)
{
	WriteImageCI(file, data, w, h, false);
}

void WriteImageCI8(FILE *file, uint8_t *data, int w, int h)
{
	WriteImageCI(file, data, w, h, true);
}

void WriteImageRGBA16(FILE *file, uint8_t *data, int w, int h)
{
	for(int i=0; i<w*h; i++) {
		//Convert each pixel to RGBA16 and then write
		WriteColorRGBA16(file, &data[i*4]);
	}
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImageRGBA32(FILE *file, uint8_t *data, int w, int h)
{
	//stb_image outputs directly in this format and so no processing is needed
	fwrite(data, 1, w*h*4, file);
	//Align to 8 bytes
	AlignFile(file, 8);
}

void WriteImage(FILE *file, uint8_t *data, int w, int h, int format)
{
	switch(format) {
		case IMG_FMT_I4:
			WriteImageI4(file, data, w, h);
			break;
			
		case IMG_FMT_I8:
			WriteImageI8(file, data, w, h);
			break;
		
		case IMG_FMT_IA4:
			WriteImageIA4(file, data, w, h);
			break;
			
		case IMG_FMT_IA8:
			WriteImageIA8(file, data, w, h);
			break;
			
		case IMG_FMT_IA16:
			WriteImageIA16(file, data, w, h);
			break;
			
		case IMG_FMT_CI4:
			WriteImageCI4(file, data, w, h);
			break;
			
		case IMG_FMT_CI8:
			WriteImageCI8(file, data, w, h);
			break;
			
		case IMG_FMT_RGBA16:
			WriteImageRGBA16(file, data, w, h);
			break;
			
		case IMG_FMT_RGBA32:
			WriteImageRGBA32(file, data, w, h);
			break;
		
		default:
			break;
	}
}

int main(int argc, char **argv)
{
	char *in_name;
	char *out_name;
	int image_w, image_h;
	int components_temp;
	int format = -1;
	if(argc != 3 && argc != 4) {
		//Print usage error
		std::cout << "Usage: " << argv[0] << " [fmt] in out" << std::endl;
		return 1;
	}
	if(argc == 4) {
		//Provided image format as second parameter
		format = ImgFormatGet(argv[1]);
		if(format == -1) {
			//Print format name invalid warning
			std::cout << "Image format " << argv[1] << " invalid." << std::endl;
			std::cout << "Image format will be automatically determined now." << std::endl;
		}
		in_name = argv[2];
		out_name = argv[3];
	} else {
		//Did not provide image format
		in_name = argv[1];
		out_name = argv[2];
		std::string temp, fmt_name;
		//Extract extension from filename
		temp = in_name;
		size_t dot_pos = temp.find_last_of('.');
		if(dot_pos != std::string::npos) {
			temp = temp.substr(0, dot_pos);
			dot_pos = temp.find_last_of('.');
			if(dot_pos != std::string::npos) {
				fmt_name = temp.substr(dot_pos+1);
				format = ImgFormatGet(fmt_name.c_str());
				if(format == -1) {
					//Print format name invalid warning
					std::cout << "Image format " << fmt_name << " invalid." << std::endl;
					std::cout << "Image format will be automatically determined now." << std::endl;
				}
			}
		}
	}
	//Load image
	uint8_t *image_data = stbi_load(in_name, &image_w, &image_h, &components_temp, 4);
	if(!image_data) {
		std::cout << "Failed to load " << in_name << ". Cause " << stbi_failure_reason() << "." << std::endl;
		return 1;
	}
	if(image_w % 2 != 0) {
		if(format == IMG_FMT_I4 || format == IMG_FMT_IA4 || format == IMG_FMT_CI4) {
			std::cout << "Image formats I4, IA4, or CI4 are invalid for odd width images." << std::endl;
			stbi_image_free(image_data);
			return 1;
		}
	}
	//Determine best image format if one is not provided or invalid
	if(format == -1) {
		format = GetBestImageFormat(image_data, image_w, image_h);
	}
	//Open output file
	FILE *out_file = fopen(out_name, "wb");
	if(!out_file) {
		std::cout << "Failed to open " << out_name << " for writing." << std::endl;
		return 1;
	}
	WriteU32(out_file, 0x10); //Write data offset
	//Write palette offset
	if(format == IMG_FMT_CI4 || format == IMG_FMT_CI8) {
		WriteU32(out_file, 0x10+GetImageDataSize(format, image_w, image_h));
	} else {
		WriteU32(out_file, 0); //No palette used
	}
	//Write image properties
	WriteU32(out_file, format);
	WriteU16(out_file, image_w);
	WriteU16(out_file, image_h);
	WriteImage(out_file, image_data, image_w, image_h, format);
	//Close program
	fclose(out_file);
	stbi_image_free(image_data);
	return 0;
}