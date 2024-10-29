////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Intel Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
// SPDX-License-Identifier: MIT
//
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#include "ici.h"
#include "icitest.h"
#include "icitest_time.h"
#include "icitest_graph.h"
#include "icitest_stream.h"

int first_frame_received = 0;
int first_frame_rendered = 0;

int running = 1;

struct timeval *curr_time, *prev_time;

int pixelformat;

int stream_id=-1;
int m_ICIEnabled = 1;
struct wl_display *g_display_connection = NULL;
struct ici_stream_format stream_fmt;

static void polling_thread(void *data)
{
	struct display *display = (struct display *)data;
	struct pollfd fd;
	unsigned int received_frames;
	unsigned int total_received_frames;
	struct timeval prev_time_th, curr_time_th;
	struct timeval time_diff;
	struct timeval tmp;
	float time_diff_secs;
	int is_topbuf = 1;
	int prev_top_idx = -1;
	fd.fd = display->strm_fd;
	fd.events = POLLIN;

	gettimeofday(&prev_time_th, NULL);
	received_frames = 0;
	total_received_frames = 0;
	while(running) {
		if(poll(&fd, 1, 5000) > 0) {
			if(fd.revents & POLLIN) {
				int buf_idx = dequeue_buffer(fd.fd, display->s->mem_type,
						&is_topbuf);

				if(buf_idx > display->s->buffer_count || buf_idx < 0)
				{
					fprintf(stderr,"Failed to Deque Buffer\n");
					break;
				}

				display->buffers[buf_idx].is_top = is_topbuf;
				if(display->s->interlaced){
					if(is_topbuf)
						prev_top_idx = buf_idx;
					else {
						if(prev_top_idx < 0)
							printf("***Warning Top buffer not received ****\n");
						else {
							display->disp_bufs[0] = &display->buffers[prev_top_idx];
							display->disp_bufs[1] = &display->buffers[buf_idx];
							prev_top_idx=-1;
						}
					}
				} else {
						display->disp_bufs[0] = &display->buffers[buf_idx];
				}
				queue_buffer(fd.fd, &display->buffers[buf_idx],
						display->s->mem_type);
				if (first_frame_received == 0) {
					first_frame_received = 1;
					GET_TS(time_measurements.first_frame_time);
				}

				received_frames++;
				total_received_frames++;
				if (display->s->frames_count != 0 &&
						total_received_frames >= display->s->frames_count) {
					running = 0;
				}
				gettimeofday(&curr_time_th, NULL);
				timersub(&curr_time_th, &prev_time_th, &time_diff);
				time_diff_secs = (time_diff.tv_sec * 1000 +
						time_diff.tv_usec / 1000) / 1000;

				if (time_diff_secs >= TARGET_NUM_SECONDS) {
					fprintf(stdout, "Received %d frames from IPU in %6.3f seconds = %6.3f FPS\n",
							received_frames, time_diff_secs, received_frames / time_diff_secs);
					fflush(stdout);

					received_frames = 0;

					tmp = prev_time_th;
					prev_time_th = curr_time_th;
					curr_time_th = tmp;
				}
			}
		}
	}
}


void format_setup(struct setup *s)
{
	if(stream_id>=0)
		sprintf(s->stream, "/dev/%s%d",
			ICI_STREAM_DEVICE_NAME, stream_id);
	else
		sprintf(s->stream, "/dev/%s%d",
			ICI_STREAM_DEVICE_NAME, DEFAULT_STREAM_ID);

	stream_fmt.ffmt.field = ICI_FIELD_NONE;
	stream_fmt.ffmt.pixelformat = pixelformat;
	switch (s->stream_input) {
		case HDMI_INPUT:
			if (s->isys_w && s->isys_h) {
				stream_fmt.ffmt.width = s->isys_w;
				stream_fmt.ffmt.height = s->isys_h;
			} else {
				stream_fmt.ffmt.width = 1920;
				stream_fmt.ffmt.height = 1080;
			}
			if (!stream_fmt.ffmt.pixelformat)
				stream_fmt.ffmt.pixelformat = ICI_FORMAT_RGB888;
			s->port = 0;
			break;
		case TPG_INPUT:
			stream_fmt.ffmt.width = 1920;
			stream_fmt.ffmt.height = 1080;
			if (!stream_fmt.ffmt.pixelformat)
				stream_fmt.ffmt.pixelformat = ICI_FORMAT_SGRBG8;
			stream_fmt.pfmt.plane_fmt[0].bytesperline = 1984;
			s->port = 0;
			break;
		case OVTI_INPUT:
			stream_fmt.ffmt.width = 1280;
			stream_fmt.ffmt.height = 720;
			if (!stream_fmt.ffmt.pixelformat)
				stream_fmt.ffmt.pixelformat = ICI_FORMAT_UYVY;
			s->port = 0;
			break;

		default:
			stream_fmt.ffmt.width = 720;
			stream_fmt.ffmt.height = 288;

			if (pixelformat && pixelformat != ICI_FORMAT_UYVY) {
				WARN_ON(1, "wrong pixelformat, only UYVY allowed!!\n");
			}
			if (!stream_fmt.ffmt.pixelformat)
				stream_fmt.ffmt.pixelformat = ICI_FORMAT_UYVY,
			s->port = 4;
			if(s->interlaced)
				stream_fmt.ffmt.field = ICI_FIELD_ALTERNATE;
			break;
	}

	stream_fmt.ffmt.colorspace = 0;
	stream_fmt.pfmt.num_planes = 1;
	stream_fmt.ffmt.flags = 0;

	/* output resolution */
	if(!s->ow || !s->oh) {
		s->ow = 1920;
		s->oh = 1080;
	}
	s->iw = stream_fmt.ffmt.width;
	s->ih = stream_fmt.ffmt.height;
	s->buffer_count = 4;
	s->in_fourcc = stream_fmt.ffmt.pixelformat;
}

