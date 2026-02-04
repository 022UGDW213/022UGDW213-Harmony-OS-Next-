#ifndef HARMONY_LOADER_H
#define HARMONY_LOADER_H

#include <stddef.h>
#include <stdint.h>
#include "loader/formats.h"

binary_type_t loader_detect_format(const uint8_t* buffer, size_t size);
int loader_parse_header(const uint8_t* buffer, size_t size, executable_image_t* out_img);

#endif
