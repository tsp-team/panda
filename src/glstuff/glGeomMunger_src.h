// Filename: glGeomMunger_src.h
// Created by:  drose (10Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"
#include "textureAttrib.h"
#include "texGenAttrib.h"
#include "renderState.h"

class CLP(GeomContext);

////////////////////////////////////////////////////////////////////
//       Class : GLGeomMunger
// Description : This specialization on GeomMunger finesses vertices
//               for OpenGL rendering.  In particular, it makes sure
//               colors aren't stored in DirectX's packed_argb format.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(GeomMunger) : public StandardMunger {
public:
  INLINE CLP(GeomMunger)(GraphicsStateGuardian *gsg, const RenderState *state);
  virtual ~CLP(GeomMunger)();

protected:
  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                    const GeomVertexAnimationSpec &animation);
  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual int geom_compare_to_impl(const GeomMunger *other) const;

public:
  INLINE void *operator new(size_t size);

private:
  CPT(TextureAttrib) _texture;
  CPT(TexGenAttrib) _tex_gen;

  typedef pset<CLP(GeomContext) *> GeomContexts;
  GeomContexts _geom_contexts;

  static GeomMunger *_deleted_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StandardMunger::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomMunger",
                  StandardMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CLP(GeomContext);
};

#include "glGeomMunger_src.I"

