/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_anim.cxx
 * @author lachbr
 * @date 2021-02-22
 */

#include "config_anim.h"

ConfigureDef(config_anim);
ConfigureFn(config_anim) {
  init_libanim();
}

NotifyCategoryDef(anim, "");

ConfigVariableBool compress_channels
("compress-channels", false,
 PRC_DESC("Set this true to enable lossy compression of animation channels "
          "when writing to the bam file.  This serves to reduce the size of "
          "the bam file only; it does not reduce the memory footprint of the "
          "channels when the bam file is loaded."));

/*
 * There are some special values above 100 which are generally only useful for
 * debugging (specifically, to research at what point a particular animation
 * artifact is being introduced): 101  Output numerically lossless data.  The
 * output is not run through the FFTCompressor.  This can be used to check
 * whether a particular artifact is due to the FFT conversion or not.
 * However, joint angles (HPR) are still converted to quaternions and
 * normalized, discarding the fourth (redundant) component.  102  As above,
 * but the fourth quaternion component is preserved.  103  Quaternions are not
 * used; instead, the HPR values are written directly.  All output is now
 * completely lossless; if some artifacts are being introduced at this point,
 * check your sanity.
 */
ConfigVariableInt compress_chan_quality
("compress-chan-quality", 95,
 PRC_DESC("The quality level is an integer number that generally ranges "
          "between 0 and 100, where smaller numbers indicate greater "
          "compression at the cost of quality, and larger numbers indicate "
          "higher quality but less compression.  Generally, 95 is the highest "
          "useful value; values between 95 and 100 produce substantially "
          "larger, but not substantially better, output files.  This is akin "
          "to the JPEG compression level."));

ConfigVariableBool read_compressed_channels
("read-compressed-channels", true,
PRC_DESC("Set this false to disable reading of compressed animation channels, "
         "even if the decompression code is available.  The only reason you "
         "might want to do this would be to speed load time when you don't "
         "care about what the animation looks like."));

ConfigVariableBool interpolate_frames
("interpolate-frames", false,
PRC_DESC("Set this true to interpolate character animations between frames, "
         "or false to hold each frame until the next one is ready.  This can "
         "also be changed on a per-character basis with "
         "PartBundle::set_frame_blend_flag()."));

ConfigVariableBool restore_initial_pose
("restore-initial-pose", true,
PRC_DESC("When this is true, setting all control effects on an Actor to 0 "
         "causes it to return to its default, unanimated pose.  When "
         "false, it retains whatever its last-computed pose was "
         "(which may or may not be the default pose)."));

ConfigVariableInt async_bind_priority
("async-bind-priority", 100,
PRC_DESC("This specifies the priority assign to an asynchronous bind "
         "task when it is requested via PartBundle::load_bind_anim().  "
         "This controls the relative order in which asynchronous loads "
         "happen (in particular, relative to asynchronous texture or "
         "model loads).  A higher number here makes the animations "
         "load sooner."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libanim() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  initialized = true;
}