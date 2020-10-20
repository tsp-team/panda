/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file motionBlur.h
 * @author lachbr
 * @date 2020-09-03
 *
 * Image-space motion blur.
 */

#ifndef MOTIONBLUR_H
#define MOTIONBLUR_H

#include "config_postprocess.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "pta_float.h"
#include "pta_LVecBase4.h"
#include "displayRegion.h"
#include "luse.h"

extern ConfigVariableBool mat_motion_blur_enabled;
extern ConfigVariableBool mat_motion_blur_forward_enabled;
extern ConfigVariableDouble mat_motion_blur_falling_min;
extern ConfigVariableDouble mat_motion_blur_falling_max;
extern ConfigVariableDouble mat_motion_blur_falling_intensity;
extern ConfigVariableDouble mat_motion_blur_rotation_intensity;
extern ConfigVariableDouble mat_motion_blur_strength;
extern ConfigVariableDouble mat_motion_blur_percent_of_screen_max;

class GraphicsOutput;
class Texture;
class CallbackData;

/**
 * Draws a screen-space quad over the scene and performs image-space
 * motion blur.
 */
class EXPCL_PANDA_POSTPROCESS MotionBlur : public ReferenceCount {
PUBLISHED:
  MotionBlur(GraphicsOutput *output);

  void setup();
  void update();
  void shutdown();

  void set_scene_camera(const NodePath &camera);

  const NodePath &get_camera() const;

public:
  void draw(CallbackData *data);

private:
  GraphicsOutput *_output;
  PT(Texture) _framebuffer_texture;
  NodePath _scene_camera;
  CPT(Geom) _quad;
  CPT(RenderState) _quad_state;

  NodePath _np;

  NodePath _camera;

  // Params to shader
  PTA_LVecBase4f _motion_blur_params;
  PTA_LVecBase4f _consts;

  // Previous frame data
  float _last_time_update;
  float _previous_pitch;
  float _previous_yaw;
  LVector3 _previous_position;
  float _no_rotational_motion_blur_until;
};

#endif // MOTIONBLUR_H
