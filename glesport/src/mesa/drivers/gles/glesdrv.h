/*
 * Mesa 3-D graphics library
 * Version:  4.0
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Author: Sami Kyöstilä <sami.kyostila@gmail.com>
 */

/* glesdd.c - GLES Mesa rendering mode setup functions */

#ifndef GLESDRV_H
#define GLESDRV_H

/* If you comment out this define, a variable takes its place, letting
 * you turn debugging on/off from the debugger.
 */

#include "glheader.h"


#if defined(__linux__)
#include <signal.h>
#endif

#include "context.h"
#include "imports.h"
#include "macros.h"
#include "matrix.h"
#include "mtypes.h"

#include "math/m_vector.h"

#include <GLES/gl.h>
#include <GLES/egl.h>

/* For the GLX functions */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define GLES_CONTEXT(ctx)     ((glesMesaContext)((ctx)->DriverCtx))
#define GET_GLES_CONTEXT(ctx) glesMesaContext glesCtx = GLES_CONTEXT(ctx)

/* FIXME: Make this configurable */
#define GLES_LIBRARY_NAME "libGLES_CM.so"
#define EGL_LIBRARY_NAME  "libGLES_CM.so"

typedef struct tglesMesaContext* glesMesaContext;
typedef struct tglesGLXContext*  glesGLXContext;

struct tglesMesaContext
{
   GLcontext *glCtx;		/* the core Mesa context */
   GLvisual *glVis;		/* describes the color buffer */
   GLframebuffer *glBuffer;	/* the ancillary buffers */

   glesGLXContext glxContext;

   /* OpenGL ES library */
   void* glesLibrary;

