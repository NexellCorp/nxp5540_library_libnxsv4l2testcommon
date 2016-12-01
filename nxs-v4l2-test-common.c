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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <nxs-v4l2.h>
#include <nx-drm.h>

#include "nxs-v4l2-test-common.h"

#define MAX_BUFFER_COUNT	16
#define MAX_NUM_PLANES		3

static void set_default_option(struct nxs_v4l2_test_common_option *option)
{
	option->format = v4l2_fourcc('Y', 'U', '1', '2');
	option->buffer_count = 8;
	option->dst_width = 0;
	option->dst_format = v4l2_fourcc('Y', 'U', '1', '2');
	option->dst_buffer_count = 8;
	option->loop_count = 100;
	option->memory = V4L2_MEMORY_DMABUF;
	option->display = false;
}

static void print_help(void)
{
	fprintf(stdout, "Options for test nxs v4l2\n");
	fprintf(stdout, "===================================\n");
	fprintf(stdout, "option\tdescription\ttype\n");
	fprintf(stdout, "-----------------------------------\n");
	fprintf(stdout, "-w\twidth\tinteger\n");
	fprintf(stdout, "-h\theight\tinteger\n");
	fprintf(stdout, "-f\tformat\tstring\n");
	fprintf(stdout, "-b\tbuffer count\tinteger\n");
	fprintf(stdout, "-W\tdst width\tinteger\n");
	fprintf(stdout, "-H\tdst height\tinteger\n");
	fprintf(stdout, "-F\tdst format\tstring\n");
	fprintf(stdout, "-B\tdst buffer count\tinteger\n");
	fprintf(stdout, "-l\tloop count\tinteger\n");
	fprintf(stdout, "-m\tv4l2 memory type\tstring\n");
	fprintf(stdout, "-d\tdisplay on\tbool\n");
	fprintf(stdout, "-q\tprint this\t\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Available formats(see videodev2.h)\n");
	fprintf(stdout, "AR15  => ARGB-1-5-5-5 16bit\n");
	fprintf(stdout, "XR15  => XRGB-1-5-5-5 16bit\n");
	fprintf(stdout, "RGBP  => RGB-5-6-5 16bit\n");
	fprintf(stdout, "BGR3  => BGR-8-8-8 24bit\n");
	fprintf(stdout, "RGB3  => RGB-8-8-8 24bit\n");
	fprintf(stdout, "BGR4  => BGR-8-8-8-8 32bit\n");
	fprintf(stdout, "AR24  => BGRA-8-8-8-8 32bit\n");
	fprintf(stdout, "XR24  => BGRX-8-8-8-8 32bit\n");
	fprintf(stdout, "RBG4  => RGB-8-8-8-8 32bit\n");
	fprintf(stdout, "BA24  => ARGB-8-8-8-8 32bit\n");
	fprintf(stdout, "BX24  => XRGB-8-8-8-8 32bit\n");
	fprintf(stdout, "YUYV  => YUV 4:2:2\n");
	fprintf(stdout, "YYUV  => YUV 4:2:2\n");
	fprintf(stdout, "YVYU  => YVU 4:2:2\n");
	fprintf(stdout, "UYVV  => YUV 4:2:2\n");
	fprintf(stdout, "VYUY  => YUV 4:2:2\n");
	fprintf(stdout, "YV12  => YVU 4:2:0\n");
	fprintf(stdout, "422P  => YUV422 planar\n");
	fprintf(stdout, "YU12  => YUV 4:2:0\n");
	fprintf(stdout, "NV12  => Y/CbCr 4:2:0\n");
	fprintf(stdout, "NV21  => Y/CrCb 4:2:0\n");
	fprintf(stdout, "NV16  => Y/CbCr 4:2:2\n");
	fprintf(stdout, "NV61  => Y/CrCb 4:2:2\n");
	fprintf(stdout, "NV24  => Y/CbCr 4:4:4\n");
	fprintf(stdout, "NV42  => Y/CrCb 4:4:4\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Available memory type(see videodev2.h)\n");
	fprintf(stdout, "dmabuf    => V4L2_MEMORY_DMABUF\n");
	fprintf(stdout, "mmap      => V4L2_MEMORY_MMAP\n");
	fprintf(stdout, "userptr   => V4L2_MEMORY_USERPTR\n");
	fprintf(stdout, "===================================\n");
}

