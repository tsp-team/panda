// Filename: config_express.cxx
// Created by:  drose (28Mar06)
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

#include "config_express.h"
#include "datagram.h"
#include "nodeReferenceCount.h"
#include "referenceCount.h"
#include "textEncoder.h"
#include "typedObject.h"
#include "typedReferenceCount.h"
#include "virtualFile.h"
#include "virtualFileComposite.h"
#include "virtualFileMount.h"
#include "virtualFileMountMultifile.h"
#include "virtualFileMountSystem.h"
#include "virtualFileSimple.h"
#include "pandaSystem.h"
#include "numeric_types.h"
#include "namable.h"

#include "dconfig.h"

ConfigureDef(config_express);
NotifyCategoryDef(express, "");
NotifyCategoryDef(clock, ":express");

ConfigureFn(config_express) {
  init_libexpress();
}

ConfigVariableInt patchfile_window_size
("patchfile-window-size", 16);

ConfigVariableInt patchfile_increment_size
("patchfile-increment-size", 8);

ConfigVariableInt patchfile_buffer_size
("patchfile-buffer-size", 4096);

ConfigVariableInt patchfile_zone_size
("patchfile-zone-size", 10000);

ConfigVariableBool keep_temporary_files
("keep-temporary-files", false,
 PRC_DESC("Set this true to keep around the temporary files from "
          "downloading, decompressing, and patching, or false (the "
          "default) to delete these.  Mainly useful for debugging "
          "when the process goes wrong."));

ConfigVariableBool lock_to_one_cpu
("lock-to-one-cpu", false,
 PRC_DESC("Set this to true if you want the entire process to use one "
          "CPU, even on multi-core and multi-CPU workstations. This is "
          "mainly a hack to solve a bug in which QueryPerformanceCounter "
          "returns inconsistent results on multi-core machines. "));

ConfigVariableInt multifile_encryption_iteration_count
("multifile-encryption-iteration-count", 0,
 PRC_DESC("This is a special value of encryption-iteration-count used to encrypt "
          "subfiles within a multifile.  It has a default value of 0 (just one "
          "application), on the assumption that the files from a multifile must "
          "be loaded quickly, without paying the cost of an expensive hash on "
          "each subfile in order to decrypt it."));

ConfigVariableBool vfs_case_sensitive
("vfs-case-sensitive", true,
 PRC_DESC("Set this true to make the VirtualFileSystem present the native "
          "OS-provided filesystem as if it were a case-sensitive file "
          "system, even if it is not (e.g. on Windows).  This variable "
          "has no effect if the native filesystem is already case-sensitive, "
          "and it has no effect on mounted multifile systems, which are "
          "always case-sensitive."));

ConfigVariableBool vfs_implicit_pz
("vfs-implicit-pz", true,
 PRC_DESC("When this is true, the VirtualFileSystem will pretend a named "
          "file exists even if it doesn't, as long as a filename with the "
          "same name and the additional extension .pz does exist.  In this "
          "case, the VirtualFileSystem will implicitly open the .pz file "
          "and decompress it on-the-fly."));

ConfigVariableBool use_vfs
("use-vfs", true,
 PRC_DESC("Set this true to use the VirtualFileSystem mechanism for loading "
          "models, etc.  Since the VirtualFileSystem is now the de facto "
          "filesystem for Panda, you should always keep this true, since "
          "there is now code that assumes it to be true.  This variable "
          "is now deprecated."));

ConfigVariableBool collect_tcp
("collect-tcp", false,
 PRC_DESC("Set this true to enable accumulation of several small consecutive "
          "TCP datagrams into one large datagram before sending it, to reduce "
          "overhead from the TCP/IP protocol.  See "
          "Connection::set_collect_tcp() or SocketStream::set_collect_tcp()."));

ConfigVariableDouble collect_tcp_interval
("collect-tcp-interval", 0.2);

////////////////////////////////////////////////////////////////////
//     Function: init_libexpress
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libexpress() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  Datagram::init_type();
  Namable::init_type();
  NodeReferenceCount::init_type();
  ReferenceCount::init_type();
  TextEncoder::init_type();
  TypedObject::init_type();
  TypedReferenceCount::init_type();
  VirtualFile::init_type();
  VirtualFileComposite::init_type();
  VirtualFileMount::init_type();
  VirtualFileMountMultifile::init_type();
  VirtualFileMountSystem::init_type();
  VirtualFileSimple::init_type();

  init_system_type_handles();

#ifdef HAVE_ZLIB
  {
    PandaSystem *ps = PandaSystem::get_global_ptr();
    ps->add_system("zlib");
  }
#endif

  // This is a fine place to ensure that the numeric types have been
  // chosen correctly.
  nassertv(sizeof(PN_int8) == 1 && sizeof(PN_uint8) == 1);
  nassertv(sizeof(PN_int16) == 2 && sizeof(PN_uint16) == 2);
  nassertv(sizeof(PN_int32) == 4 && sizeof(PN_uint32) == 4);
  nassertv(sizeof(PN_int64) == 8 && sizeof(PN_uint64) == 8);
  nassertv(sizeof(PN_float32) == 4);
  nassertv(sizeof(PN_float64) == 8);

  // Also, ensure that we have the right endianness.
  PN_uint32 word;
  memcpy(&word, "\1\2\3\4", 4);
#ifdef WORDS_BIGENDIAN
  nassertv(word == 0x01020304);
#else
  nassertv(word == 0x04030201);
#endif
}

bool
get_use_high_res_clock() {
  static ConfigVariableBool *use_high_res_clock = NULL;

  if (use_high_res_clock == (ConfigVariableBool *)NULL) {
    use_high_res_clock = new ConfigVariableBool
      ("use-high-res-clock", true,
       PRC_DESC("Set this to false to avoid using the high-precision clock, even if "
                "it is available."));
  }

  return *use_high_res_clock;
}

bool
get_paranoid_clock() {
  static ConfigVariableBool *paranoid_clock = NULL;

  if (paranoid_clock == (ConfigVariableBool *)NULL) {
    paranoid_clock = new ConfigVariableBool
      ("paranoid-clock", false,
       PRC_DESC("Set this to true to double-check the results of the high-resolution "
                "clock against the system clock.  This has no effect if NDEBUG is "
                "defined."));
  }

  return *paranoid_clock;
}

bool
get_verify_dcast() {
  static ConfigVariableBool *verify_dcast = NULL;

  if (verify_dcast == (ConfigVariableBool *)NULL) {
    verify_dcast = new ConfigVariableBool
      ("verify-dcast", true,
       PRC_DESC("Set this to true to verify that every attempted DCAST operation in "
                "fact references the correct type, or false otherwise.  This has no "
                "effect if NDEBUG is defined, in which case it is never tested."));
  }

  return *verify_dcast;
}

// Returns the configure object for accessing config variables from a
// scripting language.
ConfigExpress &
get_config_express() {
  return config_express;
}
