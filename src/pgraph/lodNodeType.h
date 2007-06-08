// Filename: lodNodeType.h
// Created by:  drose (08Jun07)
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

#ifndef LODNODETYPE_H
#define LODNODETYPE_H

#include "pandabase.h"

BEGIN_PUBLISH

enum LODNodeType {
  LNT_pop,
  LNT_fade,
};

END_PUBLISH

EXPCL_PANDA ostream &operator << (ostream &out, LODNodeType lnt);
EXPCL_PANDA istream &operator >> (istream &in, LODNodeType &cs);

#endif