static inline void print_v4l2_format(uint32_t format)
{
	fprintf(stdout, "%c%c%c%c\n",
		format & 0x000000ff,
		(format & 0x0000ff00) >> 8,
		(format & 0x00ff0000) >> 16,
		(format & 0xff000000) >> 24);
}

void
nxs_v4l2_test_common_print_option(struct nxs_v4l2_test_common_option *option)
{
	char *str;

	fprintf(stdout, "NXS V4L2 TEST OPTION\n");
	fprintf(stdout, "========================\n");
	fprintf(stdout, "width: %d\n", option->width);
	fprintf(stdout, "height: %d\n", option->height);
	fprintf(stdout, "format: %c%c%c%c\n",
		option->format & 0x000000ff,
		(option->format & 0x0000ff00) >> 8,
		(option->format & 0x00ff0000) >> 16,
		(option->format & 0xff000000) >> 24);
	fprintf(stdout, "buffer_count: %d\n", option->buffer_count);
	if (option->dst_width) {
		fprintf(stdout, "dst width: %d\n", option->dst_width);
		fprintf(stdout, "dst height: %d\n", option->dst_height);
		fprintf(stdout, "dst format: %c%c%c%c\n",
			option->dst_format & 0x000000ff,
			(option->dst_format & 0x0000ff00) >> 8,
			(option->dst_format & 0x00ff0000) >> 16,
			(option->dst_format & 0xff000000) >> 24);
		fprintf(stdout, "dst buffer_count: %d\n",
			option->dst_buffer_count);
	}
	fprintf(stdout, "loop_count: %d\n", option->loop_count);

	switch (option->memory) {
	case V4L2_MEMORY_DMABUF:
		str = "DMABUF";
		break;
	case V4L2_MEMORY_MMAP:
		str = "MMAP";
		break;
	case V4L2_MEMORY_USERPTR:
		str = "USERPTR";
		break;
	default:
		str = "UNKNOWN";
		break;
	}
	fprintf(stdout, "memory: %s\n", str);

	switch (option->buf_type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		str = "CAPTURE SINGLE";
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		str = "CAPTURE MPLANE";
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		str = "OUTPUT SINGLE";
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		str = "OUTPUT MPLANE";
		break;
	}
	fprintf(stdout, "buf_type: %s\n", str);

	if (option->dst_width) {
		switch (option->dst_buf_type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			str = "CAPTURE SINGLE";
			break;
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
			str = "CAPTURE MPLANE";
			break;
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			str = "OUTPUT SINGLE";
			break;
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
			str = "OUTPUT MPLANE";
			break;
		}
		fprintf(stdout, "dst buf_type: %s\n", str);
	}

	if (option->crop_width && option->crop_height)
		fprintf(stdout, "crop: [%d:%d:%d:%d]\n", option->crop_x,
			option->crop_y, option->crop_width,
			option->crop_height);

	fprintf(stdout, "display: %s\n",
		option->display ? "on" : "off");
}

int nxs_v4l2_test_common_get_option(int argc, char *argv[], uint32_t test_type,
				    struct nxs_v4l2_test_common_option *option)
{
	int opt;

	set_default_option(option);

	while ((opt = getopt(argc, argv, "w:h:f:b:W:H:F:B:l:m:c:dq")) != -1) {
		switch (opt) {
		case 'w':
			option->width = atoi(optarg);
			break;
		case 'h':
			option->height = atoi(optarg);
			break;
		case 'f':
			option->format = v4l2_fourcc(optarg[0], optarg[1],
						     optarg[2], optarg[3]);
			break;
		case 'b':
			option->buffer_count = atoi(optarg);
			break;
		case 'W':
			option->dst_width = atoi(optarg);
			break;
		case 'H':
			option->dst_height = atoi(optarg);
			break;
		case 'F':
			option->dst_format = v4l2_fourcc(optarg[0], optarg[1],
							 optarg[2], optarg[3]);
			break;
		case 'B':
			option->dst_buffer_count = atoi(optarg);
			break;
		case 'l':
			option->loop_count = atoi(optarg);
			break;
		case 'm':
			if (!strncmp(optarg, "dmabuf", strlen("dmabuf"))) {
			} else if (!strncmp(optarg, "mmap", strlen("mmap"))) {
			} else if (!strncmp(optarg,
					    "userptr", strlen("userptr"))) {
			} else {
				fprintf(stderr, "Unknown memory type %s\n",
					optarg);
				return -EINVAL;
			}
			break;
		case 'd':
			option->display = true;
			break;
		case 'c':
			sscanf(optarg, "%d:%d:%d:%d", &option->crop_x,
			       &option->crop_y, &option->crop_width,
			       &option->crop_height);
			break;
		case 'q':
			print_help();
			return -EBUSY;
		}
	}

