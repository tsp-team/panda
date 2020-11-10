/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_steamnet.h
 * @author lachbr
 * @date 2020-03-29
 */

#ifndef CONFIG_STEAMNET_H
#define CONFIG_STEAMNET_H

#include "dconfig.h"
#include "notifyCategoryProxy.h"

#ifdef BUILDING_PANDA_STEAMNET
#define EXPCL_PANDA_STEAMNET EXPORT_CLASS
#define EXPTP_PANDA_STEAMNET EXPORT_TEMPL
#else
#define EXPCL_PANDA_STEAMNET IMPORT_CLASS
#define EXPTP_PANDA_STEAMNET IMPORT_TEMPL
#endif

ConfigureDecl(config_steamnet, EXPCL_PANDA_STEAMNET, EXPTP_PANDA_STEAMNET);

NotifyCategoryDecl(steamnet, EXPCL_PANDA_STEAMNET, EXPTP_PANDA_STEAMNET);

extern EXPCL_PANDA_STEAMNET void init_libsteamnet();

#endif // CONFIG_STEAMNET_H