/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file linuxJoystickDevice.h
 * @author rdb
 * @date 2015-08-21
 */

#ifndef LINUXJOYSTICKDEVICE_H
#define LINUXJOYSTICKDEVICE_H

#include "inputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

/**
 * This is a type of device that uses the Linux /dev/input/js# API to read
 * data from a game controller.
 */
class EXPCL_PANDA_DEVICE LinuxJoystickDevice : public InputDevice {
public:
  LinuxJoystickDevice(int index);
  virtual ~LinuxJoystickDevice();

  bool check_events() const;

private:
  virtual void do_poll();

  bool open_device();
  bool process_events();

private:
  int _fd;
  int _index;

  // These are used for D-pad emulation.
  int _dpad_x_axis;
  int _dpad_y_axis;
  int _dpad_left_button;
  int _dpad_up_button;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    InputDevice::init_type();
    register_type(_type_handle, "LinuxJoystickDevice",
                  InputDevice::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "linuxJoystickDevice.I"

#endif  // PHAVE_LINUX_INPUT_H

#endif  // LINUXJOYSTICKDEVICE_H
