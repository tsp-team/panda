/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderStateScript.I
 * @author lachbr
 * @date 2020-10-13
 */

/**
 * Returns the filename extension for a raw text render state script.
 */
INLINE std::string RenderStateScript::
get_extension() {
  return "mat";
}

/**
 * Returns the filename extension for a binary render state script.
 */
INLINE std::string RenderStateScript::
get_binary_extension() {
  return "pmat";
}
