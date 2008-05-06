// Filename: geoMipTerrain.h
// Created by:  pro-rsoft (29jun07)
// Last updated by: pro-rsoft (03mar08)
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

#ifndef GEOMIPTERRAIN_H
#define GEOMIPTERRAIN_H

#include "pandabase.h"

#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"

#include "pnmImage.h"
#include "nodePath.h"

#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : GeoMipTerrain
// Description : GeoMipTerrain, meaning Panda3D GeoMipMapping, can convert
//               a heightfield image into a 3D terrain, consisting
//               of several GeomNodes. It uses the GeoMipMapping
//                algorithm, or Geometrical MipMapping, based on
//               the LOD (Level of Detail) algorithm. For more
//               information about the GeoMipMapping algoritm, see
//               this paper, written by Willem H. de Boer:
//               http://flipcode.com/articles/article_geomipmaps.pdf
//               
////////////////////////////////////////////////////////////////////
class GeoMipTerrain {
PUBLISHED:
  INLINE GeoMipTerrain(const string &name);
  INLINE ~GeoMipTerrain();
  
  INLINE PNMImage &heightfield();
  INLINE bool set_heightfield(const Filename &filename,
                                    PNMFileType *type = NULL);
  INLINE bool set_heightfield(const PNMImage &image);
  INLINE bool set_heightfield(const Texture *image);
  INLINE PNMImage &color_map();
  INLINE bool set_color_map(const Filename &filename,
                                  PNMFileType *type = NULL);
  INLINE bool set_color_map(const PNMImage &image);
  INLINE bool set_color_map(const Texture *image);
  INLINE bool has_color_map();
  INLINE void clear_color_map();
  double get_elevation(double x, double y);
  LVector3f get_normal(int x, int y);
  INLINE LVector3f get_normal(unsigned short mx, unsigned short my, 
                                                          int x,int y);
  INLINE void set_bruteforce(bool bf);
  INLINE bool get_bruteforce();

  // The flatten mode specifies whether the terrain nodes are flattened
  // together after each terrain update.
  enum AutoFlattenMode {
    // FM_off: don't ever flatten the terrain.
    AFM_off     = 0,
    // FM_light: the terrain is flattened using flatten_light.
    AFM_light   = 1,
    // FM_medium: the terrain is flattened using flatten_medium.
    AFM_medium  = 2,
    // FM_strong: the terrain is flattened using flatten_strong.
    AFM_strong  = 3,
  };

  INLINE void set_auto_flatten(int mode);

  // The focal point is the point at which the terrain will have the
  // lowest level of detail (highest quality). Parts farther away
  // from the focal point will hae a higher level of detail. The
  // focal point is not taken in respect if bruteforce is set true.
  INLINE void set_focal_point(LPoint2d fp);
  INLINE void set_focal_point(LPoint2f fp);
  INLINE void set_focal_point(LPoint3d fp);
  INLINE void set_focal_point(LPoint3f fp);
  INLINE void set_focal_point(double x, double y);
  INLINE void set_focal_point(NodePath fnp);
  INLINE NodePath get_focal_point() const;
  INLINE NodePath get_root() const;

  INLINE void set_min_level(unsigned short minlevel);
  INLINE unsigned short get_min_level();
  INLINE unsigned short get_block_size();
  INLINE void set_block_size(unsigned short newbs);
  INLINE bool is_dirty();
  INLINE float get_factor();
  INLINE void  set_factor(float factor);
  INLINE const NodePath get_block_node_path(unsigned short mx,
                                            unsigned short my);
  INLINE LVecBase2f get_block_from_pos(double x, double y);
  
  void generate();
  bool update();
  
private:


  NodePath generate_block(unsigned short mx, unsigned short my, unsigned short level);
  bool update_block(unsigned short mx, unsigned short my,
                    signed short level = -1, bool forced = false);
  void calc_levels();
  void auto_flatten();
  bool root_flattened();
  
  INLINE std::string int_to_str(int i);
  INLINE int str_to_int(std::string str);
  INLINE bool is_power_of_two(unsigned int i);
  INLINE float f_part(float i);
  INLINE double f_part(double i);
  INLINE int sfav(int n, int powlevel, int mypowlevel);
  INLINE double get_pixel_value(int x, int y);
  INLINE double get_pixel_value(unsigned short mx, unsigned short my, int x, int y);
  INLINE unsigned short lod_decide(unsigned short mx, unsigned short my);

  NodePath _root;
  int _auto_flatten;
  bool _root_flattened;
  PNMImage _heightfield;
  PNMImage _color_map;
  bool _is_dirty;
  bool _has_color_map;
  unsigned int _xsize;
  unsigned int _ysize;
  float _factor;
  unsigned short _block_size;
  bool _bruteforce;
  NodePath _focal_point;
  bool _focal_is_temporary;
  unsigned short _min_level;
  pvector<pvector<NodePath> > _blocks;
  pvector<pvector<unsigned short> > _levels;
  pvector<pvector<unsigned short> > _old_levels;
  
};

#include "geoMipTerrain.I"

#endif /*GEOMIPTERRAIN_H*/
