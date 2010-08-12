/*
 * Mesa 3-D graphics library
 * Version:  5.1
 *
 * Copyright (C) 1999-2003  Brian Paul   All Rights Reserved.
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

/* glesdd.c - GLES Mesa device driver functions */


#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

/* FIXME */
#define GLES
#if defined(GLES)

#if defined(__linux__)
#include <dlfcn.h>
#else
#error please port me
#endif

#include "image.h"
#include "mtypes.h"
#include "buffers.h"
#include "enums.h"
#include "extensions.h"
#include "macros.h"
#include "texstore.h"
#include "teximage.h"
#include "swrast/swrast.h"
#include "swrast/s_context.h"
#include "swrast_setup/swrast_setup.h"
#include "tnl/tnl.h"
#include "tnl/t_context.h"
#include "tnl/t_pipeline.h"
#include "vbo/vbo.h"
#include "vbo/vbo_context.h"
#include "glesdrv.h"

/* Signify an unsupported (no-op) function */
#define GLES_UNSUPPORTED() \
   do { \
      if (GLES_DEBUG) { \
         fprintf(stderr, "Unsupported GLES operation: %s\n", __func__); \
      } \
   } while (0)

/* Check a condition and return if it's false */

#define GLES_FAIL_UNLESS(COND) \
   do { \
      if (!(COND)) { \
         if (GLES_DEBUG) { \
            fprintf(stderr, "GLES operation %s failed: %s\n", __func__, #COND); \
         } \
      } \
  } while (0)

/* Check that the GLES error status is clear */
#define GLES_CHECK_ERROR(CTX) \
   do { \
      glesMesaContext _glesCtx = GLES_CONTEXT(CTX); \
      GLint _err = _glesCtx->glGetError(); \
      if (_err != GL_NO_ERROR) { \
         if (GLES_DEBUG) { \
            fprintf(stderr, "GLES operation %s failed at line %d due to error 0x%x\n", __func__, __LINE__, _err); \
         } \
      } \
  } while (0)


static void updatePackFormat(glesMesaContext glesCtx, const struct gl_pixelstore_attrib *pack)
{
   /* TODO: We really should check the pack attributes more carefully here */
   glesCtx->glPixelStorei(GL_PACK_ALIGNMENT, pack->Alignment);
}

static void updateUnpackFormat(glesMesaContext glesCtx, const struct gl_pixelstore_attrib *unpack)
{
   /* TODO: We really should check the unpack attributes more carefully here */
   glesCtx->glPixelStorei(GL_UNPACK_ALIGNMENT, unpack->Alignment);
}

static void glesDDClear( GLcontext *ctx, GLbitfield mask )
{
   GET_GLES_CONTEXT(ctx);
   GLbitfield glMask = 0;

   if (mask & ctx->DrawBuffer->_ColorDrawBufferMask[0])
   {
      glMask |= GL_COLOR_BUFFER_BIT;
   }

   if (mask & BUFFER_BIT_DEPTH)
   {
      glMask |= GL_DEPTH_BUFFER_BIT;
   }
   
   if (mask & BUFFER_BIT_STENCIL)
   {
      glMask |= GL_STENCIL_BUFFER_BIT;
   }

   glesCtx->glClear(glMask);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDFinish( GLcontext *ctx )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glFinish();
   GLES_CHECK_ERROR(ctx);
}

static void glesDDFlush( GLcontext *ctx )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glFlush();
   GLES_CHECK_ERROR(ctx);
}

static void glesDDAccum( GLcontext *ctx, GLenum op, GLfloat value )
{
   GLES_UNSUPPORTED();
}

static void glesDDDrawPixels( GLcontext *ctx,
                              GLint x, GLint y, GLsizei width, GLsizei height,
                              GLenum format, GLenum type,
                              const struct gl_pixelstore_attrib *unpack,
                              const GLvoid *pixels )
{
   GLES_UNSUPPORTED();
   /* TODO */
}