	switch (test_type) {
	case NXS_V4L2_TEST_TYPE_CAPTURE:
		switch (option->format) {
		case V4L2_PIX_FMT_ARGB555:
		case V4L2_PIX_FMT_XRGB555:
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_BGR24:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR32:
		case V4L2_PIX_FMT_ABGR32:
		case V4L2_PIX_FMT_XBGR32:
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_ARGB32:
		case V4L2_PIX_FMT_XRGB32:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
			option->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			break;
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_NV24:
		case V4L2_PIX_FMT_NV42:
		case V4L2_PIX_FMT_NV12M:
		case V4L2_PIX_FMT_NV21M:
		case V4L2_PIX_FMT_NV16M:
		case V4L2_PIX_FMT_NV61M:
		case V4L2_PIX_FMT_YUV420M:
		case V4L2_PIX_FMT_YVU420M:
			option->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			break;
		default:
			fprintf(stderr, "Unsupported pixel format\n");
			print_v4l2_format(option->format);
			break;
		}
		break;

	case NXS_V4L2_TEST_TYPE_RENDER:
		switch (option->format) {
		case V4L2_PIX_FMT_ARGB555:
		case V4L2_PIX_FMT_XRGB555:
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_BGR24:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR32:
		case V4L2_PIX_FMT_ABGR32:
		case V4L2_PIX_FMT_XBGR32:
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_ARGB32:
		case V4L2_PIX_FMT_XRGB32:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
			option->buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			break;
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_NV24:
		case V4L2_PIX_FMT_NV42:
		case V4L2_PIX_FMT_NV12M:
		case V4L2_PIX_FMT_NV21M:
		case V4L2_PIX_FMT_NV16M:
		case V4L2_PIX_FMT_NV61M:
		case V4L2_PIX_FMT_YUV420M:
		case V4L2_PIX_FMT_YVU420M:
			option->buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
			break;
		default:
			fprintf(stderr, "Unsupported pixel format\n");
			print_v4l2_format(option->format);
			break;
		}
		break;

	case NXS_V4L2_TEST_TYPE_M2M:
		switch (option->format) {
		case V4L2_PIX_FMT_ARGB555:
		case V4L2_PIX_FMT_XRGB555:
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_BGR24:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR32:
		case V4L2_PIX_FMT_ABGR32:
		case V4L2_PIX_FMT_XBGR32:
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_ARGB32:
		case V4L2_PIX_FMT_XRGB32:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
			/* src buf type is OUTPUT */
			option->buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			break;
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_NV24:
		case V4L2_PIX_FMT_NV42:
		case V4L2_PIX_FMT_NV12M:
		case V4L2_PIX_FMT_NV21M:
		case V4L2_PIX_FMT_NV16M:
		case V4L2_PIX_FMT_NV61M:
		case V4L2_PIX_FMT_YUV420M:
		case V4L2_PIX_FMT_YVU420M:
			/* src buf type is OUTPUT */
			option->buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
			break;
		default:
			fprintf(stderr, "Unsupported pixel format\n");
			print_v4l2_format(option->format);
			break;
		}

		switch (option->dst_format) {
		case V4L2_PIX_FMT_ARGB555:
		case V4L2_PIX_FMT_XRGB555:
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_BGR24:
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR32:
		case V4L2_PIX_FMT_ABGR32:
		case V4L2_PIX_FMT_XBGR32:
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_ARGB32:
		case V4L2_PIX_FMT_XRGB32:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
			/* dst buf type is CAPTURE */
			option->dst_buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			break;
		case V4L2_PIX_FMT_YVU420:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_NV24:
		case V4L2_PIX_FMT_NV42:
		case V4L2_PIX_FMT_NV12M:
		case V4L2_PIX_FMT_NV21M:
		case V4L2_PIX_FMT_NV16M:
		case V4L2_PIX_FMT_NV61M:
		case V4L2_PIX_FMT_YUV420M:
		case V4L2_PIX_FMT_YVU420M:
			/* dst buf type is CAPTURE */
			option->dst_buf_type =
				V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			break;
		default:
			fprintf(stderr, "Unsupported pixel format\n");
			print_v4l2_format(option->dst_format);
			break;
		}
		break;
	}

