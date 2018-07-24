/*
** mrb_remocon.c - Remocon class
**
** Copyright (c) Hiroki Mori 2016
**
** See Copyright Notice in LICENSE
*/

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/array.h"
#include "mrb_remocon.h"
#if defined( __APPLE__ )
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#elif defined( __FreeBSD__ )
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#if defined( __APPLE__ )
#else
libusb_device_handle* open_device(libusb_context *ctx);
void close_device(libusb_context *ctx, libusb_device_handle *devh);
void write_device(struct libusb_device_handle *devh, unsigned char *cmd, int len);
int receive_ir(struct libusb_device_handle *devh, unsigned char *data, int length, int extend);
int version(struct libusb_device_handle *devh, unsigned char *data, int length, int extend);
#endif


#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
#if defined( __APPLE__ )
  hid_device* handle;
#else
  libusb_context *ctx;
  libusb_device_handle *devh;
#endif
} mrb_remocon_data;

static const struct mrb_data_type mrb_remocon_data_type = {
  "mrb_remocon_data", mrb_free,
};

static mrb_value mrb_remocon_init(mrb_state *mrb, mrb_value self)
{
  mrb_remocon_data *data;

  data = (mrb_remocon_data *)DATA_PTR(self);
  if (data) {
    mrb_free(mrb, data);
  }
  DATA_TYPE(self) = &mrb_remocon_data_type;
  DATA_PTR(self) = NULL;

  data = (mrb_remocon_data *)mrb_malloc(mrb, sizeof(mrb_remocon_data));

  DATA_PTR(self) = data;

  return self;
}

static mrb_value mrb_remocon_open(mrb_state *mrb, mrb_value self)
{
  mrb_remocon_data *data = DATA_PTR(self);
#if defined( __APPLE__ )
  char *devPath;
  devPath = NULL;
  hid_init();
  struct hid_device_info* allDevices = hid_enumerate(0x22ea, 0x001e);
  struct hid_device_info* currentDevice;
  currentDevice = allDevices;

  while (currentDevice)
  {
    int len = strlen(currentDevice->path);
    if (currentDevice->path[len - 1] == '3') {
      devPath = malloc(strlen(currentDevice->path) + 1);
      strcpy(devPath, currentDevice->path);
    }
    currentDevice = currentDevice->next;
  }
  if (devPath == NULL) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Can't open USB module.");
    return mrb_nil_value();
  }
  hid_free_enumeration(allDevices);
  data->handle = hid_open_path(devPath);
  free(devPath);
#else
  data->ctx = NULL;
  libusb_init(&data->ctx);
  data->devh = open_device(data->ctx);
  if (data->devh == -1) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Can't open USB module.");
    return mrb_nil_value();
  }
#endif
  return mrb_fixnum_value(1);
}

static mrb_value mrb_remocon_send(mrb_state *mrb, mrb_value self)
{
  int i;
  int b;
  mrb_int size;
  mrb_value *argv;
  unsigned char buf[64];

  buf[0] = 0x60;
  mrb_get_args(mrb, "a", &argv, &size);
  for(i = 0; i < size; ++i, ++argv){
    b = mrb_fixnum(*argv);
    if( i == 0 ) {
      buf[i+1] = b;
    } else {
      // reverse bit
      buf[i+1] = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
    }
  }

  mrb_remocon_data *data = DATA_PTR(self);

#if defined( __APPLE__ )
  hid_write(data->handle, buf, 65);
#else
  write_device(data->devh, buf, 64);
#endif

  return mrb_fixnum_value(0);
}

static mrb_value mrb_remocon_recieve(mrb_state *mrb, mrb_value self)
{
	int i;
	int b;
	int len;
	unsigned char tmp[64];
	unsigned char buf[64];
	int off;
	
	mrb_remocon_data *data = DATA_PTR(self);
	
#if defined( __APPLE__ )
	tmp[0] = 0x51;
	tmp[1] = 0x01;
	hid_write(data->handle, tmp, 65);
	hid_read(data->handle, tmp, 65);
        while(1) {
	tmp[0] = 0x50;
	hid_write(data->handle, tmp, 65);
	hid_read(data->handle, buf, 65);
        if(buf[1] != 0) {
          break;
        }
        }
	tmp[0] = 0x51;
	tmp[1] = 0x00;
	hid_write(data->handle, tmp, 65);
	hid_read(data->handle, tmp, 65);
	off = 1;
#else
	receive_ir(data->devh, buf, 64, 0);
	off = 1;
#endif
	len = ((buf[off] >> 4) + 1) / 2 + 1;
	mrb_value array;
	array = mrb_ary_new(mrb);
	for(i = 0; i < len; ++i) {
		if(i != 0) {
			b = buf[i+off];
			buf[i+off] = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16; 
		}
		mrb_ary_push(mrb, array, mrb_fixnum_value(buf[i+off]));
	}
	
	return array;
}

static mrb_value mrb_remocon_version(mrb_state *mrb, mrb_value self)
{
	unsigned char tmp[64];

	mrb_remocon_data *data = DATA_PTR(self);
#if defined( __APPLE__ )
	tmp[0] = 0x56;
	hid_write(data->handle, tmp, 65);
	hid_read(data->handle, tmp, 65);
#else
	version(data->devh, buf, 64, 0);
#endif
	return mrb_str_new_cstr(mrb, (const char*)tmp + 1);
}

void mrb_mruby_remocon_gem_init(mrb_state *mrb)
{
  struct RClass *remocon;
  remocon = mrb_define_class(mrb, "Remocon", mrb->object_class);
  mrb_define_method(mrb, remocon, "initialize", mrb_remocon_init, MRB_ARGS_NONE());
  mrb_define_method(mrb, remocon, "open", mrb_remocon_open, MRB_ARGS_NONE());
  mrb_define_method(mrb, remocon, "send", mrb_remocon_send, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, remocon, "recieve", mrb_remocon_recieve, MRB_ARGS_NONE());
  mrb_define_method(mrb, remocon, "version", mrb_remocon_version, MRB_ARGS_NONE());
  DONE;
}

void mrb_mruby_remocon_gem_final(mrb_state *mrb)
{
}

