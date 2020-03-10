/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>

#include <linux/fb.h>
#include <linux/kd.h>
#define RESIZE 4
#define RESIZE_W 4
#define  Up_amplify 20

struct simple_fb {
    void *data;
    int width;
    int height;
    int stride;
    int bpp;
};

static struct simple_fb gr_fbs[2];
static unsigned gr_active_fb = 0;

static int gr_fb_fd = -1;
static int gr_vt_fd = -1;

static struct fb_var_screeninfo vi;
struct fb_fix_screeninfo fi;
struct timespec tv, tv2;

static void dumpinfo(struct fb_fix_screeninfo *fi,
                     struct fb_var_screeninfo *vi);


void Navi_matResize(uint8_t* src, int32_t srcheight, int32_t srcwidth, uint8_t* dst, int32_t dstheight, int32_t dstwidth)
{
	uint8_t* dataDst = (uint8_t*)dst;
	int32_t stepDst = dstwidth;
	uint8_t* dataSrc = (uint8_t*)src;
	int32_t stepSrc = srcwidth;
	int32_t iWidthSrc = srcwidth;
	int32_t iHiehgtSrc = srcheight;
	int32_t i, j;
	int64_t sx, sy;
	//int  scale_y = srcheight*(1<<(Up_amplify-1))/dstheight;//0.5;
	//int  scale_x =  srcwidth*(1<<(Up_amplify-1))/dstwidth;//0.5;
	int32_t  scale_y = srcheight * 1000 / dstheight;//;
	int32_t  scale_x = srcwidth * 1000 / dstwidth;//;
	int64_t fx, fy;
#if Up_amplify > 8
	int64_t cbufy[2];
	int64_t cbufx[2];
#else
	uint32_t cbufx[2];
	uint32_t cbufy[2];
#endif

	for (j = 0; j < dstheight; ++j)
	{
		fy = ((j << 1) + 1)*scale_y - 1000;
		fy = ((fy << (Up_amplify - 1)) / 1000);
		sy = fy >> (Up_amplify);
		fy -= (sy << Up_amplify);

		sy = sy < (iHiehgtSrc - 2) ? sy : (iHiehgtSrc - 2);
		sy = 0 > sy ? 0 : sy;

		cbufy[0] = (1 << Up_amplify) - fy;// cv::saturate_cast<short>((1.f - fy) * 2048);  
		cbufy[1] = fy;//2048 - cbufy[0];  

		for (i = 0; i < dstwidth; ++i)
		{
			fx = ((i << 1) + 1)*scale_x - 1000;
			fx = ((fx << (Up_amplify - 1)) / 1000);
			sx = fx >> (Up_amplify);
			fx -= (sx << Up_amplify);
			//printf("i=%d----j=%d\n",i,j);
			if (sx < 0) {
				fx = 0, sx = 0;
			}
			if (sx >= iWidthSrc - 1) {
				fx = 0, sx = iWidthSrc - 2;
			}
			cbufx[0] = (1 << Up_amplify) - fx;// cv::saturate_cast<short>((1.f - fx) * 2048);  
			cbufx[1] = fx;// 2048 - cbufx[0];  
			*(dataDst + j*stepDst + i) = (uint8_t)((*(dataSrc + sy*stepSrc + sx) * cbufx[0] * cbufy[0] +
				*(dataSrc + (sy + 1)*stepSrc + sx) * cbufx[0] * cbufy[1] +
				*(dataSrc + sy*stepSrc + (sx + 1)) * cbufx[1] * cbufy[0] +
				*(dataSrc + (sy + 1)*stepSrc + (sx + 1)) * cbufx[1] * cbufy[1] + (1LL << (Up_amplify * 2 - 1))) >> (Up_amplify * 2));
		}
	}
}