	return 0;
}

static int v4l2_test_common(int fd, int drm_fd,
			    struct nx_gem_buffer gem_buffer[],
			    struct nxs_v4l2_test_common_option *option)
{
	int ret;
	struct v4l2_capability cap;
	struct v4l2_fmtdesc desc;
	int i;
	uint32_t buf_type, memory, buf_count;
	uint32_t w, h, f;
	struct nx_gem_buffer *buf;

	buf_type = option->buf_type;
	memory = option->memory;
	buf_count = option->buffer_count;
	w = option->width;
	h = option->height;
	f = option->format;


	/* alloc buffers */
	for (i = 0; i < buf_count; i++) {
		buf = &gem_buffer[i];
		bzero(buf, sizeof(*buf));

		/* fprintf(stdout, "drm_fd %d, w %d, h %d, f 0x%x, buf %p\n", */
		/* 	drm_fd, w, h, f, buf); */
		ret = nx_drm_alloc_nx_gem(drm_fd, w, h, f, buf);
		if (ret)
			return ret;
	}

	/* query cap */
	ret = nxs_v4l2_querycap(fd, &cap);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_query_cap\n");
		return ret;
	}

	fprintf(stdout, "===============================\n");
	fprintf(stdout, "V4L2 CAPABILITY\n");
	fprintf(stdout, "===============================\n");
	fprintf(stdout, "driver:\t%s\n", cap.driver);
	fprintf(stdout, "card:\t%s\n", cap.card);
	fprintf(stdout, "bus_info:\t%s\n", cap.bus_info);
	fprintf(stdout, "version:\t0x%x\n", cap.version);
	fprintf(stdout, "capabilities:\t0x%x\n", cap.capabilities);
	fprintf(stdout, "device_caps:\t0x%x\n", cap.device_caps);

	/* enum format */
	fprintf(stdout, "===============================\n");
	fprintf(stdout, "V4L2 ENUM FORMAT\n");
	fprintf(stdout, "===============================\n");
	i = 0;
	while (1) {
		bzero(&desc, sizeof(desc));

		desc.index = i;
		desc.type  = buf_type;

		ret = nxs_v4l2_enum_format(fd, &desc);
		if (ret)
			break;

		fprintf(stdout, "index:\t%d\n", desc.index);
		fprintf(stdout, "type:\t0x%x\n", desc.type);
		fprintf(stdout, "flags:\t0x%x\n", desc.flags);
		fprintf(stdout, "description:\t%s\n", desc.description);
		fprintf(stdout, "pixelformat:\t0x%x\n", desc.pixelformat);
		print_v4l2_format(desc.pixelformat);

		i++;
	}

	/* try format */
	ret = nxs_v4l2_try_format(fd, buf_type, w, h, f);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_try_format\n");
		return ret;
	}

	/* set format */
	buf = &gem_buffer[0];
	ret = nxs_v4l2_set_format(fd, buf_type, w, h, f, buf->num_planes,
				  buf->strides, buf->sizes);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_set_format\n");
		return ret;
	}

	/* get format */
	ret = nxs_v4l2_get_format(fd, buf_type, &w, &h, &f);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_get_format\n");
		return ret;
	}

	/* TODO: check no hardcoding */
	if (w != option->width &&
	    h != option->height &&
	    f != option->format) {
		fprintf(stderr, "format mismatch: %d, %d, 0x%x\n",
			w, h, f);
		print_v4l2_format(f);
		return -EINVAL;
	}

	/* reqbuf */
	ret = nxs_v4l2_reqbuf(fd, buf_type, memory, buf_count);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_reqbuf\n");
		return ret;
	}

	return 0;
}

static int v4l2_test_qbuf(int fd, int index, uint32_t buf_type, uint32_t memory,
			  struct nx_gem_buffer *buf)
{
	struct v4l2_buffer v4l2_buf;
	struct v4l2_plane v4l2_planes[MAX_NUM_PLANES];
	int i;
	int ret;