static void glesDDReadPixels( GLcontext *ctx,
                              GLint x, GLint y, GLsizei width, GLsizei height,
                              GLenum format, GLenum type,
                              const struct gl_pixelstore_attrib *unpack,
                              GLvoid *dest )
{
   GET_GLES_CONTEXT(ctx);
   updateUnpackFormat(glesCtx, unpack);
   glesCtx->glReadPixels(x, y, width, height, format, type, dest);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDCopyPixels( GLcontext *ctx, GLint srcx, GLint srcy,
                              GLsizei width, GLsizei height,
                              GLint dstx, GLint dsty, GLenum type )
{
   GLES_UNSUPPORTED();
   /* TODO */
}

static void glesDDBitmap( GLcontext *ctx,
                          GLint x, GLint y, GLsizei width, GLsizei height,
                          const struct gl_pixelstore_attrib *unpack,
                          const GLubyte *bitmap )
{
   GLES_UNSUPPORTED();
   /* TODO */
}

static void glesDDTexImage1D( GLcontext *ctx, GLenum target, GLint level,
                              GLint internalFormat,
                              GLint width, GLint border,
                              GLenum format, GLenum type, const GLvoid *pixels,
                              const struct gl_pixelstore_attrib *packing,
                              struct gl_texture_object *texObj,
                              struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* No one dimensional textures */
}

static void glesDDTexImage2D( GLcontext *ctx, GLenum target, GLint level,
                              GLint internalFormat,
                              GLint width, GLint height, GLint border,
                              GLenum format, GLenum type, const GLvoid *pixels,
                              const struct gl_pixelstore_attrib *packing,
                              struct gl_texture_object *texObj,
                              struct gl_texture_image *texImage )
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(border == 0);
   GLES_FAIL_UNLESS(
      internalFormat == GL_ALPHA ||
      internalFormat == GL_RGB ||
      internalFormat == GL_RGBA ||
      internalFormat == GL_LUMINANCE ||
      internalFormat == GL_LUMINANCE_ALPHA
   );
   GLES_FAIL_UNLESS(format == internalFormat);
   /* TODO: type conversion */
   GLES_FAIL_UNLESS(
      type == GL_UNSIGNED_BYTE ||
      type == GL_UNSIGNED_SHORT_5_6_5 ||
      type == GL_UNSIGNED_SHORT_4_4_4_4 ||
      type == GL_UNSIGNED_SHORT_5_5_5_1
   );

   updatePackFormat(glesCtx, packing);
   glesCtx->glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTexImage3D( GLcontext *ctx, GLenum target, GLint level,
                              GLint internalFormat,
                              GLint width, GLint height, GLint depth, GLint border,
                              GLenum format, GLenum type, const GLvoid *pixels,
                              const struct gl_pixelstore_attrib *packing,
                              struct gl_texture_object *texObj,
                              struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* No three dimensional textures */
}

static void glesDDTexSubImage1D( GLcontext *ctx, GLenum target, GLint level,
                                 GLint xoffset, GLsizei width,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels,
                                 const struct gl_pixelstore_attrib *packing,
                                 struct gl_texture_object *texObj,
                                 struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* No one dimensional textures */
}

static void glesDDTexSubImage2D( GLcontext *ctx, GLenum target, GLint level,
                                 GLint xoffset, GLint yoffset,
                                 GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels,
                                 const struct gl_pixelstore_attrib *packing,
                                 struct gl_texture_object *texObj,
                                 struct gl_texture_image *texImage )
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(
      format == GL_ALPHA ||
      format == GL_RGB ||
      format == GL_RGBA ||
      format == GL_LUMINANCE ||
      format == GL_LUMINANCE_ALPHA
   );
   /* TODO: type conversion */
   GLES_FAIL_UNLESS(
      type == GL_UNSIGNED_BYTE ||
      type == GL_UNSIGNED_SHORT_5_6_5 ||
      type == GL_UNSIGNED_SHORT_4_4_4_4 ||
      type == GL_UNSIGNED_SHORT_5_5_5_1
   );

   updatePackFormat(glesCtx, packing);
   glesCtx->glTexImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTexSubImage3D( GLcontext *ctx, GLenum target, GLint level,
                                 GLint xoffset, GLint yoffset, GLint zoffset,
                                 GLsizei width, GLsizei height, GLint depth,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels,
                                 const struct gl_pixelstore_attrib *packing,
                                 struct gl_texture_object *texObj,
                                 struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* No three dimensional textures */
}

static void glesDDGetTexImage( GLcontext *ctx, GLenum target, GLint level,
                               GLenum format, GLenum type, GLvoid *pixels,
                               struct gl_texture_object *texObj,
                               struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* TODO */
}

static void glesDDCopyTexImage1D( GLcontext *ctx, GLenum target, GLint level,
                                  GLenum internalFormat, GLint x, GLint y,
                                  GLsizei width, GLint border )
{
   GLES_UNSUPPORTED();
   /* No one dimensional textures */
}

static void glesDDCopyTexImage2D( GLcontext *ctx, GLenum target, GLint level,
                                  GLenum internalFormat, GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border )
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(border == 0);
   GLES_FAIL_UNLESS(
      internalFormat == GL_ALPHA ||
      internalFormat == GL_RGB ||
      internalFormat == GL_RGBA ||
      internalFormat == GL_LUMINANCE ||
      internalFormat == GL_LUMINANCE_ALPHA
   );
   glesCtx->glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDCopyTexSubImage1D( GLcontext *ctx, GLenum target, GLint level,
                                     GLint xoffset,
                                     GLint x, GLint y, GLsizei width )
{
   GLES_UNSUPPORTED();
   /* No one dimensional textures */
}

static void glesDDCopyTexSubImage2D( GLcontext *ctx, GLenum target, GLint level,
                                     GLint xoffset, GLint yoffset,
                                     GLint x, GLint y,
                                     GLsizei width, GLsizei height )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDCopyTexSubImage3D( GLcontext *ctx, GLenum target, GLint level,
                                     GLint xoffset, GLint yoffset, GLint zoffset,
                                     GLint x, GLint y,
                                     GLsizei width, GLsizei height )
{
   GLES_UNSUPPORTED();
   /* No three dimensional textures */
}

static GLboolean glesDDTestProxyTexImage(GLcontext *ctx, GLenum target,
                                         GLint level, GLint internalFormat,
                                         GLenum format, GLenum type,
                                         GLint width, GLint height,
                                         GLint depth, GLint border)
{
   /* Accept all textures by default */
   return GL_TRUE;
}

static void glesDDCompressedTexImage1D( GLcontext *ctx, GLenum target,
                                        GLint level, GLint internalFormat,
                                        GLsizei width, GLint border,
                                        GLsizei imageSize, const GLvoid *data,
                                        struct gl_texture_object *texObj,
                                        struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static void glesDDCompressedTexImage2D( GLcontext *ctx, GLenum target,
                                        GLint level, GLint internalFormat,
                                        GLsizei width, GLsizei height, GLint border,
                                        GLsizei imageSize, const GLvoid *data,
                                        struct gl_texture_object *texObj,
                                        struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static void glesDDCompressedTexImage3D( GLcontext *ctx, GLenum target,
                                        GLint level, GLint internalFormat,
                                        GLsizei width, GLsizei height, GLsizei depth,
                                        GLint border,
                                        GLsizei imageSize, const GLvoid *data,
                                        struct gl_texture_object *texObj,
                                        struct gl_texture_image *texImage )
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static void glesDDCompressedTexSubImage1D(GLcontext *ctx, GLenum target, GLint level,
                                          GLint xoffset, GLsizei width,
                                          GLenum format,
                                          GLsizei imageSize, const GLvoid *data,
                                          struct gl_texture_object *texObj,
                                          struct gl_texture_image *texImage)
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static void glesDDCompressedTexSubImage2D(GLcontext *ctx, GLenum target, GLint level,
                                          GLint xoffset, GLint yoffset,
                                          GLsizei width, GLint height,
                                          GLenum format,
                                          GLsizei imageSize, const GLvoid *data,
                                          struct gl_texture_object *texObj,
                                          struct gl_texture_image *texImage)
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}


static void glesDDCompressedTexSubImage3D(GLcontext *ctx, GLenum target, GLint level,
                                          GLint xoffset, GLint yoffset, GLint zoffset,
                                          GLsizei width, GLint height, GLint depth,
                                          GLenum format,
                                          GLsizei imageSize, const GLvoid *data,
                                          struct gl_texture_object *texObj,
                                          struct gl_texture_image *texImage)
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static void glesDDGetCompressedTexImage(GLcontext *ctx, GLenum target, GLint level,
                                        GLvoid *img,
                                        const struct gl_texture_object *texObj,
                                        const struct gl_texture_image *texImage)
{
   GLES_UNSUPPORTED();
   /* TODO: Support some compressed texture formats  */
}

static GLuint glesDDCompressedTextureSize( GLcontext *ctx, GLsizei width,
                                           GLsizei height, GLsizei depth,
                                           GLenum format )
{
   /* TODO: Support some compressed texture formats  */
   return 0;
}

static void glesDDBindTexture( GLcontext *ctx, GLenum target,
                               struct gl_texture_object *tObj )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glBindTexture(target, tObj->Name);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDDeleteTexture( GLcontext *ctx, struct gl_texture_object *tObj )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glDeleteTextures(1, &tObj->Name);
   GLES_CHECK_ERROR(ctx);
}

static GLboolean glesDDIsTextureResident( GLcontext *ctx,
                                          struct gl_texture_object *t )
{
   /* All textures are resident by default as far as we know */
   return GL_TRUE;
}

static void glesDDPrioritizeTexture( GLcontext *ctx,  struct gl_texture_object *t,
                                     GLclampf priority )
{
   GLES_UNSUPPORTED();
}

static void glesDDActiveTexture( GLcontext *ctx, GLuint texUnitNumber )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glActiveTexture(GL_TEXTURE0 + texUnitNumber);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDUpdateTexturePalette( GLcontext *ctx,
                                        struct gl_texture_object *tObj )
{
   GLES_UNSUPPORTED();
}

static void glesDDCopyColorTable( GLcontext *ctx,
                                  GLenum target, GLenum internalformat,
                                  GLint x, GLint y, GLsizei width )
{
   GLES_UNSUPPORTED();
}

static void glesDDCopyColorSubTable( GLcontext *ctx,
                                     GLenum target, GLsizei start,
                                     GLint x, GLint y, GLsizei width )
{
   GLES_UNSUPPORTED();
}

static void glesDDCopyConvolutionFilter1D( GLcontext *ctx, GLenum target,
                                           GLenum internalFormat,
                                           GLint x, GLint y, GLsizei width )
{
   GLES_UNSUPPORTED();
}

static void glesDDCopyConvolutionFilter2D( GLcontext *ctx, GLenum target,
                                           GLenum internalFormat,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height )
{
   GLES_UNSUPPORTED();
}

static void glesDDBindProgram(GLcontext *ctx, GLenum target, struct gl_program *prog)
{
   GLES_UNSUPPORTED();
}

static void glesDDDeleteProgram(GLcontext *ctx, struct gl_program *prog)
{
   GLES_UNSUPPORTED();
}

static void glesDDProgramStringNotify(GLcontext *ctx, GLenum target,
                                      struct gl_program *prog)
{
   GLES_UNSUPPORTED();
}

static void glesDDGetProgramRegister(GLcontext *ctx, enum register_file file,
                                     GLuint index, GLfloat val[4])
{
   GLES_UNSUPPORTED();
}

static GLboolean glesDDIsProgramNative(GLcontext *ctx, GLenum target,
                                       struct gl_program *prog)
{
   GLES_UNSUPPORTED();
   return GL_FALSE;
}

static void glesDDAlphaFunc(GLcontext *ctx, GLenum func, GLfloat ref)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glAlphaFunc(func, ref);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDBlendColor(GLcontext *ctx, const GLfloat color[4])
{
   GLES_UNSUPPORTED();
}

static void glesDDBlendEquationSeparate(GLcontext *ctx, GLenum modeRGB, GLenum modeA)
{
   GLES_UNSUPPORTED();
}

static void glesDDBlendFuncSeparate(GLcontext *ctx,
                                    GLenum sfactorRGB, GLenum dfactorRGB,
                                    GLenum sfactorA, GLenum dfactorA)
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(sfactorRGB == sfactorA);
   GLES_FAIL_UNLESS(dfactorRGB == dfactorA);
   glesCtx->glBlendFunc(sfactorRGB, dfactorRGB);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDClearColor(GLcontext * ctx, const GLfloat color[4])
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glClearColor(color[0], color[1], color[2], color[3]);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDClearDepth(GLcontext *ctx, GLclampd d)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glClearDepthf(d);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDClearIndex(GLcontext *ctx, GLuint index)
{
   GLES_UNSUPPORTED();
}

static void glesDDClearStencil(GLcontext *ctx, GLint s)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glClearStencil(s);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDClipPlane(GLcontext *ctx, GLenum plane, const GLfloat *equation )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glClipPlanef(plane, equation);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDColorMask(GLcontext *ctx, GLboolean rmask, GLboolean gmask,
                            GLboolean bmask, GLboolean amask )
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glColorMask(rmask, gmask, bmask, amask);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDColorMaterial(GLcontext *ctx, GLenum face, GLenum mode)
{
   GLES_UNSUPPORTED();
}

static void glesDDCullFace(GLcontext *ctx, GLenum mode)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glCullFace(mode);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDFrontFace(GLcontext *ctx, GLenum mode)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glFrontFace(mode);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDDepthFunc(GLcontext *ctx, GLenum func)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glDepthFunc(func);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDDepthMask(GLcontext *ctx, GLboolean flag)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glDepthMask(flag);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDDepthRange(GLcontext *ctx, GLclampd nearval, GLclampd farval)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glDepthRangef(nearval, farval);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDDrawBuffer( GLcontext *ctx, GLenum buffer )
{
   GLES_UNSUPPORTED();
}

static void glesDDDrawBuffers( GLcontext *ctx, GLsizei n, const GLenum *buffers )
{
   GLES_UNSUPPORTED();
}

static void glesDDEnable(GLcontext *ctx, GLenum cap, GLboolean state)
{
   GET_GLES_CONTEXT(ctx);
   if (state /*&& cap != GL_LIGHTING*/) {
      glesCtx->glEnable(cap);
   } else {
      glesCtx->glDisable(cap);
   }
   GLES_CHECK_ERROR(ctx);
}

static void glesDDFogfv(GLcontext *ctx, GLenum pname, const GLfloat *params)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glFogfv(pname, params);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDHint(GLcontext *ctx, GLenum target, GLenum mode)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glHint(target, mode);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDIndexMask(GLcontext *ctx, GLuint mask)
{
   GLES_UNSUPPORTED();
}

static void glesDDLightfv(GLcontext *ctx, GLenum light,
                          GLenum pname, const GLfloat *params )
{
   /* TODO: Coordinates already in eye-space? */
   GET_GLES_CONTEXT(ctx);
   glesCtx->glLightfv(light, pname, params);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDLightModelfv(GLcontext *ctx, GLenum pname, const GLfloat *params)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glLightModelfv(pname, params);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDLineStipple(GLcontext *ctx, GLint factor, GLushort pattern )
{
   GLES_UNSUPPORTED();
}

static void glesDDLineWidth(GLcontext *ctx, GLfloat width)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glLineWidth(width);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDLogicOpcode(GLcontext *ctx, GLenum opcode)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glLogicOp(opcode);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDPointParameterfv(GLcontext *ctx, GLenum pname,
                                   const GLfloat *params)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glPointParameterfv(pname, params);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDPointSize(GLcontext *ctx, GLfloat size)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glPointSize(size);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDPolygonMode(GLcontext *ctx, GLenum face, GLenum mode)
{
   GLES_UNSUPPORTED();
}

static void glesDDPolygonOffset(GLcontext *ctx, GLfloat factor, GLfloat units)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glPolygonOffset(factor, units);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDPolygonStipple(GLcontext *ctx, const GLubyte *mask )
{
   GLES_UNSUPPORTED();
}

static void glesDDReadBuffer( GLcontext *ctx, GLenum buffer )
{
   GLES_UNSUPPORTED();
}

static void glesDDRenderMode(GLcontext *ctx, GLenum mode )
{
   GLES_UNSUPPORTED();
}

static void glesDDScissor(GLcontext *ctx, GLint x, GLint y, GLsizei w, GLsizei h)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glScissor(x, y, w, h);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDShadeModel(GLcontext *ctx, GLenum mode)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glShadeModel(mode);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDStencilFuncSeparate(GLcontext *ctx, GLenum face, GLenum func,
                                      GLint ref, GLuint mask)
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(face == GL_FRONT_AND_BACK);
   glesCtx->glStencilFunc(func, ref, mask);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDStencilMaskSeparate(GLcontext *ctx, GLenum face, GLuint mask)
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(face == GL_FRONT_AND_BACK);
   glesCtx->glStencilMask(mask);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDStencilOpSeparate(GLcontext *ctx, GLenum face, GLenum fail,
                                    GLenum zfail, GLenum zpass)
{
   GET_GLES_CONTEXT(ctx);
   GLES_FAIL_UNLESS(face == GL_FRONT_AND_BACK);
   glesCtx->glStencilOp(fail, zfail, zpass);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTexGen(GLcontext *ctx, GLenum coord, GLenum pname,
                         const GLfloat *params)
{
   GLES_UNSUPPORTED();
}

static void glesDDTexEnv(GLcontext *ctx, GLenum target, GLenum pname,
                         const GLfloat *param)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glTexEnvfv(target, pname, param);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTexParameter(GLcontext *ctx, GLenum target,
                               struct gl_texture_object *texObj,
                               GLenum pname, const GLfloat *params)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glTexParameterf(target, pname, *params);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTextureMatrix(GLcontext *ctx, GLuint unit, const GLmatrix *mat)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glActiveTexture(GL_TEXTURE0 + unit);
   glesCtx->glMatrixMode(GL_TEXTURE);
   glesCtx->glLoadMatrixf(mat->m);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDViewport(GLcontext *ctx, GLint x, GLint y, GLsizei w, GLsizei h)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glViewport(x, y, w, h);
   GLES_CHECK_ERROR(ctx);
}

static GLboolean glesDDGetBooleanv(GLcontext *ctx, GLenum pname, GLboolean *result)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glGetBooleanv(pname, result);
   GLES_CHECK_ERROR(ctx);
   return GL_TRUE;
}

static GLboolean glesDDGetDoublev(GLcontext *ctx, GLenum pname, GLdouble *result)
{
   GLES_UNSUPPORTED();
   return GL_FALSE;
}

static GLboolean glesDDGetFloatv(GLcontext *ctx, GLenum pname, GLfloat *result)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glGetFloatv(pname, result);
   GLES_CHECK_ERROR(ctx);
   return GL_TRUE;
}

