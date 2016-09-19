/*
** mrb_remocon.c - Remocon class
**
** Copyright (c) Hiroki Mori 2016
**
** See Copyright Notice in LICENSE
*/

#include "mruby.h"
#include "mruby/data.h"
#include "mrb_remocon.h"
#ifdef __FreeBSD__
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

libusb_device_handle* open_device(libusb_context *ctx);
void close_device(libusb_context *ctx, libusb_device_handle *devh);
void write_device(struct libusb_device_handle *devh, unsigned char *cmd, int len
);


#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  libusb_context *ctx;
  libusb_device_handle *devh;
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
  data->ctx = NULL;
  libusb_init(&data->ctx);
  data->devh = open_device(data->ctx);

  DATA_PTR(self) = data;

  return self;
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

  write_device(data->devh, buf, 64);

  return mrb_fixnum_value(0);
}

void mrb_mruby_remocon_gem_init(mrb_state *mrb)
{
  struct RClass *remocon;
  remocon = mrb_define_class(mrb, "Remocon", mrb->object_class);
  mrb_define_method(mrb, remocon, "initialize", mrb_remocon_init, MRB_ARGS_NONE());
  mrb_define_method(mrb, remocon, "send", mrb_remocon_send, MRB_ARGS_REQ(1));
  DONE;
}

void mrb_mruby_remocon_gem_final(mrb_state *mrb)
{
}