	bzero(v4l2_planes, sizeof(struct v4l2_plane) * MAX_NUM_PLANES);
	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.type = buf_type;
	v4l2_buf.memory = memory;
	v4l2_buf.index = index;

	switch (memory) {
	case V4L2_MEMORY_DMABUF:
		for (i = 0; i < buf->num_planes; i++) {
			v4l2_planes[i].m.fd = buf->dma_fds[i];
			v4l2_planes[i].bytesused =
				v4l2_planes[i].length =
				buf->sizes[i];
		}

		v4l2_buf.length = buf->num_planes;
		v4l2_buf.m.planes = v4l2_planes;
		break;

		/* TODO: userptr */
		/* case V4L2_MEMORY_USERPTR: */
		/* 	#<{(| TODO: how to handle userptr buffer? |)}># */
		/* 	v4l2_buf.length = 1; */
		/* 	v4l2_buf.m.planes[0].m.userptr = 0; */
		/* 	v4l2_buf.m.planes[0].length = buffer_size; */
		/* 	break; */
	}

	ret = nxs_v4l2_qbuf(fd, &v4l2_buf);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_qbuf\n");
		return ret;
	}

	return 0;
}

static int v4l2_test_dqbuf(int fd, int *index, uint32_t buf_type,
			   uint32_t memory, struct nx_gem_buffer *buf)
{
	int ret;
	struct v4l2_buffer v4l2_buf;
	struct v4l2_plane v4l2_planes[MAX_NUM_PLANES];

	bzero(v4l2_planes, sizeof(struct v4l2_plane) * MAX_NUM_PLANES);
	bzero(&v4l2_buf, sizeof(v4l2_buf));
	v4l2_buf.type = buf_type;
	v4l2_buf.memory = memory;

	switch (memory) {
	case V4L2_MEMORY_DMABUF:
		v4l2_buf.length = buf->num_planes;
		v4l2_buf.m.planes = v4l2_planes;
		break;

		/* TODO: userptr */
		/* case V4L2_MEMORY_USERPTR: */
		/* 	#<{(| TODO: how to handle userptr buffer? |)}># */
		/* 	v4l2_buf.length = 1; */
		/* 	v4l2_buf.m.planes[0].length = buffer_size; */
		/* 	break; */
	}

	ret = nxs_v4l2_dqbuf(fd, &v4l2_buf);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_dqbuf\n");
		return ret;
	}

	*index = v4l2_buf.index;

	return 0;
}

static int v4l2_test_close(int fd, int drm_fd, uint32_t buf_type,
			   uint32_t buf_count,
			   struct nx_gem_buffer gem_buffer[])
{
	int ret;
	int i, j;
	struct nx_gem_buffer *buf;

	/* stream off */
	ret = nxs_v4l2_streamoff(fd, buf_type);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamoff\n");
		return ret;
	}

	/* free gem */
	for (i = 0; i < buf_count; i++) {
		buf = &gem_buffer[i];

		for (j = 0; j < buf->num_planes; j++)
			nx_drm_free_gem(drm_fd, buf->gem_fds[j]);
	}

	return 0;
}

int nxs_v4l2_test_common_capture(int fd,
				 struct nxs_v4l2_test_common_option *option)
{
	int ret;
	int i;
	uint32_t buf_type, memory, buf_count;
	int drm_fd;
	int loop_count;
	struct nx_gem_buffer gem_buffer[MAX_BUFFER_COUNT];
	struct nx_gem_buffer *buf;

	/* open drm device */
	drm_fd = nx_drm_open_device();
	if (drm_fd < 0) {
		fprintf(stderr, "failed to nx_drm_open_device\n");
		return -ENODEV;
	}

	ret = v4l2_test_common(fd, drm_fd, gem_buffer, option);
	if (ret)
		return ret;

	buf_type = option->buf_type;
	memory = option->memory;
	buf_count = option->buffer_count;
	loop_count = option->loop_count;

	/* qbuf */
	for (i = 0; i < buf_count; i++) {
		ret = v4l2_test_qbuf(fd, i, buf_type, memory, &gem_buffer[i]);
		if (ret)
			return ret;
	}