static GLboolean glesDDGetIntegerv(GLcontext *ctx, GLenum pname, GLint *result)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glGetIntegerv(pname, result);
   GLES_CHECK_ERROR(ctx);
   return GL_TRUE;
}

static GLboolean glesDDGetPointerv(GLcontext *ctx, GLenum pname, GLvoid **result)
{
   GLES_UNSUPPORTED();
   return GL_FALSE;
}

static const void* arrayDataPointer(const struct gl_client_array* array)
{
   const void* base   = array->BufferObj ? array->BufferObj->Data : 0;
   const void* offset = array->Ptr;
   return (const void*)((unsigned int)base + (unsigned int)offset);
}

static void allocateIndexBuffer(glesMesaContext glesCtx, int newIndexCount)
{
   if (newIndexCount > glesCtx->indexBufferSize || !glesCtx->indexBuffer) {
      glesCtx->indexBufferSize = newIndexCount;
      glesCtx->indexBuffer     = (GLshort*)realloc(glesCtx->indexBuffer, newIndexCount * sizeof(glesCtx->indexBuffer[0]));
   }
}

static void allocateColorBuffer(glesMesaContext glesCtx, int newColorCount)
{
   if (newColorCount > glesCtx->colorBufferSize || !glesCtx->colorBuffer) {
      glesCtx->colorBufferSize = newColorCount;
      glesCtx->colorBuffer     = (GLbyte*)realloc(glesCtx->colorBuffer, newColorCount * sizeof(glesCtx->colorBuffer[0]));
   }
}

