// Filename: glSavedFrameBuffer.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef GLSAVEDFRAMEBUFFER_H
#define GLSAVEDFRAMEBUFFER_H

#include <pandabase.h>

#include <savedFrameBuffer.h>
#include <texture.h>
#include <textureContext.h>
#include <pixelBuffer.h>


////////////////////////////////////////////////////////////////////
//       Class : GLSavedFrameBuffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL GLSavedFrameBuffer : public SavedFrameBuffer {
public:
  INLINE GLSavedFrameBuffer(const RenderBuffer &buffer,
                            CPT(DisplayRegion) dr);
  INLINE ~GLSavedFrameBuffer();

  PT(Texture) _back_rgba;
  PT(PixelBuffer) _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedFrameBuffer::init_type();
    register_type(_type_handle, "GLSavedFrameBuffer",
                  SavedFrameBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "glSavedFrameBuffer.I"

#endif