	/* stream on */
	ret = nxs_v4l2_streamon(fd, buf_type);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamon\n");
		return ret;
	}

	/* dq buf */
	buf = &gem_buffer[0];
	while (loop_count--) {
		int index;

		ret = v4l2_test_dqbuf(fd, &index, buf_type, memory, buf);
		if (ret)
			return ret;

		fprintf(stdout, "dq index: %d\n", index);

		buf = &gem_buffer[index];

		ret = v4l2_test_qbuf(fd, index, buf_type, memory,
				     &gem_buffer[index]);
		if (ret)
			return ret;
	}

	/* stream off */
	ret = v4l2_test_close(fd, drm_fd, buf_type, buf_count, gem_buffer);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamoff\n");
		return ret;
	}

	return 0;
}

int nxs_v4l2_test_common_render(int fd,
				struct nxs_v4l2_test_common_option *option)
{
	int ret;
	uint32_t buf_type, memory, buf_count;
	int drm_fd;
	int loop_count;
	struct nx_gem_buffer gem_buffer[MAX_BUFFER_COUNT];
	struct nx_gem_buffer *buf;
	bool started = false;
	int out_index = 0;
	int out_dq_index = 0;
	int out_q_count = 0;

	/* open drm device */
	drm_fd = nx_drm_open_device();
	if (drm_fd < 0) {
		fprintf(stderr, "failed to nx_drm_open_device\n");
		return -ENODEV;
	}

	ret = v4l2_test_common(fd, drm_fd, gem_buffer, option);
	if (ret)
		return ret;

	buf_type = option->buf_type;
	memory = option->memory;
	buf_count = option->buffer_count;
	loop_count = option->loop_count;

	while (loop_count--) {
		buf = &gem_buffer[out_index];

		ret = v4l2_test_qbuf(fd, out_index, buf_type, memory, buf);
		if (ret)
			return ret;

		out_q_count++;
		out_index++;
		out_index %= buf_count;

		if (!started) {
			ret = nxs_v4l2_streamon(fd, buf_type);
			if (ret) {
				fprintf(stderr, "failed to nxs_v4l2_streamon\n");
				return ret;
			}

			started = true;
		}

		if (out_q_count >= 2) {
			ret = v4l2_test_dqbuf(fd, &out_dq_index, buf_type,
					      memory, buf);
			if (ret)
				return ret;

			out_q_count--;
		}

		fprintf(stdout, "dq index: %d\n", out_dq_index);
	}

	/* stream off */
	ret = v4l2_test_close(fd, drm_fd, buf_type, buf_count, gem_buffer);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamoff\n");
		return ret;
	}

	return 0;
}

int nxs_v4l2_test_common_m2m(int fd,
			     struct nxs_v4l2_test_common_option *option)
{
	int ret;
	int drm_fd;
	int loop_count;
	struct nx_gem_buffer src_gem_buffer[MAX_BUFFER_COUNT];
	struct nx_gem_buffer dst_gem_buffer[MAX_BUFFER_COUNT];
	struct nx_gem_buffer *src_buf, *dst_buf;
	bool started;
	int src_q_index, src_dq_index;
	int dst_q_index, dst_dq_index;
	struct v4l2_capability cap;
	struct v4l2_fmtdesc desc;
	int i;
	uint32_t w, h, f;

	/* open drm device */
	drm_fd = nx_drm_open_device();
	if (drm_fd < 0) {
		fprintf(stderr, "failed to nx_drm_open_device\n");
		return -ENODEV;
	}

	/* alloc buffers */
	for (i = 0; i < option->buffer_count; i++) {
		src_buf = &src_gem_buffer[i];
		bzero(src_buf, sizeof(*src_buf));

		ret = nx_drm_alloc_nx_gem(drm_fd, option->width, option->height,
					  option->format, src_buf);
		if (ret)
			return ret;
	}

	for (i = 0; i < option->dst_buffer_count; i++) {
		dst_buf = &dst_gem_buffer[i];
		bzero(dst_buf, sizeof(*dst_buf));

		ret = nx_drm_alloc_nx_gem(drm_fd, option->dst_width,
					  option->dst_height, option->format,
					  dst_buf);
		if (ret)
			return ret;
	}

	/* query cap */
	ret = nxs_v4l2_querycap(fd, &cap);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_query_cap\n");
		return ret;
	}

	fprintf(stdout, "===============================\n");
	fprintf(stdout, "V4L2 CAPABILITY\n");
	fprintf(stdout, "===============================\n");
	fprintf(stdout, "driver:\t%s\n", cap.driver);
	fprintf(stdout, "card:\t%s\n", cap.card);
	fprintf(stdout, "bus_info:\t%s\n", cap.bus_info);
	fprintf(stdout, "version:\t0x%x\n", cap.version);
	fprintf(stdout, "capabilities:\t0x%x\n", cap.capabilities);
	fprintf(stdout, "device_caps:\t0x%x\n", cap.device_caps);