static void glesDDDrawPrimitives( GLcontext *ctx,
                                  const struct gl_client_array *arrays[],
		                  const struct _mesa_prim *prim,
		                  GLuint nr_prims,
		                  const struct _mesa_index_buffer *ib,
		                  GLuint min_index,
		                  GLuint max_index )
{
   GET_GLES_CONTEXT(ctx);
   int i;
/*
struct _mesa_prim {
   GLuint mode:8;
   GLuint indexed:1;
   GLuint begin:1;
   GLuint end:1;
   GLuint weak:1;
   GLuint pad:20;

   GLuint start;
   GLuint count;
};
*/

   /* Prepare arrays */
   if (GLES_DEBUG && 0) {
      for (i = 0; i < VERT_ATTRIB_MAX; i++) {
         int j;
         printf("--- Array %d: ptr=%p stride=%d enabled=%d\n", i, arrays[i]->Ptr, arrays[i]->Stride, arrays[i]->Enabled);
         printf("--- BufObj %p ptr=%p data=%p\n", arrays[i]->BufferObj, arrays[i]->BufferObj->Pointer, arrays[i]->BufferObj->Data);
         printf("--- Eff. ptr=%p\n", arrayDataPointer(arrays[i]));
         if (!arrayDataPointer(arrays[i]))
            continue;
         for (j = min_index; j <= max_index; j++) {
            int k;
            GLfloat* d = (GLfloat*)arrayDataPointer(arrays[i]);
            printf("%d: ", j);
            for (k = 0; k < arrays[i]->Size; k++) {
               printf("%f\t", d[j * arrays[i]->Stride/4 + k]);
            }
            printf("\n");
         }
      }
   }

   glesCtx->glVertexPointer(arrays[VERT_ATTRIB_POS]->Size,
                            arrays[VERT_ATTRIB_POS]->Type,
                            arrays[VERT_ATTRIB_POS]->Stride,
                            arrayDataPointer(arrays[VERT_ATTRIB_POS]));
   GLES_CHECK_ERROR(ctx);
   glesCtx->glEnableClientState(GL_VERTEX_ARRAY);

   if (arrays[VERT_ATTRIB_NORMAL]->Enabled) {
      glesCtx->glEnableClientState(GL_NORMAL_ARRAY);
      glesCtx->glNormalPointer(arrays[VERT_ATTRIB_NORMAL]->Type,
                               arrays[VERT_ATTRIB_NORMAL]->Stride,
                               arrayDataPointer(arrays[VERT_ATTRIB_NORMAL]));
   } else {
      glesCtx->glDisableClientState(GL_NORMAL_ARRAY);
   }

   if (arrays[VERT_ATTRIB_COLOR0]->Enabled) {
      glesCtx->glEnableClientState(GL_COLOR_ARRAY);
      if (arrays[VERT_ATTRIB_COLOR0]->Size != 4) {
         /*
         if (GLES_DEBUG) {
            fprintf(stderr, "Unsupported color array component count: %d\n", arrays[VERT_ATTRIB_COLOR0]->Size);
         }
         */
         /* Create a temporary vertex color buffer with all four components */
         GLint newColorCount = max_index + 1;
         GLint j;
         allocateColorBuffer(glesCtx, newColorCount * 4);

         switch(arrays[VERT_ATTRIB_COLOR0]->Type) {
         case GL_BYTE:
         case GL_UNSIGNED_BYTE:
            {
               const GLbyte* colors = (const GLbyte*)arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]);
               for (j = 0; j < newColorCount; j++) {
                  glesCtx->colorBuffer[j * 4 + 0] = colors[0];
                  glesCtx->colorBuffer[j * 4 + 1] = (arrays[VERT_ATTRIB_COLOR0]->Size > 1) ? colors[1] : 0;
                  glesCtx->colorBuffer[j * 4 + 2] = (arrays[VERT_ATTRIB_COLOR0]->Size > 2) ? colors[2] : 0;
                  glesCtx->colorBuffer[j * 4 + 3] = (arrays[VERT_ATTRIB_COLOR0]->Size > 3) ? colors[3] : 255;
                  colors += arrays[VERT_ATTRIB_COLOR0]->Stride / sizeof(colors[0]);
               }
               break;
            }
         case GL_SHORT:
         case GL_UNSIGNED_SHORT:
            {
               const GLshort* colors = (const GLshort*)arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]);
               for (j = 0; j < newColorCount; j++) {
                  glesCtx->colorBuffer[j * 4 + 0] = colors[0] >> 8;
                  glesCtx->colorBuffer[j * 4 + 1] = (arrays[VERT_ATTRIB_COLOR0]->Size > 1) ? (colors[1] >> 8): 0;
                  glesCtx->colorBuffer[j * 4 + 2] = (arrays[VERT_ATTRIB_COLOR0]->Size > 2) ? (colors[2] >> 8): 0;
                  glesCtx->colorBuffer[j * 4 + 3] = (arrays[VERT_ATTRIB_COLOR0]->Size > 3) ? (colors[3] >> 8): 255;
                  colors += arrays[VERT_ATTRIB_COLOR0]->Stride / sizeof(colors[0]);
               }
               break;
            }
         case GL_INT:
         case GL_UNSIGNED_INT:
            {
               const GLint* colors = (const GLint*)arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]);
               for (j = 0; j < newColorCount; j++) {
                  glesCtx->colorBuffer[j * 4 + 0] = colors[0] >> 16;
                  glesCtx->colorBuffer[j * 4 + 1] = (arrays[VERT_ATTRIB_COLOR0]->Size > 1) ? (colors[1] >> 16): 0;
                  glesCtx->colorBuffer[j * 4 + 2] = (arrays[VERT_ATTRIB_COLOR0]->Size > 2) ? (colors[2] >> 16): 0;
                  glesCtx->colorBuffer[j * 4 + 3] = (arrays[VERT_ATTRIB_COLOR0]->Size > 3) ? (colors[3] >> 16): 255;
                  colors += arrays[VERT_ATTRIB_COLOR0]->Stride / sizeof(colors[0]);
               }
               break;
            }
         case GL_FLOAT:
            {
               const GLfloat* colors = (const GLfloat*)arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]);
               for (j = 0; j < newColorCount; j++) {
                  glesCtx->colorBuffer[j * 4 + 0] = (GLbyte)(colors[0] * 255);
                  glesCtx->colorBuffer[j * 4 + 1] = (arrays[VERT_ATTRIB_COLOR0]->Size > 1) ? (GLbyte)(colors[1] * 255) : 0;
                  glesCtx->colorBuffer[j * 4 + 2] = (arrays[VERT_ATTRIB_COLOR0]->Size > 2) ? (GLbyte)(colors[2] * 255) : 0;
                  glesCtx->colorBuffer[j * 4 + 3] = (arrays[VERT_ATTRIB_COLOR0]->Size > 3) ? (GLbyte)(colors[3] * 255) : 255;
                  colors += arrays[VERT_ATTRIB_COLOR0]->Stride / sizeof(colors[0]);
               }
               break;
            }
         case GL_DOUBLE:
            {
               const GLdouble* colors = (const GLdouble*)arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]);
               for (j = 0; j < newColorCount; j++) {
                  glesCtx->colorBuffer[j * 4 + 0] = (GLbyte)(colors[0] * 255);
                  glesCtx->colorBuffer[j * 4 + 1] = (arrays[VERT_ATTRIB_COLOR0]->Size > 1) ? (GLbyte)(colors[1] * 255) : 0;
                  glesCtx->colorBuffer[j * 4 + 2] = (arrays[VERT_ATTRIB_COLOR0]->Size > 2) ? (GLbyte)(colors[2] * 255) : 0;
                  glesCtx->colorBuffer[j * 4 + 3] = (arrays[VERT_ATTRIB_COLOR0]->Size > 3) ? (GLbyte)(colors[3] * 255) : 255;
                  colors += arrays[VERT_ATTRIB_COLOR0]->Stride / sizeof(colors[0]);
               }
               break;
            }
         default:
            assert(!"Unsupported color array type");
         }
         glesCtx->glColorPointer(4, GL_UNSIGNED_BYTE, 0, glesCtx->colorBuffer);
      } else {
         glesCtx->glEnableClientState(GL_COLOR_ARRAY);
         glesCtx->glColorPointer(arrays[VERT_ATTRIB_COLOR0]->Size,
                                 arrays[VERT_ATTRIB_COLOR0]->Type,
                                 arrays[VERT_ATTRIB_COLOR0]->Stride,
                                 arrayDataPointer(arrays[VERT_ATTRIB_COLOR0]));
      }
   } else {
      glesCtx->glDisableClientState(GL_COLOR_ARRAY);
   }

   for (i = 0; i < glesCtx->maxTextureUnits - 1; i++) {
      glesCtx->glClientActiveTexture(GL_TEXTURE0 + i);
      if (arrays[VERT_ATTRIB_TEX0 + i]->Enabled && arrays[VERT_ATTRIB_TEX0 + i]->Size > 1) {
         glesCtx->glTexCoordPointer(arrays[VERT_ATTRIB_TEX0 + i]->Size,
                                    arrays[VERT_ATTRIB_TEX0 + i]->Type,
                                    arrays[VERT_ATTRIB_TEX0 + i]->Stride,
                                    arrayDataPointer(arrays[VERT_ATTRIB_TEX0 + i]));
         glesCtx->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      } else {
         glesCtx->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      }
   }
   GLES_CHECK_ERROR(ctx);

   for (i = 0; i < nr_prims; i++) {
      const struct _mesa_prim* p = &prim[i];
      GLint mode       = p->mode;
      GLint indexCount = p->indexed ? p->count : (max_index - min_index + 1);

      switch (mode)
      {
      default:
         /* FIXME: convert geometry */
         if (GLES_DEBUG) {
            fprintf(stderr, "Unsupported geometry mode: 0x%x\n", p->mode);
         }
         mode = GL_LINE_LOOP;
         break;
         /* fallthrough */
      case GL_POINTS:
      case GL_LINES:
      case GL_LINE_LOOP:
      case GL_LINE_STRIP:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
         if (p->indexed) {
            glesCtx->glDrawElements(mode, indexCount, ib->type, ib->ptr);
         } else {
            glesCtx->glDrawArrays(mode, min_index, indexCount);
         }
         break;
      case GL_POLYGON:
         if (p->indexed) {
            glesCtx->glDrawElements(GL_TRIANGLE_FAN, indexCount, ib->type, ib->ptr);
         } else {
            glesCtx->glDrawArrays(GL_TRIANGLE_FAN, min_index, indexCount);
         }
         break;
      case GL_QUADS:
         /* Convert into triangle list */
         {
            GLint newIndexCount = (6 * indexCount / 4);
            allocateIndexBuffer(glesCtx, newIndexCount);

            if (p->indexed) {
               switch (ib->type) {
               case GL_UNSIGNED_BYTE:
                  {
                     const GLbyte* indices = (const GLbyte*)ib->ptr;
                     GLint j, k;

                     for (j = 0, k = 0; j < indexCount; j+=4, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 0];
                        glesCtx->indexBuffer[k + 4] = indices[j + 2];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               case GL_UNSIGNED_SHORT:
                  {
                     const GLshort* indices = (const GLshort*)ib->ptr;
                     GLint j, k;

                     for (j = 0, k = 0; j < indexCount; j+=4, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 0];
                        glesCtx->indexBuffer[k + 4] = indices[j + 2];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               case GL_UNSIGNED_INT:
                  {
                     const GLint* indices = (const GLint*)ib->ptr;
                     GLint j, k;

                     /* FIXME: overflow? */
                     for (j = 0, k = 0; j < indexCount; j+=4, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 0];
                        glesCtx->indexBuffer[k + 4] = indices[j + 2];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               }
            } else {
               GLint j, k;

               for (j = min_index, k = 0; j < min_index + indexCount; j+=4, k+=6) {
                  glesCtx->indexBuffer[k + 0] = j + 0;
                  glesCtx->indexBuffer[k + 1] = j + 1;
                  glesCtx->indexBuffer[k + 2] = j + 2;
                  glesCtx->indexBuffer[k + 3] = j + 0;
                  glesCtx->indexBuffer[k + 4] = j + 2;
                  glesCtx->indexBuffer[k + 5] = j + 3;
               }
            }
            /* Render with the new indices */
            glesCtx->glDrawElements(GL_TRIANGLES, newIndexCount, GL_UNSIGNED_SHORT, glesCtx->indexBuffer);
            GLES_CHECK_ERROR(ctx);
            break;
         }
      case GL_QUAD_STRIP:
         /* Convert into triangle list */
         {
            GLint newIndexCount = (6 * (indexCount - 2) / 2);
            allocateIndexBuffer(glesCtx, newIndexCount);

            if (p->indexed) {
               switch (ib->type) {
               case GL_UNSIGNED_BYTE:
                  {
                     const GLbyte* indices = (const GLbyte*)ib->ptr;
                     GLint j, k;

                     for (j = 0, k = 0; j < indexCount - 4; j+=2, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 2];
                        glesCtx->indexBuffer[k + 4] = indices[j + 1];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               case GL_UNSIGNED_SHORT:
                  {
                     const GLshort* indices = (const GLshort*)ib->ptr;
                     GLint j, k;

                     for (j = 0, k = 0; j < indexCount - 4; j+=2, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 2];
                        glesCtx->indexBuffer[k + 4] = indices[j + 1];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               case GL_UNSIGNED_INT:
                  {
                     const GLint* indices = (const GLint*)ib->ptr;
                     GLint j, k;

                     /* FIXME: overflow? */
                     for (j = 0, k = 0; j < indexCount - 4; j+=2, k+=6) {
                        glesCtx->indexBuffer[k + 0] = indices[j + 0];
                        glesCtx->indexBuffer[k + 1] = indices[j + 1];
                        glesCtx->indexBuffer[k + 2] = indices[j + 2];
                        glesCtx->indexBuffer[k + 3] = indices[j + 2];
                        glesCtx->indexBuffer[k + 4] = indices[j + 1];
                        glesCtx->indexBuffer[k + 5] = indices[j + 3];
                     }
                  }
                  break;
               }
            } else {
               GLint j, k;

               for (j = min_index, k = 0; j <= min_index + indexCount - 4; j+=2, k+=6) {
                  glesCtx->indexBuffer[k + 0] = j + 0;
                  glesCtx->indexBuffer[k + 1] = j + 1;
                  glesCtx->indexBuffer[k + 2] = j + 2;
                  glesCtx->indexBuffer[k + 3] = j + 2;
                  glesCtx->indexBuffer[k + 4] = j + 1;
                  glesCtx->indexBuffer[k + 5] = j + 3;
               }
               /*printf("%d -> %d, %d\n", indexCount, newIndexCount, k);*/
            }
            /* Render with the new indices */
            glesCtx->glDrawElements(GL_TRIANGLES, newIndexCount, GL_UNSIGNED_SHORT, glesCtx->indexBuffer);
            GLES_CHECK_ERROR(ctx);
            break;
         }
      }
   }

   {
         const GLfloat vertices[] = {
            0, -1, -3,
            -1,  1, -3,
            1,  1, -3,
         };

         /*glesCtx->glMatrixMode(GL_PROJECTION);
         glesCtx->glLoadIdentity();*/
         /*glesCtx->glViewport(0, 0, 300, 300);*/
         /*glesCtx->glOrthof(-2.f, 2.f, 2.f, -2.f, 1.f, 1000.f);*/
         /*
         glesCtx->glMatrixMode(GL_MODELVIEW);
         glesCtx->glLoadIdentity();
         */
         /*glesCtx->glDisable(GL_DEPTH_TEST);*/
         /*glesCtx->glEnableClientState(GL_VERTEX_ARRAY);
         glesCtx->glVertexPointer(3, GL_FLOAT, 0, vertices);*/
         /*glesCtx->glDrawArrays(GL_TRIANGLES, 0, 3);*/
   }
   
   GLES_CHECK_ERROR(ctx);
}

static void glesDDVertexPointer(GLcontext *ctx, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *ptr)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glVertexPointer(size, type, stride, ptr);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDNormalPointer(GLcontext *ctx, GLenum type,
                                GLsizei stride, const GLvoid *ptr)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glNormalPointer(type, stride, ptr);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDColorPointer(GLcontext *ctx, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *ptr)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glColorPointer(size, type, stride, ptr);
   GLES_CHECK_ERROR(ctx);
}

static void glesDDTexCoordPointer(GLcontext *ctx, GLint size, GLenum type,
                                GLsizei stride, const GLvoid *ptr)
{
   GET_GLES_CONTEXT(ctx);
   glesCtx->glTexCoordPointer(size, type, stride, ptr);
   GLES_CHECK_ERROR(ctx);
}

static const GLubyte *glesDDGetString(GLcontext * ctx, GLenum name)
{
   GET_GLES_CONTEXT(ctx);
   if (name != GL_EXTENSIONS) {
      const GLubyte* str = glesCtx->glGetString(name);
      GLES_CHECK_ERROR(ctx);
   }
   return NULL;
}

static const struct tnl_pipeline_stage *gles_pipeline[] = {
#if 0
   &_tnl_vertex_transform_stage,
   &_tnl_normal_transform_stage,
   &_tnl_lighting_stage,
   &_tnl_fog_coordinate_stage,
   &_tnl_texgen_stage,
   &_tnl_texture_transform_stage,
   &_tnl_point_attenuation_stage,
#if defined(FEATURE_NV_vertex_program) || defined(FEATURE_ARB_vertex_program)
   &_tnl_vertex_program_stage,
#endif
   &_tnl_render_stage,
#endif
   0,
};

typedef void (*GLESFunc)(void);

#define GLES_LOAD_FUNC(NAME) \
   do { \
      GLESFunc* _func = (GLESFunc*)dlsym(glesCtx->glesLibrary, #NAME); \
      *((GLESFunc*)&(glesCtx->NAME)) = _func; \
      if (!glesCtx->NAME) { \
         fprintf(stderr, "Unable to load OpenGL ES symbol: %s\n", #NAME); \
         assert(0); \
      } \
   } while( 0)

int
glesDDInitGLESMesaContext(glesMesaContext glesCtx)
{
   GLcontext *ctx = glesCtx->glCtx;
   struct vbo_context *vbo;

   /* Load the OpenGL ES library */
   glesCtx->glesLibrary = dlopen(GLES_LIBRARY_NAME, RTLD_NOW);

   if (!glesCtx->glesLibrary) {
      fprintf(stderr, "Unable to open OpenGL ES library: %s\n", GLES_LIBRARY_NAME);
      return 0;
   }

   /* Get the OpenGL ES function pointers */
   GLES_LOAD_FUNC(glActiveTexture);
   GLES_LOAD_FUNC(glAlphaFunc);
   GLES_LOAD_FUNC(glBindTexture);
   GLES_LOAD_FUNC(glBlendFunc);
   GLES_LOAD_FUNC(glClear);
   GLES_LOAD_FUNC(glClearColor);
   GLES_LOAD_FUNC(glClearDepthf);
   GLES_LOAD_FUNC(glClearStencil);
   GLES_LOAD_FUNC(glClientActiveTexture);
   GLES_LOAD_FUNC(glClipPlanef);
   GLES_LOAD_FUNC(glColorMask);
   GLES_LOAD_FUNC(glColorPointer);
   GLES_LOAD_FUNC(glCopyTexImage2D);
   GLES_LOAD_FUNC(glCopyTexSubImage2D);
   GLES_LOAD_FUNC(glCullFace);
   GLES_LOAD_FUNC(glDeleteTextures);
   GLES_LOAD_FUNC(glDepthFunc);
   GLES_LOAD_FUNC(glDepthMask);
   GLES_LOAD_FUNC(glDepthRangef);
   GLES_LOAD_FUNC(glDisable);
   GLES_LOAD_FUNC(glDisableClientState);
   GLES_LOAD_FUNC(glDrawArrays);
   GLES_LOAD_FUNC(glDrawElements);
   GLES_LOAD_FUNC(glEnable);
   GLES_LOAD_FUNC(glEnableClientState);
   GLES_LOAD_FUNC(glFinish);
   GLES_LOAD_FUNC(glFlush);
   GLES_LOAD_FUNC(glFogfv);
   GLES_LOAD_FUNC(glFrontFace);
   GLES_LOAD_FUNC(glGetBooleanv);
   GLES_LOAD_FUNC(glGetFloatv);
   GLES_LOAD_FUNC(glGetIntegerv);
   GLES_LOAD_FUNC(glGetError);
   GLES_LOAD_FUNC(glGetString);
   GLES_LOAD_FUNC(glHint);
   GLES_LOAD_FUNC(glLightModelfv);
   GLES_LOAD_FUNC(glLightfv);
   GLES_LOAD_FUNC(glLineWidth);
   GLES_LOAD_FUNC(glLogicOp);
   GLES_LOAD_FUNC(glLoadMatrixf);
   GLES_LOAD_FUNC(glMatrixMode);
   GLES_LOAD_FUNC(glNormalPointer);
   GLES_LOAD_FUNC(glPixelStorei);
   GLES_LOAD_FUNC(glPointParameterfv);
   GLES_LOAD_FUNC(glPointSize);
   GLES_LOAD_FUNC(glPolygonOffset);
   GLES_LOAD_FUNC(glReadPixels);
   GLES_LOAD_FUNC(glScissor);
   GLES_LOAD_FUNC(glShadeModel);
   GLES_LOAD_FUNC(glStencilFunc);
   GLES_LOAD_FUNC(glStencilMask);
   GLES_LOAD_FUNC(glStencilOp);
   GLES_LOAD_FUNC(glTexCoordPointer);
   GLES_LOAD_FUNC(glTexEnvfv);
   GLES_LOAD_FUNC(glTexImage2D);
   GLES_LOAD_FUNC(glTexParameterf);
   GLES_LOAD_FUNC(glVertexPointer);
   GLES_LOAD_FUNC(glViewport);
   GLES_LOAD_FUNC(glOrthof);
   GLES_LOAD_FUNC(glLoadIdentity);
   GLES_LOAD_FUNC(glTranslatef);
   GLES_LOAD_FUNC(glRotatef);
   GLES_LOAD_FUNC(glScalef);
   GLES_LOAD_FUNC(glColor4f);

   /* Read the engine features */
   glesCtx->glGetIntegerv(GL_MAX_TEXTURE_UNITS, &glesCtx->maxTextureUnits);

   /* Initialize the software rasterizer and helper modules.
    */
   /*_swrast_CreateContext(ctx);*/
   _vbo_CreateContext(ctx);
   _tnl_CreateContext(ctx);
   _swsetup_CreateContext(ctx);

   /* Install customized pipeline */
   /*_tnl_destroy_pipeline(ctx);
   _tnl_install_pipeline(ctx, gles_pipeline);*/

   /* Install our draw method */
   vbo = vbo_context(ctx);
   vbo->draw_prims = glesDDDrawPrimitives;

   glesSetupDDPointers(ctx);

   /* Tell the software rasterizer to use pixel fog always.
    */
    /*
   _swrast_allow_vertex_fog(ctx, GL_FALSE);
   _swrast_allow_pixel_fog(ctx, GL_TRUE);
   _tnl_allow_vertex_fog( ctx, GL_FALSE );
   _tnl_allow_pixel_fog( ctx, GL_TRUE );
   */

   glesDDInitExtensions(ctx);

   return 1;
}

/* Undo the above.
 */
void
glesDDDestroyGLESMesaContext(glesMesaContext glesCtx)
{
   GLcontext *ctx = glesCtx->glCtx;

   if (glesCtx->glesLibrary) {
      dlclose(glesCtx->glesLibrary);
      glesCtx->glesLibrary = 0;
   }

   if (glesCtx->indexBuffer) {
      free(glesCtx->indexBuffer);
   }
   
   if (glesCtx->colorBuffer) {
      free(glesCtx->colorBuffer);
   }
   
   _swsetup_DestroyContext(ctx);
   _tnl_DestroyContext(ctx);
   _vbo_DestroyContext(ctx);
   /*_swrast_DestroyContext(ctx);*/
}

void
glesDDInitExtensions(GLcontext * ctx)
{
   _mesa_enable_extension(ctx, "GL_ARB_point_sprite");
   _mesa_enable_extension(ctx, "GL_EXT_point_parameters");
   _mesa_enable_extension(ctx, "GL_EXT_paletted_texture");

   /* OpenGL ES 1.1 */
   _mesa_enable_extension(ctx, "GL_ARB_multitexture");
   _mesa_enable_extension(ctx, "GL_ARB_texture_env_combine");
   _mesa_enable_extension(ctx, "GL_EXT_texture_env_combine");
   _mesa_enable_extension(ctx, "GL_ARB_vertex_buffer_object");
}

#if 0
#define _NEW_MODELVIEW		0x1        /**< __GLcontextRec::ModelView */
#define _NEW_PROJECTION		0x2        /**< __GLcontextRec::Projection */
#define _NEW_TEXTURE_MATRIX	0x4        /**< __GLcontextRec::TextureMatrix */
#define _NEW_COLOR_MATRIX	0x8        /**< __GLcontextRec::ColorMatrix */
#define _NEW_ACCUM		0x10       /**< __GLcontextRec::Accum */
#define _NEW_COLOR		0x20       /**< __GLcontextRec::Color */
#define _NEW_DEPTH		0x40       /**< __GLcontextRec::Depth */
#define _NEW_EVAL		0x80       /**< __GLcontextRec::Eval, __GLcontextRec::EvalMap */
#define _NEW_FOG		0x100      /**< __GLcontextRec::Fog */
#define _NEW_HINT		0x200      /**< __GLcontextRec::Hint */
#define _NEW_LIGHT		0x400      /**< __GLcontextRec::Light */
#define _NEW_LINE		0x800      /**< __GLcontextRec::Line */
#define _NEW_PIXEL		0x1000     /**< __GLcontextRec::Pixel */
#define _NEW_POINT		0x2000     /**< __GLcontextRec::Point */
#define _NEW_POLYGON		0x4000     /**< __GLcontextRec::Polygon */
#define _NEW_POLYGONSTIPPLE	0x8000     /**< __GLcontextRec::PolygonStipple */
#define _NEW_SCISSOR		0x10000    /**< __GLcontextRec::Scissor */
#define _NEW_STENCIL		0x20000    /**< __GLcontextRec::Stencil */
#define _NEW_TEXTURE		0x40000    /**< __GLcontextRec::Texture */
#define _NEW_TRANSFORM		0x80000    /**< __GLcontextRec::Transform */
#define _NEW_VIEWPORT		0x100000   /**< __GLcontextRec::Viewport */
#define _NEW_PACKUNPACK		0x200000   /**< __GLcontextRec::Pack, __GLcontextRec::Unpack */
#define _NEW_ARRAY	        0x400000   /**< __GLcontextRec::Array */
#define _NEW_RENDERMODE		0x800000   /**< __GLcontextRec::RenderMode, __GLcontextRec::Feedback, __GLcontextRec::Select */
#define _NEW_BUFFERS            0x1000000  /**< __GLcontextRec::Visual, __GLcontextRec::DrawBuffer, */
#define _NEW_MULTISAMPLE        0x2000000  /**< __GLcontextRec::Multisample */
#define _NEW_TRACK_MATRIX       0x4000000  /**< __GLcontextRec::VertexProgram */
#define _NEW_PROGRAM            0x8000000  /**< __GLcontextRec::VertexProgram */
#define _NEW_ALL ~0
#endif

static void glesDDUpdateState(GLcontext * ctx, GLuint new_state)
{
   GET_GLES_CONTEXT(ctx);
   
   if (GLES_DEBUG && 0) {
      fprintf(stderr, "glesDDUpdateState: %x\n", new_state);
   }

   if (new_state & _NEW_MODELVIEW) {
      /*glesCtx->glMatrixMode(GL_MODELVIEW);
      glesCtx->glLoadMatrixf(ctx->_ModelViewMatrix.m);*/
      /*glesCtx->glLoadIdentity();
      glesCtx->glTranslatef(0, 0, -10);*/
      GLES_CHECK_ERROR(ctx);
   }

   if (new_state & (_NEW_PROJECTION | _NEW_MODELVIEW)) {
      glesCtx->glMatrixMode(GL_PROJECTION);
      glesCtx->glLoadMatrixf(ctx->_ModelProjectMatrix.m);
      /*glesCtx->glLoadIdentity();
      glesCtx->glOrthof(-50.f, 50.f, 50.f, -50.f, .1, 100.f);
      glesCtx->glViewport(0, 0, 100, 100);*/
      GLES_CHECK_ERROR(ctx);
   }

   if (new_state & _NEW_TEXTURE_MATRIX) {
      glesCtx->glMatrixMode(GL_TEXTURE);
      glesCtx->glLoadMatrixf(ctx->TextureMatrixStack[ctx->Texture.CurrentUnit].Top->m);
      GLES_CHECK_ERROR(ctx);
   }

   /*swrast_InvalidateState(ctx, new_state);*/
   _vbo_InvalidateState(ctx, new_state);
   _tnl_InvalidateState(ctx, new_state);
   _swsetup_InvalidateState(ctx, new_state);
}

#define GLES_HOOK_FUNCTION(NAME) \
   ctx->Driver.NAME = glesDD ## NAME;

void
glesSetupDDPointers(GLcontext * ctx)
{
   if (GLES_DEBUG) {
      fprintf(stderr, "glesSetupDDPointers()\n");
   }

   /* Redirect all the supported functions */
   GLES_HOOK_FUNCTION(UpdateState);
   GLES_HOOK_FUNCTION(GetString);
   GLES_HOOK_FUNCTION(Clear);
   GLES_HOOK_FUNCTION(Finish);
   GLES_HOOK_FUNCTION(Flush);
   GLES_HOOK_FUNCTION(Accum);
   GLES_HOOK_FUNCTION(ReadPixels);
   GLES_HOOK_FUNCTION(DrawPixels);
   GLES_HOOK_FUNCTION(CopyPixels);
   GLES_HOOK_FUNCTION(Bitmap);
   GLES_HOOK_FUNCTION(TexImage1D);
   GLES_HOOK_FUNCTION(TexImage2D);
   GLES_HOOK_FUNCTION(TexImage3D);
   GLES_HOOK_FUNCTION(TexSubImage1D);
   GLES_HOOK_FUNCTION(TexSubImage2D);
   GLES_HOOK_FUNCTION(TexSubImage3D);
   GLES_HOOK_FUNCTION(GetTexImage);
   GLES_HOOK_FUNCTION(CopyTexImage1D);
   GLES_HOOK_FUNCTION(CopyTexImage2D);
   GLES_HOOK_FUNCTION(CopyTexSubImage1D);
   GLES_HOOK_FUNCTION(CopyTexSubImage2D);
   GLES_HOOK_FUNCTION(CopyTexSubImage3D);
   GLES_HOOK_FUNCTION(TestProxyTexImage);
   GLES_HOOK_FUNCTION(CompressedTexImage1D);
   GLES_HOOK_FUNCTION(CompressedTexImage2D);
   GLES_HOOK_FUNCTION(CompressedTexImage3D);
   GLES_HOOK_FUNCTION(CompressedTexSubImage1D);
   GLES_HOOK_FUNCTION(CompressedTexSubImage2D);
   GLES_HOOK_FUNCTION(CompressedTexSubImage3D);
   GLES_HOOK_FUNCTION(CompressedTextureSize);
   GLES_HOOK_FUNCTION(GetCompressedTexImage);
   GLES_HOOK_FUNCTION(BindTexture);
   GLES_HOOK_FUNCTION(DeleteTexture);
   GLES_HOOK_FUNCTION(IsTextureResident);
   GLES_HOOK_FUNCTION(PrioritizeTexture);
   GLES_HOOK_FUNCTION(ActiveTexture);
   GLES_HOOK_FUNCTION(UpdateTexturePalette);
   GLES_HOOK_FUNCTION(CopyColorTable);
   GLES_HOOK_FUNCTION(CopyColorSubTable);
   GLES_HOOK_FUNCTION(CopyConvolutionFilter1D);
   GLES_HOOK_FUNCTION(CopyConvolutionFilter2D);
   GLES_HOOK_FUNCTION(BindProgram);
   GLES_HOOK_FUNCTION(DeleteProgram);
   GLES_HOOK_FUNCTION(ProgramStringNotify);
   GLES_HOOK_FUNCTION(GetProgramRegister);
   GLES_HOOK_FUNCTION(IsProgramNative);
   GLES_HOOK_FUNCTION(AlphaFunc);
   GLES_HOOK_FUNCTION(BlendColor);
   GLES_HOOK_FUNCTION(BlendEquationSeparate);
   GLES_HOOK_FUNCTION(BlendFuncSeparate);
   GLES_HOOK_FUNCTION(ClearColor);
   GLES_HOOK_FUNCTION(ClearDepth);
   GLES_HOOK_FUNCTION(ClearIndex);
   GLES_HOOK_FUNCTION(ClearStencil);
   GLES_HOOK_FUNCTION(ClipPlane);
   GLES_HOOK_FUNCTION(ColorMask);
   GLES_HOOK_FUNCTION(ColorMaterial);
   GLES_HOOK_FUNCTION(CullFace);
   GLES_HOOK_FUNCTION(FrontFace);
   GLES_HOOK_FUNCTION(DepthFunc);
   GLES_HOOK_FUNCTION(DepthMask);
   GLES_HOOK_FUNCTION(DepthRange);
   GLES_HOOK_FUNCTION(DrawBuffer);
   GLES_HOOK_FUNCTION(DrawBuffers);
   GLES_HOOK_FUNCTION(Enable);
   GLES_HOOK_FUNCTION(Fogfv);
   GLES_HOOK_FUNCTION(Hint);
   GLES_HOOK_FUNCTION(IndexMask);
   GLES_HOOK_FUNCTION(Lightfv);
   GLES_HOOK_FUNCTION(LightModelfv);
   GLES_HOOK_FUNCTION(LineStipple);
   GLES_HOOK_FUNCTION(LineWidth);
   GLES_HOOK_FUNCTION(LogicOpcode);
   GLES_HOOK_FUNCTION(PointParameterfv);
   GLES_HOOK_FUNCTION(PointSize);
   GLES_HOOK_FUNCTION(PolygonMode);
   GLES_HOOK_FUNCTION(PolygonOffset);
   GLES_HOOK_FUNCTION(PolygonStipple);
   GLES_HOOK_FUNCTION(ReadBuffer);
   GLES_HOOK_FUNCTION(RenderMode);
   GLES_HOOK_FUNCTION(Scissor);
   GLES_HOOK_FUNCTION(ShadeModel);
   GLES_HOOK_FUNCTION(StencilFuncSeparate);
   GLES_HOOK_FUNCTION(StencilMaskSeparate);
   GLES_HOOK_FUNCTION(StencilOpSeparate);
   GLES_HOOK_FUNCTION(TexGen);
   GLES_HOOK_FUNCTION(TexEnv);
   GLES_HOOK_FUNCTION(TexParameter);
   GLES_HOOK_FUNCTION(TextureMatrix);
   GLES_HOOK_FUNCTION(Viewport);
   GLES_HOOK_FUNCTION(GetBooleanv);
   GLES_HOOK_FUNCTION(GetDoublev);
   GLES_HOOK_FUNCTION(GetFloatv);
   GLES_HOOK_FUNCTION(GetIntegerv);
   GLES_HOOK_FUNCTION(GetPointerv);
   GLES_HOOK_FUNCTION(VertexPointer);
   GLES_HOOK_FUNCTION(NormalPointer);
   GLES_HOOK_FUNCTION(ColorPointer);
   GLES_HOOK_FUNCTION(TexCoordPointer);
   glesDDUpdateState(ctx, ~0);
   GLES_CHECK_ERROR(ctx);
}

#undef GLES_HOOK_FUNCTION

#else


/*
 * Need this to provide at least one external definition.
 */

extern int gl_GLES_dummy_function_dd(void);
int
gl_GLES_dummy_function_dd(void)
{
   return 0;
}

#endif /* GLES */
