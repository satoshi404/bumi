#ifndef X11_H
#define X11_H

#define X11_XLIB_HEADERS 1

#if X11_XLIB_HEADERS 
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xcursor/Xcursor.h>
#else

typedef Display X11_Display
typedef Screen  X11_Screen;

#undef Display

typedef unsigned long XID;
typedef unsigned long XColormap;
typedef XID XPointer;
typedef XID X11_Window;

struct X11_Screen;

struct X11_Display 
{
    void *ext_data;
	void *private1;
	int fd;
	int private2;
	int proto_major_version;
	int proto_minor_version;
	char *vendor;
	XID private3;
	XID private4;
	XID private5;
	int private6;
	XID (*resource_alloc)
	(struct X11_Display *);
	int byte_order;
	int bitmap_unit;
	int bitmap_pad;
	int bitmap_bit_order;
	int nformats;
	void *pixmap_format;
	int private8;
	int release;
	void *private9;
	void *private10;
	int qlen;
	unsigned long last_request_read;
	unsigned long request;
	XPointer private11;
	XPointer private12;
	XPointer private13;
	XPointer private14;
	unsigned max_request_size;
	void *db;
	int (*private15)(struct X11_Display *);
	char *display_name;
	int default_screen;
	int nscreens;
	X11_Screen *screens;
	unsigned long motion_buffer;
	unsigned long private16;
	int min_keycode;
	int max_keycode;
	XPointer private17;
	XPointer private18;
	int private19;
	char *xdefaults;
};


struct X11_Screen 
{
    void *ext_data;
	X11_Display *display;
	X11_Window root;
	int width;
	int height;
	int mwidth;
	int mheight;
	int ndepths;
	void *depths;
	int root_depth;
	void *root_visual;
	void *default_gc;
	XColormap cmap;
	unsigned long white_pixel;
	unsigned long black_pixel;
	int max_maps;
	int min_maps;
	int backing_store;
	int save_unders;
	long root_input_mask;
};


#ifdef __cplusplus 
extern "C" {
#endif  
    X11_Display* XOpenDisplay(
        const char*          // display
    );

    void 
}
#endif

#endif