int initWlConnection(void)
{
	if(g_display_connection == NULL)
	{
		g_display_connection = wl_display_connect(NULL);
		fprintf(stderr,"wayland connection\n");
	}
	return 0;
}

int disconnectWlConnection(void)
{
	if(g_display_connection != NULL)
	{
		wl_display_disconnect(g_display_connection);
		g_display_connection = NULL;
		fprintf(stderr,"disconnect wayland connection\n");
	}
	return 0;
}

int iciStartDisplay(struct setup param, int io_stream_id, int start, void *gpioclass, int *ici_rdy)
{
	GET_TS(time_measurements.app_start_time);

	struct setup s = param;
	struct display display = { 0 };
	struct window  window  = { 0 };
	int ret = 0;
	pthread_t poll_thread;
	struct stat tmp;
	char wayland_path[255];
	unsigned long buf_size = 0;
	int dev_fd;
	/* if ici still not ready let us wait it till ready*/
	/* with new earlyapp-fastboot, ipu4 modules will finish init
	 * in 950ms after kernel start
	 * */
	if (!(*ici_rdy))
		*ici_rdy = ConfigureICI(true);

	stream_id = io_stream_id;
	format_setup(&s);

	/* open the device */
	dev_fd = open_device(s.stream);
	if(dev_fd == -1)
	{
		fprintf(stderr,"Failed to open device");
		return 0;
	}

	/* Do any specific intilization */
	ret = init_stream(dev_fd);
	if(ret) {
		close_device(dev_fd);
		fprintf(stderr,"Stream Init Failed\n");
		return 0;
	}

    s.stride_width = stream_fmt.pfmt.plane_fmt[0].bytesperline /
		(stream_fmt.pfmt.plane_fmt[0].bpp >> 3);

	/* Setup Buffers */
	struct buffer buffers[s.buffer_count];
	memset(buffers, 0, sizeof(struct buffer) * s.buffer_count);

	if (s.mem_type == ICI_MEM_DMABUF)
		init_gem(&display);

	buf_size = stream_fmt.pfmt.plane_fmt[0].sizeimage;
	printf("bufsize: %lu\n", buf_size);
	allocate_buffers(buffers, buf_size, s.buffer_count, s.mem_type, &display);

	window.display = &display;
	display.window = &window;
	window.window_size.width  = s.ow;
	window.window_size.height = s.oh;
	window.fullscreen = s.fullscreen;
	window.output = 0;
	window.print_fps = 1;
	display.s = &s;
	display.strm_fd = dev_fd;
	display.buffers = buffers;

	/* IPU4_ICI Prepare for streaming */
	if(queue_buffers(dev_fd, s.buffer_count, buffers, s.mem_type) < 0)
		goto cleanup0;

	if(stream_on(dev_fd) < 0)
		goto cleanup0;

	running = start;
	GET_TS(time_measurements.streamon_time);

	/* IPU4_ICI Start Streaming*/
	if(pthread_create(&poll_thread, NULL,
				(void *) &polling_thread, (void *) &display)) {
		printf("Couldn't create polling thread\n");
	}

	snprintf(wayland_path, 255, "%s/wayland-0", getenv("XDG_RUNTIME_DIR"));
	while (stat(wayland_path, &tmp) != 0) {
		usleep(100);
	}

	GET_TS(time_measurements.weston_init_time);

	display.display = g_display_connection;
	assert(display.display);
	wl_list_init(&display.output_list);

	display.registry = wl_display_get_registry(display.display);
	wl_registry_add_listener(display.registry,
			&registry_listener, &display);

	wl_display_dispatch(display.display);
	wl_display_roundtrip(display.display);

	init_egl(&display, window.opaque);
	create_surface(&window, gpioclass);
	init_gl(&window);

	GET_TS(time_measurements.rendering_init_time);

	prev_time = calloc(1, sizeof(struct timeval));
	if(!prev_time) {
		fprintf(stderr, "Cannot allocate memory: %m\n");
		return 0;
	}

	curr_time = calloc(1, sizeof(struct timeval));
	if(!curr_time) {
		fprintf(stderr, "Cannot allocate memory: %m\n");
		return 0;
	}

	gettimeofday(prev_time, NULL);

	/* Main display loop */

	while (running && ret != -1) {
		ret = wl_display_dispatch(display.display);
	}

	fprintf(stderr, "\nici-test exiting\n");
	printf("\nici-test existing running = %d\n", running);

	pthread_join(poll_thread, NULL);

	free(curr_time);
	free(prev_time);

	cleanup(dev_fd);

cleanup0:
	free_buffers(buffers, s.buffer_count, s.mem_type);

	if (s.mem_type == ICI_MEM_DMABUF)
		destroy_gem(&display);

	destroy_surface(&window);

	close_device(dev_fd);

	wl_shell_destroy(display.wl_shell);
	printf("WL_SHELL destroy\n");

	printf("WL_COMPOSITOR destroy\n");
	wl_compositor_destroy(display.compositor);

	wl_display_flush(display.display);

	return 0;
}

void iciStopDisplay(int stop)
{
	running = stop;
}
