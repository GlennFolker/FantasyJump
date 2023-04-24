#include "pngio.h"

#include <png.h>
#include <cstdio>
#include <malloc.h>
#include <string>

namespace Fantasy {
    Png::Png(const std::string &filename) {
        this->filename = filename;
        width = height = -1;
        errored = false;
        data = nullptr;

        FILE *fp = fopen(filename.c_str(), "rb");
        if(!fp) {
            printf("%s: Failed to open file.\n", filename.c_str());
            errored = true;
            return;
        }

        png_struct *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if(!png_ptr) {
            printf("%s: Failed to create read struct.\n", filename.c_str());

            fclose(fp);
            errored = true;
            return;
        }

        png_info *info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr) {
            printf("%s: Failed to create info struct.\n", filename.c_str());

            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            fclose(fp);
            errored = true;
            return;
        }

        if(setjmp(png_jmpbuf(png_ptr))) {
            printf("%s: PNG loading encountered errors.\n", filename.c_str());

            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            fclose(fp);
            errored = true;
            return;
        }

        png_init_io(png_ptr, fp);
        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        unsigned char type = png_get_color_type(png_ptr, info_ptr);
        unsigned char channels = png_get_channels(png_ptr, info_ptr);
        unsigned char depth = png_get_bit_depth(png_ptr, info_ptr);

        if(type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
        if(type == PNG_COLOR_TYPE_GRAY && depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
        if(type == PNG_COLOR_TYPE_GRAY || type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
        if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
        if(depth == 16) png_set_strip_16(png_ptr);
        if(depth < 8) png_set_packing(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        data = new unsigned char *[height];
        for(int y = 0; y < height; y++)data[y] = new unsigned char[width * channels];

        png_read_image(png_ptr, data);

        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);
    }

    Png::Png(int width, int height) {
        this->width = width;
        this->height = height;

        data = new unsigned char *[height];
        for(int y = 0; y < height; y++) {
            int count = width * 4;
            data[y] = new unsigned char[count];
            for(int x = 0; x < count; x++) data[y][x] = 0;
        }
    }

    Png::~Png() {
        if(data != nullptr) {
            for(int y = 0; y < height; y++) delete[] data[y];
            delete[] data;
        }
    }

    void Png::write(const std::string &filename) const {
        FILE *png_file = fopen(filename.c_str(), "wb");
        if(!png_file) {
            printf("Couldn't open %s", filename.c_str());
            return;
        }

        png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if(!png_ptr) {
            printf("%s: Couldn't create write struct.", filename.c_str());
            return;
        }

        png_info *info_ptr = png_create_info_struct(png_ptr);
        if(!info_ptr) {
            printf("%s: Couldn't create info struct.", filename.c_str());
            return;
        }

        png_init_io(png_ptr, png_file);
        png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);
        png_write_image(png_ptr, data);
        png_write_end(png_ptr, nullptr);

        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(png_file);
    }
}
