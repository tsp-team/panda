// Filename: config_wgldisplay.cxx
// Created by:  drose (20Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#include "wglGraphicsWindow.h"
#include "graphicsPipeSelection.h"
#include "dconfig.h"

Configure(config_wgldisplay);
NotifyCategoryDef(wgldisplay, "windisplay");

ConfigureFn(config_wgldisplay) {
  init_libwgldisplay();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libwgldisplay
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libwgldisplay() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  wglGraphicsPipe::init_type();
  wglGraphicsWindow::init_type();

  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->add_pipe_type(wglGraphicsPipe::get_class_type(),
                           wglGraphicsPipe::pipe_constructor);
}

int gl_force_pixfmt = config_wgldisplay.GetInt("gl-force-pixfmt", 0);

