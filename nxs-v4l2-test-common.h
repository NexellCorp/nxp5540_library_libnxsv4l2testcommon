/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sungwoo, Park <swpark@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIB_NXS_V4L2_TEST_COMMON_H__
#define __LIB_NXS_V4L2_TEST_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

struct nxs_v4l2_test_common_option {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t buffer_count;
	uint32_t dst_width;
	uint32_t dst_height;
	uint32_t dst_format;
	uint32_t dst_buffer_count;
	uint32_t loop_count;
	uint32_t memory;
	uint32_t buf_type; /* m2m: src, output path */
	uint32_t dst_buf_type; /* m2m: dst, capture path */
	uint32_t crop_x; /* flyby: source crop start x */
	uint32_t crop_y; /* flyby: source crop start y */
	uint32_t crop_width; /* flyby: source crop width */
	uint32_t crop_height; /* flyby: source crop height */
	bool     display;
};

enum nxs_v4l2_test_type {
	NXS_V4L2_TEST_TYPE_CAPTURE = 1,
	NXS_V4L2_TEST_TYPE_RENDER,
	NXS_V4L2_TEST_TYPE_M2M,
	NXS_V4L2_TEST_TYPE_FLYBY,
};

int nxs_v4l2_test_common_get_option(int argc, char *argv[], uint32_t test_type,
				    struct nxs_v4l2_test_common_option *option);
void
nxs_v4l2_test_common_print_option(struct nxs_v4l2_test_common_option *option);
int nxs_v4l2_test_common_capture(int fd,
				 struct nxs_v4l2_test_common_option *option);
int nxs_v4l2_test_common_render(int fd,
				struct nxs_v4l2_test_common_option *option);
int nxs_v4l2_test_common_m2m(int fd,
			     struct nxs_v4l2_test_common_option *option);
int nxs_v4l2_test_common_flyby(int fd,
			       struct nxs_v4l2_test_common_option *option);

#ifdef __cplusplus
}
#endif

#endif