#if 0
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr,
			"m2m device must support V4L2_CAP_VIDEO_CAPTURE\n");
		return -EINVAL;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
		fprintf(stderr,
			"m2m device must support V4L2_CAP_VIDEO_OUTPUT\n");
		return -EINVAL;
	}
#else
	if (!(cap.capabilities &
	      (V4L2_CAP_VIDEO_M2M | V4L2_CAP_VIDEO_M2M_MPLANE))) {
		fprintf(stderr,
			"m2m device must support V4L2_CAP_VIDEO_M2M\n");
		return -EINVAL;
	}
#endif

	/* enum format */
	fprintf(stdout, "===============================\n");
	fprintf(stdout, "V4L2 ENUM FORMAT for src path\n");
	fprintf(stdout, "===============================\n");
	i = 0;
	while (1) {
		bzero(&desc, sizeof(desc));

		desc.index = i;
		desc.type  = option->buf_type;

		ret = nxs_v4l2_enum_format(fd, &desc);
		if (ret)
			break;

		fprintf(stdout, "index:\t%d\n", desc.index);
		fprintf(stdout, "type:\t0x%x\n", desc.type);
		fprintf(stdout, "flags:\t0x%x\n", desc.flags);
		fprintf(stdout, "description:\t%s\n", desc.description);
		fprintf(stdout, "pixelformat:\t0x%x\n", desc.pixelformat);
		print_v4l2_format(desc.pixelformat);

		i++;
	}

	fprintf(stdout, "===============================\n");
	fprintf(stdout, "V4L2 ENUM FORMAT for dst path\n");
	fprintf(stdout, "===============================\n");
	i = 0;
	while (1) {
		bzero(&desc, sizeof(desc));

		desc.index = i;
		desc.type  = option->dst_buf_type;

		ret = nxs_v4l2_enum_format(fd, &desc);
		if (ret)
			break;

		fprintf(stdout, "index:\t%d\n", desc.index);
		fprintf(stdout, "type:\t0x%x\n", desc.type);
		fprintf(stdout, "flags:\t0x%x\n", desc.flags);
		fprintf(stdout, "description:\t%s\n", desc.description);
		fprintf(stdout, "pixelformat:\t0x%x\n", desc.pixelformat);
		print_v4l2_format(desc.pixelformat);

		i++;
	}

	/* try format */
	ret = nxs_v4l2_try_format(fd, option->buf_type, option->width,
				  option->height, option->format);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_try_format for src\n");
		return ret;
	}

	ret = nxs_v4l2_try_format(fd, option->dst_buf_type, option->dst_width,
				  option->dst_height, option->dst_format);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_try_format for dst\n");
		return ret;
	}

	/* set format */
	src_buf = &src_gem_buffer[0];
	ret = nxs_v4l2_set_format(fd, option->buf_type, option->width,
				  option->height, option->format,
				  src_buf->num_planes, src_buf->strides,
				  src_buf->sizes);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_set_format for src\n");
		return ret;
	}

	dst_buf = &dst_gem_buffer[0];
	ret = nxs_v4l2_set_format(fd, option->dst_buf_type, option->dst_width,
				  option->dst_height, option->dst_format,
				  dst_buf->num_planes, dst_buf->strides,
				  dst_buf->sizes);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_set_format for dst\n");
		return ret;
	}

	/* get format */
	ret = nxs_v4l2_get_format(fd, option->buf_type, &w, &h, &f);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_get_format for src\n");
		return ret;
	}

	if (w != option->width &&
	    h != option->height &&
	    f != option->format) {
		fprintf(stderr, "src format mismatch: %d, %d, 0x%x\n",
			w, h, f);
		print_v4l2_format(f);
		return -EINVAL;
	}

	ret = nxs_v4l2_get_format(fd, option->dst_buf_type, &w, &h, &f);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_get_format for dst\n");
		return ret;
	}

	if (w != option->dst_width &&
	    h != option->dst_height &&
	    f != option->dst_format) {
		fprintf(stderr, "dst format mismatch: %d, %d, 0x%x\n",
			w, h, f);
		print_v4l2_format(f);
		return -EINVAL;
	}

	/* reqbuf */
	ret = nxs_v4l2_reqbuf(fd, option->buf_type, option->memory,
			      option->buffer_count);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_reqbuf for src\n");
		return ret;
	}

	ret = nxs_v4l2_reqbuf(fd, option->dst_buf_type, option->memory,
			      option->dst_buffer_count);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_reqbuf for dst\n");
		return ret;
	}

	src_q_index = 0;
	src_dq_index = 0;
	dst_q_index = 0;
	dst_dq_index = 0;
	started = false;
	loop_count = option->loop_count;
	while (loop_count--) {
		src_buf = &src_gem_buffer[src_q_index];
		ret = v4l2_test_qbuf(fd, src_q_index, option->buf_type,
				     option->memory, src_buf);
		if (ret) {
			fprintf(stderr, "failed to qbuf for src, index %d\n",
				src_q_index);
			return ret;
		}
		src_q_index++;
		src_q_index %= option->buffer_count;

		dst_buf = &dst_gem_buffer[dst_q_index];
		ret = v4l2_test_qbuf(fd, dst_q_index, option->dst_buf_type,
				     option->memory, dst_buf);
		if (ret) {
			fprintf(stderr, "failed to qbuf for dst, index %d\n",
				dst_q_index);
			return ret;
		}
		dst_q_index++;
		dst_q_index %= option->dst_buffer_count;

		if (!started) {
			ret = nxs_v4l2_streamon(fd, option->buf_type);
			if (ret) {
				fprintf(stderr, "failed to nxs_v4l2_streamon for src\n");
				return ret;
			}
			ret = nxs_v4l2_streamon(fd, option->dst_buf_type);
			if (ret) {
				fprintf(stderr, "failed to nxs_v4l2_streamon for dst\n");
				return ret;
			}

			started = true;
		}

		ret = v4l2_test_dqbuf(fd, &src_dq_index, option->buf_type,
				      option->memory, src_buf);
		if (ret) {
			fprintf(stderr, "failed to dqbuf for src\n");
			return ret;
		}
		fprintf(stdout, "src dq index: %d\n", src_dq_index);

		ret = v4l2_test_dqbuf(fd, &dst_dq_index, option->dst_buf_type,
				      option->memory, dst_buf);
		if (ret) {
			fprintf(stderr, "failed to dqbuf for dst\n");
			return ret;
		}
		fprintf(stdout, "dst dq index: %d\n", dst_dq_index);
	}

	/* stream off */
	ret = v4l2_test_close(fd, drm_fd, option->buf_type,
			      option->buffer_count, src_gem_buffer);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamoff for src\n");
		return ret;
	}

	ret = v4l2_test_close(fd, drm_fd, option->dst_buf_type,
			      option->dst_buffer_count, dst_gem_buffer);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_streamoff for dst\n");
		return ret;
	}

	return 0;
}