   /* OpenGL ES functions */
   void (*glActiveTexture) (GLenum texture);
   void (*glAlphaFunc) (GLenum func, GLclampf ref);
   void (*glBindBuffer)(GLenum target, GLuint buffer);
   void (*glBindTexture) (GLenum target, GLuint texture);
   void (*glBlendFunc) (GLenum sfactor, GLenum dfactor);
   void (*glBufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
   void (*glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
   void (*glClear) (GLbitfield mask);
   void (*glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void (*glClearDepthf) (GLclampf depth);
   void (*glClearStencil) (GLint s);
   void (*glClientActiveTexture) (GLenum texture);
   void (*glClipPlanef)(GLenum plane, const GLfloat *equation);
   void (*glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   void (*glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   void (*glColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
   void (*glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
   void (*glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
   void (*glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   void (*glCullFace) (GLenum mode);
   void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers);
   void (*glDeleteTextures) (GLsizei n, const GLuint *textures);
   void (*glDepthFunc) (GLenum func);
   void (*glDepthMask) (GLboolean flag);
   void (*glDepthRangef) (GLclampf zNear, GLclampf zFar);
   void (*glDisable) (GLenum cap);
   void (*glDisableClientState) (GLenum array);
   void (*glDrawArrays) (GLenum mode, GLint first, GLsizei count);
   void (*glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
   void (*glEnable) (GLenum cap);
   void (*glEnableClientState) (GLenum array);
   void (*glFinish) (void);
   void (*glFlush) (void);
   void (*glFogf) (GLenum pname, GLfloat param);
   void (*glFogfv) (GLenum pname, const GLfloat *params);
   void (*glFogxv) (GLenum pname, const GLfixed *params);
   void (*glFrontFace) (GLenum mode);
   void (*glFrustumf) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void (*glGetBooleanv)(GLenum pname, GLboolean *params);
   void (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
   void (*glGetClipPlanef)(GLenum pname, GLfloat eqn[4]);
   void (*glGenBuffers)(GLsizei n, GLuint *buffers);
   void (*glGenTextures) (GLsizei n, GLuint *textures);
   GLenum (*glGetError) (void);
   void (*glGetFixedv)(GLenum pname, GLfixed *params);
   void (*glGetFloatv)(GLenum pname, GLfloat *params);
   void (*glGetIntegerv) (GLenum pname, GLint *params);
   void (*glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
   void (*glGetLightxv)(GLenum light, GLenum pname, GLfixed *params);
   void (*glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
   void (*glGetMaterialxv)(GLenum face, GLenum pname, GLfixed *params);
   void (*glGetPointerv)(GLenum pname, void **params);
   const GLubyte * (*glGetString) (GLenum name);
   void (*glGetTexEnviv)(GLenum env, GLenum pname, GLint *params);
   void (*glGetTexEnvfv)(GLenum env, GLenum pname, GLfloat *params);
   void (*glGetTexEnvxv)(GLenum env, GLenum pname, GLfixed *params);
   void (*glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
   void (*glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
   void (*glGetTexParameterxv)(GLenum target, GLenum pname, GLfixed *params);
   void (*glHint) (GLenum target, GLenum mode);
   GLboolean (*glIsBuffer)(GLuint buffer);
   GLboolean (*glIsEnabled)(GLenum cap);
   GLboolean (*glIsTexture)(GLuint texture);
   void (*glLightModelf) (GLenum pname, GLfloat param);
   void (*glLightModelfv) (GLenum pname, const GLfloat *params);
   void (*glLightf) (GLenum light, GLenum pname, GLfloat param);
   void (*glLightfv) (GLenum light, GLenum pname, const GLfloat *params);
   void (*glLineWidth) (GLfloat width);
   void (*glLoadIdentity) (void);
   void (*glLoadMatrixf) (const GLfloat *m);
   void (*glLogicOp) (GLenum opcode);
   void (*glMaterialf) (GLenum face, GLenum pname, GLfloat param);
   void (*glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params);
   void (*glMaterialxv) (GLenum face, GLenum pname, const GLfixed *params);
   void (*glMatrixMode) (GLenum mode);
   void (*glMultMatrixf) (const GLfloat *m);
   void (*glMultiTexCoord4f) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
   void (*glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
   void (*glNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glOrthof) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void (*glPixelStorei) (GLenum pname, GLint param);
   void (*glPointParameterf)(GLenum pname, GLfloat param);
   void (*glPointParameterfv)(GLenum pname, const GLfloat *params);
   void (*glPointSize) (GLfloat size);
   void (*glPolygonOffset) (GLfloat factor, GLfloat units);
   void (*glPopMatrix) (void);
   void (*glPushMatrix) (void);
   void (*glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
   void (*glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
   void (*glSampleCoverage) (GLclampf value, GLboolean invert);
   void (*glScalef) (GLfloat x, GLfloat y, GLfloat z);
   void (*glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
   void (*glShadeModel) (GLenum mode);
   void (*glStencilFunc) (GLenum func, GLint ref, GLuint mask);
   void (*glStencilMask) (GLuint mask);
   void (*glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
   void (*glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glTexEnvf) (GLenum target, GLenum pname, GLfloat param);
   void (*glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
   void (*glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
   void (*glTexParameteri) (GLenum target, GLenum pname, GLint param);
   void (*glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
   void (*glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
   void (*glTranslatef) (GLfloat x, GLfloat y, GLfloat z);
   void (*glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);
   void (*glCurrentPaletteMatrixOES) (GLuint matrixpaletteindex);
   void (*glLoadPaletteFromModelViewMatrixOES) (void);
   void (*glMatrixIndexPointerOES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glWeightPointerOES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glPointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid *pointer);
   void (*glDrawTexsOES) (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
   void (*glDrawTexiOES) (GLint x, GLint y, GLint z, GLint width, GLint height);
   void (*glDrawTexfOES) (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
   void (*glDrawTexxOES) (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
   void (*glDrawTexsvOES) (GLshort *coords);
   void (*glDrawTexivOES) (GLint *coords);
   void (*glDrawTexfvOES) (GLfloat *coords);
   void (*glDrawTexxvOES) (GLfixed *coords);

   /* GLES engine features */
   GLint maxTextureUnits;

   /* Temporary buffers for format conversion */
   GLshort* indexBuffer;
   GLint indexBufferSize;
   GLbyte* colorBuffer;
   GLint colorBufferSize;
};

extern int glesDDInitGLESMesaContext(glesMesaContext glesMesa);
extern void glesDDDestroyGLESMesaContext(glesMesaContext glesMesa);

extern void glesSetupDDPointers(GLcontext *);
extern void glesDDInitExtensions(GLcontext * ctx);

struct tglesGLXContext
{
   glesMesaContext glesContext;
   EGLDisplay      display;
   EGLContext      context;
   EGLConfig       config;
   EGLSurface      surface;
   
   /* EGL library */
   void* eglLibrary;

   /* This library */
   void* thisLibrary;

   /* EGL functions */
   EGLint (*eglGetError) (void);
   EGLDisplay (*eglGetDisplay) (NativeDisplayType display);
   EGLBoolean (*eglInitialize) (EGLDisplay dpy, EGLint *major, EGLint *minor);
   EGLBoolean (*eglTerminate) (EGLDisplay dpy);
   const char * (*eglQueryString) (EGLDisplay dpy, EGLint name);
   void (* (*eglGetProcAddress) (const char *procname))();
   EGLBoolean (*eglGetConfigs) (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
   EGLBoolean (*eglChooseConfig) (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
   EGLBoolean (*eglGetConfigAttrib) (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
   EGLSurface (*eglCreateWindowSurface) (EGLDisplay dpy, EGLConfig config, NativeWindowType window, const EGLint *attrib_list);
   EGLSurface (*eglCreatePixmapSurface) (EGLDisplay dpy, EGLConfig config, NativePixmapType pixmap, const EGLint *attrib_list);
   EGLSurface (*eglCreatePbufferSurface) (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
   EGLBoolean (*eglDestroySurface) (EGLDisplay dpy, EGLSurface surface);
   EGLBoolean (*eglQuerySurface) (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
   EGLBoolean (*eglSurfaceAttrib) (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
   EGLBoolean (*eglBindTexImage) (EGLDisplay dpy, EGLSurface surface, EGLint buffer);
   EGLBoolean (*eglReleaseTexImage) (EGLDisplay dpy, EGLSurface surface, EGLint buffer);
   EGLBoolean (*eglSwapInterval) (EGLDisplay dpy, EGLint interval);
   EGLContext (*eglCreateContext) (EGLDisplay dpy, EGLConfig config, EGLContext share_list, const EGLint *attrib_list);
   EGLBoolean (*eglDestroyContext) (EGLDisplay dpy, EGLContext ctx);
   EGLBoolean (*eglMakeCurrent) (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
   EGLContext (*eglGetCurrentContext) (void);
   EGLSurface (*eglGetCurrentSurface) (EGLint readdraw);
   EGLDisplay (*eglGetCurrentDisplay) (void);
   EGLBoolean (*eglQueryContext) (EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
   EGLBoolean (*eglWaitGL) (void);
   EGLBoolean (*eglWaitNative) (EGLint engine);
   EGLBoolean (*eglSwapBuffers) (EGLDisplay dpy, EGLSurface draw);
   EGLBoolean (*eglCopyBuffers) (EGLDisplay dpy, EGLSurface surface, NativePixmapType target);
};

/*
 * GLXFBConfig is a pointer to opaque data.
 */
typedef struct tglesGLXContext *GLXFBConfig;

/*
 * GLX handles
 */
typedef XID GLXDrawable;
typedef XID GLXPixmap;

/*
 * GLX functions
 */

/*
 * Names for attributes to glXGetConfig.
 */
#define GLX_USE_GL              1       /* support GLX rendering */
#define GLX_BUFFER_SIZE         2       /* depth of the color buffer */
#define GLX_LEVEL               3       /* level in plane stacking */
#define GLX_RGBA                4       /* true if RGBA mode */
#define GLX_DOUBLEBUFFER        5       /* double buffering supported */
#define GLX_STEREO              6       /* stereo buffering supported */
#define GLX_AUX_BUFFERS         7       /* number of aux buffers */
#define GLX_RED_SIZE            8       /* number of red component bits */
#define GLX_GREEN_SIZE          9       /* number of green component bits */
#define GLX_BLUE_SIZE           10      /* number of blue component bits */
#define GLX_ALPHA_SIZE          11      /* number of alpha component bits */
#define GLX_DEPTH_SIZE          12      /* number of depth bits */
#define GLX_STENCIL_SIZE        13      /* number of stencil bits */
#define GLX_ACCUM_RED_SIZE      14      /* number of red accum bits */
#define GLX_ACCUM_GREEN_SIZE    15      /* number of green accum bits */
#define GLX_ACCUM_BLUE_SIZE     16      /* number of blue accum bits */
#define GLX_ACCUM_ALPHA_SIZE    17      /* number of alpha accum bits */
#define GLX_MAX_PBUFFER_WIDTH		0x8016
#define GLX_MAX_PBUFFER_HEIGHT		0x8017
#define GLX_SAMPLE_BUFFERS              0x186a0 /*100000*/
#define GLX_SAMPLES                     0x186a1 /*100001*/

/*
 * Error return values from glXGetConfig.  Success is indicated by
 * a value of 0.
 */
#define GLX_BAD_SCREEN                  1  /* screen # is bad */
#define GLX_BAD_ATTRIBUTE               2  /* attribute to get is bad */
#define GLX_NO_EXTENSION                3  /* no glx extension on server */
#define GLX_BAD_VISUAL                  4  /* visual # not known by GLX */
#define GLX_BAD_CONTEXT                 5
#define GLX_BAD_VALUE                   6
#define GLX_BAD_ENUM                    7

extern XVisualInfo* glXChooseVisual(Display *dpy, int screen,
                                    int *attrib_list);

extern glesGLXContext glXCreateContext(Display *dpy, XVisualInfo *vis,
                                       glesGLXContext share_list, Bool direct);

extern void glXDestroyContext(Display *dpy, glesGLXContext ctx);

extern void glXDestroyGLXPixmap(Display *dpy, GLXPixmap pix);

extern int glXGetConfig(Display *dpy, XVisualInfo *vis,
                        int attrib, int *value);

extern Bool glXMakeCurrent(Display *dpy, GLXDrawable drawable,
                           glesGLXContext ctx);

extern void glXSwapBuffers(Display *dpy, GLXDrawable drawable);

extern void glXWaitGL(void);

extern void glXWaitX(void);

extern Bool glXQueryExtension(Display *dpy, int *error_base, int *event_base);

extern const char *glXQueryExtensionsString( Display *dpy, int screen );

extern void glXCopyContext(Display *dpy, glesGLXContext src,
                           glesGLXContext dst, unsigned long mask);

extern GLXPixmap glXCreateGLXPixmap(Display *dpy, XVisualInfo *vis,
                                    Pixmap pixmap);

extern glesGLXContext glXGetCurrentContext(void);

extern GLXDrawable glXGetCurrentDrawable(void);

extern Bool glXIsDirect(Display *dpy, glesGLXContext ctx);

extern Bool glXQueryVersion(Display *dpy, int *major, int *minor);

extern void* glXGetProcAddressARB (const GLubyte *);

/*
 * Run-time debugging
 */
#define GLES_DEBUG 1
#ifndef GLES_DEBUG
#define GLES_DEBUG 0
#endif

#endif /* GLESDRV_H */
