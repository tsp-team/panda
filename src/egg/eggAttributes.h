// Filename: eggAttributes.h
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGATTRIBUTES_H
#define EGGATTRIBUTES_H

#include <pandabase.h>

#include "eggMorphList.h"

#include <typeHandle.h>
#include <luse.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggAttributes
// Description : The set of attributes that may be applied to vertices
//               as well as polygons, such as surface normal and
//               color.
//
//               This class cannot inherit from EggObject, because it
//               causes problems at the EggPolygon level with multiple
//               appearances of the EggObject base class.  And making
//               EggObject a virtual base class is just no fun.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggAttributes {
public:
  EggAttributes();
  EggAttributes(const EggAttributes &copy);
  EggAttributes &operator = (const EggAttributes &copy);
  virtual ~EggAttributes();

  INLINE bool has_normal() const;
  INLINE const Normald &get_normal() const;
  INLINE void set_normal(const Normald &normal);
  INLINE void clear_normal();

  INLINE bool has_uv() const;
  INLINE const TexCoordd &get_uv() const;
  INLINE void set_uv(const TexCoordd &texCoord);
  INLINE void clear_uv();

  INLINE bool has_color() const;
  INLINE const Colorf &get_color() const;
  INLINE void set_color(const Colorf &Color);
  INLINE void clear_color();

  void write(ostream &out, int indent_level) const;
  bool sorts_less_than(const EggAttributes &other) const;

  void transform(const LMatrix4d &mat);

  EggMorphNormalList _dnormals;
  EggMorphTexCoordList _duvs;
  EggMorphColorList _drgbas;

private:
  enum Flags {
    F_has_normal = 0x001,
    F_has_uv     = 0x002,
    F_has_color  = 0x004,
  };

  int _flags;
  Normald _normal;
  TexCoordd _uv;
  Colorf _color;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "EggAttributes");
  }

private:
  static TypeHandle _type_handle;
};

#include "eggAttributes.I"

#endif