int nxs_v4l2_test_common_flyby(int fd,
			       struct nxs_v4l2_test_common_option *option)
{
	int ret;
	int loop_count;

	ret = nxs_v4l2_subdev_set_format(fd, option->width, option->height,
					 option->format, V4L2_FIELD_NONE);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_subdev_set_format\n");
		return ret;
	}

	if (option->dst_width > 0) {
		ret = nxs_v4l2_subdev_set_dstformat(fd, option->dst_width,
						    option->dst_height,
						    option->dst_format,
						    V4L2_FIELD_NONE);
		if (ret) {
			fprintf(stderr, "failed to nxs_v4l2_subdev_set_dstformat\n");
			return ret;
		}
	}

	if (option->crop_width > 0) {
		ret = nxs_v4l2_subdev_set_crop(fd, option->crop_x,
					       option->crop_y,
					       option->crop_width,
					       option->crop_height);
		if (ret) {
			fprintf(stderr, "failed to nxs_v4l2_subdev_set_crop\n");
			return ret;
		}
	}

	ret = nxs_v4l2_subdev_start(fd);
	if (ret) {
		fprintf(stderr, "failed to nxs_v4l2_subdev_start\n");
		return ret;
	}

	loop_count = option->loop_count;
	while (loop_count >= 0) {
		loop_count--;
		usleep(10000);
	}

	return nxs_v4l2_subdev_stop(fd);
}
