
/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees  and is
   provided without guarantee or warrantee expressed or  implied. This
   program is -not- in the public domain. */

#ifdef _WIN32
#include <windows.h>
#include <assert.h>
#endif

#include "vixen.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "render/texfont.h"

#if 0
/* Uncomment to debug various scenarios. */
#undef GL_VERSION_1_1
#undef GL_EXT_texture_object
#undef GL_EXT_texture
#endif

#ifndef GL_VERSION_1_1
#if defined(GL_EXT_texture_object) && defined(GL_EXT_texture)
#define glGenTextures glGenTexturesEXT
#define glBindTexture glBindTextureEXT
#ifndef GL_INTENSITY4
#define GL_INTENSITY4 GL_INTENSITY4_EXT
#endif
int useLuminanceAlpha = 0;
#else
#define USE_DISPLAY_LISTS
/* Intensity texture format not in OpenGL 1.0; added by the EXT_texture
   extension and now part of OpenGL 1.1. */
int useLuminanceAlpha = 1;
#endif
#else
int useLuminanceAlpha = 0;
#endif

GLuint
txfEstablishTexture(
  TexFont * txf,
  GLuint texobj,
  GLboolean setupMipmaps)
{
  if (txf->texobj == 0) {
    if (texobj == 0) {
#if !defined(USE_DISPLAY_LISTS)
      glGenTextures(1, &txf->texobj);
#else
      txf->texobj = glGenLists(1);
#endif
    } else {
      txf->texobj = texobj;
    }
  }
#if !defined(USE_DISPLAY_LISTS)
  glBindTexture(GL_TEXTURE_2D, txf->texobj);
#else
  glNewList(txf->texobj, GL_COMPILE);
#endif

#if 1
  /* XXX Indigo2 IMPACT in IRIX 5.3 and 6.2 does not support the GL_INTENSITY
     internal texture format. Sigh. Win32 non-GLX users should disable this
     code. */
  if (useLuminanceAlpha == 0) {
    char *vendor, *renderer, *version;

    renderer = (char *) glGetString(GL_RENDERER);
    vendor = (char *) glGetString(GL_VENDOR);
    if (!strcmp(vendor, "SGI") && !strncmp(renderer, "IMPACT", 6)) {
      version = (char *) glGetString(GL_VERSION);
      if (!strcmp(version, "1.0 Irix 6.2") ||
        !strcmp(version, "1.0 Irix 5.3")) {
        unsigned char *latex;
        int width = txf->tex_width;
        int height = txf->tex_height;
        int i;

        useLuminanceAlpha = 1;
        latex = (unsigned char *) calloc(width * height * 2, 1);
        /* XXX unprotected alloc. */
        for (i = 0; i < height * width; i++) {
          latex[i * 2] = txf->teximage[i];
          latex[i * 2 + 1] = txf->teximage[i];
        }
        free(txf->teximage);
        txf->teximage = latex;
      }
    }
  }
#endif

  if (useLuminanceAlpha) {
    if (setupMipmaps) {
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA,
        txf->tex_width, txf->tex_height,
        GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
        txf->tex_width, txf->tex_height, 0,
        GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, txf->teximage);
    }
  } else {
#if defined(GL_VERSION_1_1) || defined(GL_EXT_texture)
    /* Use GL_INTENSITY4 as internal texture format since we want to use as
       little texture memory as possible. */
    if (setupMipmaps) {
      gluBuild2DMipmaps(GL_TEXTURE_2D, GL_INTENSITY4,
        txf->tex_width, txf->tex_height,
        GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4,
        txf->tex_width, txf->tex_height, 0,
        GL_LUMINANCE, GL_UNSIGNED_BYTE, txf->teximage);
    }
#else
    abort();            /* Should not get here without EXT_texture or OpenGL
                           1.1. */
#endif
  }

#if defined(USE_DISPLAY_LISTS)
  glEndList();
  glCallList(txf->texobj);
#endif
  return txf->texobj;
}

void
txfBindFontTexture(
  TexFont * txf)
{
#if !defined(USE_DISPLAY_LISTS)
  glBindTexture(GL_TEXTURE_2D, txf->texobj);
#else
  glCallList(txf->texobj);
#endif
}