static int get_framebuffer(struct simple_fb *fb, unsigned bpp)
{
    int fd;
    void *bits;
    int bytes_per_pixel;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        printf("cannot open /dev/graphics/fb0, retrying with /dev/fb0\n");
        if ((fd = open("/dev/fb0", O_RDWR)) < 0) {
            perror("cannot open /dev/fb0");
            return -1;
        }
    }
    ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
    if(ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        perror("failed to get fb0 info");
        return -1;
    }

    if (bpp && vi.bits_per_pixel != bpp) {
        printf("bpp != %d, forcing...\n", bpp);
        vi.bits_per_pixel = bpp;
        if(ioctl(fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
            perror("failed to force bpp");
            return -1;
        }
    }

    if(ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
        perror("failed to get fb0 info");
        return -1;
    }

    dumpinfo(&fi, &vi);

    bits = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(bits == MAP_FAILED) {
        perror("failed to mmap framebuffer");
        return -1;
    }

    bytes_per_pixel = vi.bits_per_pixel >> 3;

    fb->width = vi.xres;
    fb->height = vi.yres;
    fb->stride = fi.line_length / bytes_per_pixel;
    fb->data = bits;
    fb->bpp = vi.bits_per_pixel;

    fb++;

    fb->width = vi.xres;
    fb->height = vi.yres;
    fb->stride = fi.line_length / bytes_per_pixel;
    fb->data = (void *)((unsigned long)bits +
                        vi.yres * vi.xres * bytes_per_pixel);
    fb->bpp = vi.bits_per_pixel;

    return fd;
}

