// Filename: windowsRegistry.cxx
// Created by:  drose (06Aug01)
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

#include "windowsRegistry.h"
#include "config_express.h"

#ifdef WIN32_VC

#include <windows.h>

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::set_string_value
//       Access: Published, Static
//  Description: Sets the registry key to the indicated value as a
//               string.  The supplied string value is automatically
//               converted from whatever encoding is set by
//               TextEncoder::set_default_encoding() and written as a
//               Unicode string.  The registry key must already exist
//               prior to calling this function.
////////////////////////////////////////////////////////////////////
bool WindowsRegistry::
set_string_value(const string &key, const string &name, const string &value) {
  TextEncoder encoder;
  wstring wvalue = encoder.decode_text(value);

  return do_set(key, name, REG_SZ, wvalue.data(),
                wvalue.length() * sizeof(wchar_t));
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::set_int_value
//       Access: Published, Static
//  Description: Sets the registry key to the indicated value as an
//               integer.  The registry key must already exist prior
//               to calling this function.
////////////////////////////////////////////////////////////////////
bool WindowsRegistry::
set_int_value(const string &key, const string &name, int value) {
  DWORD dw = value;
  return do_set(key, name, REG_DWORD, &dw, sizeof(dw));
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::get_key_type
//       Access: Published, Static
//  Description: Returns the type of the indicated key, or T_none if
//               the key is not known or is some unsupported type.
////////////////////////////////////////////////////////////////////
WindowsRegistry::Type WindowsRegistry::
get_key_type(const string &key, const string &name) {
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data)) {
    return T_none;
  }

  switch (data_type) {
  case REG_SZ:
    return T_string;

  case REG_DWORD:
    return T_int;

  default:
    return T_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::get_string_value
//       Access: Published, Static
//  Description: Returns the value associated with the indicated
//               registry key, assuming it is a string value.  The
//               string value is automatically encoded using
//               TextEncoder::get_default_encoding().  If the key is
//               not defined or is not a string type value,
//               default_value is returned instead.
////////////////////////////////////////////////////////////////////
string WindowsRegistry::
get_string_value(const string &key, const string &name,
                 const string &default_value) {
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data)) {
    return default_value;
  }

  if (data_type != REG_SZ) {
    express_cat.warning()
      << "Registry key " << key << " does not contain a string value.\n";
    return default_value;
  }

  // Now we have a Unicode string stored in a string buffer.  Lame.
  wstring wdata((wchar_t *)data.data(), (data.length() / sizeof(wchar_t)));

  TextEncoder encoder;
  return encoder.encode_wtext(wdata);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::get_int_value
//       Access: Published, Static
//  Description: Returns the value associated with the indicated
//               registry key, assuming it is an integer value.  If
//               the key is not defined or is not an integer type
//               value, default_value is returned instead.
////////////////////////////////////////////////////////////////////
int WindowsRegistry::
get_int_value(const string &key, const string &name, int default_value) {
  int data_type;
  string data;
  if (!do_get(key, name, data_type, data)) {
    return default_value;
  }

  if (data_type != REG_DWORD) {
    express_cat.warning()
      << "Registry key " << key << " does not contain an integer value.\n";
    return default_value;
  }
  
  // Now we have a DWORD encoded in a string.
  nassertr(data.length() == sizeof(DWORD), default_value);
  DWORD dw = *(DWORD *)data.data();
  return dw;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::do_set
//       Access: Private, Static
//  Description: The internal function to actually make all of the
//               appropriate windows calls to set the registry value.
////////////////////////////////////////////////////////////////////
bool WindowsRegistry::
do_set(const string &key, const string &name,
       int data_type, const void *data, int data_length) {
  HKEY hkey;
  LONG error;

  error =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_SET_VALUE, &hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to open registry key " << key
      << ": " << format_message(error) << "\n";
    return false;
  }

  TextEncoder encoder;
  wstring wname = encoder.decode_text(name);

  bool okflag = true;

  // It's possible that making this direct call to RegSetValueExW()
  // will cause this dll not to load on Win95.  It's difficult to test
  // this hypothesis without having a Win95 machine available.  But
  // since we're not officially supporting Win95 anymore, maybe I
  // shouldn't even care.
  error =
    RegSetValueExW(hkey, wname.c_str(), 0, data_type, 
                   (CONST BYTE *)data, data_length);
  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to set registry key " << key << " name " << name 
      << ": " << format_message(error) << "\n";
    okflag = false;
  }

  error = RegCloseKey(hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.warning()
      << "Unable to close opened registry key " << key
      << ": " << format_message(error) << "\n";
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::do_get
//       Access: Private, Static
//  Description: The internal function to actually make all of the
//               appropriate windows calls to retrieve the registry
//               value.
////////////////////////////////////////////////////////////////////
bool WindowsRegistry::
do_get(const string &key, const string &name, int &data_type, string &data) {
  HKEY hkey;
  LONG error;

  error =
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_QUERY_VALUE, &hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to open registry key " << key
      << ": " << format_message(error) << "\n";
    return false;
  }

  TextEncoder encoder;
  wstring wname = encoder.decode_text(name);

  bool okflag = true;

  // We start with a 1K buffer; presumably that will be big enough
  // most of the time.
  static const size_t init_buffer_size = 1024;
  char init_buffer[init_buffer_size];
  DWORD buffer_size = init_buffer_size;
  DWORD dw_data_type;

  error =
    RegQueryValueExW(hkey, wname.c_str(), 0, &dw_data_type, 
                     (BYTE *)init_buffer, &buffer_size);
  if (error == ERROR_SUCCESS) {
    data_type = dw_data_type;
    if (data_type == REG_SZ || 
        data_type == REG_MULTI_SZ || 
        data_type == REG_EXPAND_SZ) {
      // Eliminate the trailing null character.
      buffer_size--;
    }
    data = string(init_buffer, buffer_size);

  } else if (error == ERROR_MORE_DATA) {
    // Huh, 1K wasn't big enough.  Ok, get a bigger buffer.

    // If we were querying HKEY_PERFORMANCE_DATA, we'd have to keep
    // guessing bigger and bigger until we got it.  Since we're
    // querying static data for now, we can just use the size Windows
    // tells us.
    char *new_buffer = new char[buffer_size];
    error =
      RegQueryValueExW(hkey, wname.c_str(), 0, &dw_data_type, 
                       (BYTE *)new_buffer, &buffer_size);
    if (error == ERROR_SUCCESS) {
      data_type = dw_data_type;
      if (data_type == REG_SZ || 
          data_type == REG_MULTI_SZ || 
          data_type == REG_EXPAND_SZ) {
        // Eliminate the trailing null character.
        buffer_size--;
      }
      data = string(new_buffer, buffer_size);
    }
    delete new_buffer;
  }

  if (error != ERROR_SUCCESS) {
    express_cat.error()
      << "Unable to get registry key " << key << " name " << name 
      << ": " << format_message(error) << "\n";
    okflag = false;
  }

  error = RegCloseKey(hkey);
  if (error != ERROR_SUCCESS) {
    express_cat.warning()
      << "Unable to close opened registry key " << key
      << ": " << format_message(error) << "\n";
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowsRegistry::format_message
//       Access: Private, Static
//  Description: Returns the Windows error message associated with the
//               given error code.
////////////////////////////////////////////////////////////////////
string WindowsRegistry::
format_message(int error_code) {
  PVOID buffer;
  DWORD length = 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, error_code, 0, (LPTSTR)&buffer, 0, NULL);
  if (length == 0) {
    return "Unknown error message";
  }
  string result((const char *)buffer, length);
  LocalFree(buffer);
  return result;
}

#endif  // WIN32_VC