static void set_active_framebuffer(unsigned n)
{
    if(n > 1) return;
    vi.yres_virtual = vi.yres * 2;
    vi.yoffset = n * vi.yres;
    vi.activate = FB_ACTIVATE_FORCE | FB_ACTIVATE_NOW;
    if(ioctl(gr_fb_fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
        fprintf(stderr,"active fb swap failed!\n");
    } 
}

static void dumpinfo(struct fb_fix_screeninfo *fi, struct fb_var_screeninfo *vi)
{
    fprintf(stderr,"vi.xres = %d\n", vi->xres);
    fprintf(stderr,"vi.yres = %d\n", vi->yres);
 /*   fprintf(stderr,"vi.xresv = %d\n", vi->xres_virtual);
    fprintf(stderr,"vi.yresv = %d\n", vi->yres_virtual);
    fprintf(stderr,"vi.xoff = %d\n", vi->xoffset);
    fprintf(stderr,"vi.yoff = %d\n", vi->yoffset);
    fprintf(stderr, "vi.bits_per_pixel = %d\n", vi->bits_per_pixel);

    fprintf(stderr, "fi.line_length = %d\n", fi->line_length);
*/
}

int gr_init(int bpp, int id)
{
    int fd = -1;

    if (!access("/dev/tty0", F_OK)) {
        fd = open("/dev/tty0", O_RDWR | O_SYNC);
        if(fd < 0)
            return -1;

        if(ioctl(fd, KDSETMODE, (void*) KD_GRAPHICS)) {
            close(fd);
            return -1;
        }
    }

    gr_fb_fd = get_framebuffer(gr_fbs, bpp);

    if(gr_fb_fd < 0) {
        if (fd >= 0) {
            ioctl(fd, KDSETMODE, (void*) KD_TEXT);
            close(fd);
        }
        return -1;
    }

    gr_vt_fd = fd;

        /* start with 0 as front (displayed) and 1 as back (drawing) */
    gr_active_fb = id;
    //set_active_framebuffer(id);

    return 0;
}

void gr_exit(void)
{
    close(gr_fb_fd);
    gr_fb_fd = -1;

    if (gr_vt_fd >= 0) {
        ioctl(gr_vt_fd, KDSETMODE, (void*) KD_TEXT);
        close(gr_vt_fd);
        gr_vt_fd = -1;
    }
}

int gr_fb_width(void)
{
    return gr_fbs[0].width;
}

int gr_fb_height(void)
{
    return gr_fbs[0].height;
}

uint16_t red = 0xf800;
uint16_t green = 0x07e0;
uint16_t blue = 0x001f;
uint16_t white = 0xffff;
uint16_t black = 0x0;

uint32_t red32 = 0x00ff0000;
uint32_t green32 = 0x0000ff00;
uint32_t blue32 = 0x000000ff;
uint32_t white32 = 0x00ffffff;
uint32_t black32 = 0x0;

void draw_grid(int w, int h, void* _loc) {
    int i, j;
    int v;
    int stride = fi.line_length / (vi.bits_per_pixel >> 3);
    uint16_t *loc = _loc;
    uint32_t *loc32 = _loc;

    for (j = 0; j < h/2; j++) {
        for (i = 0; i < w/2; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = red;
            else
                loc32[i + j*(stride)] = red32;
        for (; i < w; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = green;
            else
                loc32[i + j*(stride)] = green32;
    }

    for (; j < h; j++) {
        for (i = 0; i < w/2; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = blue;
            else
                loc32[i + j*(stride)] = blue32;
        for (; i < w; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = white;
            else
                loc32[i + j*(stride)] = white32;
    }

}
void draw_image(int w,int h,void* image,void* finger)
{
	int i,j;
    int stride = fi.line_length / (vi.bits_per_pixel >> 3);
    uint32_t pix=0;
    uint32_t qix=0;
	uint16_t *loc = gr_fbs[0].data;
    uint32_t *loc32 = gr_fbs[0].data;
	uint8_t *p=NULL;
	uint8_t *q=NULL;
	p = (uint8_t *)malloc(h*RESIZE*w*RESIZE_W);
    q = (uint8_t *)malloc(h*RESIZE*w*RESIZE_W);
	//memcpy(p,image,w*h);
   if(RESIZE==1 && RESIZE_W==1)
	{
		//printf("not zoom in \n");
		memcpy(p,image,w*h);
		if(finger!=NULL)
			memcpy(q,finger,w*h);
	}
	else
	{
		Navi_matResize(image,h,w,p,h*RESIZE,w*RESIZE_W);
		if(finger!=NULL)
		Navi_matResize(finger,h,w,q,h*RESIZE,w*RESIZE);
    
    		h=RESIZE*h;
    		w=RESIZE_W*w;
   	}
	//printf("w=%d,h=%d\n",w*RESIZE_W,h*RESIZE);
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = pix;
            else{
		        pix=p[i + j*(w)]|(p[i + j*(w)]<<8)|(p[i + j*(w)]<<16);
                loc32[i + j*(stride)] = pix;

	        }

    if(finger!=NULL){
	for (j = 0; j < h; j++)
		for (i = 0; i < w; i++)
			if (vi.bits_per_pixel == 16)
				loc[i + j*(stride)] = qix;
			else{
				qix=q[i + j*(w)]|(q[i + j*(w)]<<8)|(q[i + j*(w)]<<16);
				loc32[i + j*(stride)+(h+5)*(stride)] = qix;
			 }
			 
    }
    set_active_framebuffer(0);
    free(p);
    free(q);
}
void clear_screen(int w, int h, void* _loc)
{
    int i,j;
    int stride = fi.line_length / (vi.bits_per_pixel >> 3);
    uint16_t *loc = _loc;
    uint32_t *loc32 = _loc;

    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (vi.bits_per_pixel == 16)
                loc[i + j*(stride)] = blue;
            else
                loc32[i + j*(stride)] = green32;

    //=================
//    for (j = 894; j < 894+150; j++)
//        for (i = 160; i < 160+150; i++)
//            if (vi.bits_per_pixel == 16)
//                loc[i + j*(stride)] = blue;
//            else
//                loc32[i + j*(stride)] = green32;
    //================
}

  int fb_main(void) {
  int w;
  int h;
  int b_fd;
  int id = 0;
  int bpp = 0;
 
  gr_init(bpp, id);
  b_fd=open("/sys/class/leds/lcd-backlight/brightness",O_RDWR);
  write(b_fd,"255",4);
  w = vi.xres;
  h = vi.yres;
  clear_screen(vi.xres, vi.yres, gr_fbs[0].data);
  clear_screen(vi.xres, vi.yres, gr_fbs[1].data);

  //draw_grid(w, h, gr_fbs[id].data);

 // set_active_framebuffer(!id);
  //set_active_framebuffer(id);
  return 0;
}
