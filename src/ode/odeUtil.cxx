// Filename: odeUtil.cxx
// Created by:  joswilso (27Dec06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "odeUtil.h"

dReal OdeUtil::OC_infinity = dInfinity;

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::get_connecting_joint
//       Access: Public, Static
//  Description: Returns the joint that connects the given bodies.
////////////////////////////////////////////////////////////////////
OdeJoint OdeUtil::
get_connecting_joint(const OdeBody &body1, const OdeBody &body2) {
  return OdeJoint(dConnectingJoint(body1.get_id(),body2.get_id()));
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::get_connecting_joint_list
//       Access: Public, Static
//  Description: Returns a list of joints connecting the bodies.
////////////////////////////////////////////////////////////////////
OdeJointCollection OdeUtil::
get_connecting_joint_list(const OdeBody &body1, const OdeBody &body2) {
  const int max_possible_joints = min(body1.get_num_joints(), body1.get_num_joints());
  
  dJointID *joint_list = (dJointID *)PANDA_MALLOC_ARRAY(max_possible_joints * sizeof(dJointID));
  int num_joints = dConnectingJointList(body1.get_id(), body2.get_id(),
          joint_list);
  OdeJointCollection joints;
  for (int i = 0; i < num_joints; i++) {
    joints.add_joint(OdeJoint(joint_list[i]));
  }
  
  PANDA_FREE_ARRAY(joint_list);
  return joints;
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::are_connected
//       Access: Public, Static
//  Description: Returns 1 if the given bodies are connected
//               by a joint, returns 0 otherwise.
////////////////////////////////////////////////////////////////////
int OdeUtil::
are_connected(const OdeBody &body1, const OdeBody &body2) {
  return dAreConnected(body1.get_id(),body2.get_id());
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::are_connected_excluding
//       Access: Public, Static
//  Description: Returns 1 if the given bodies are connected
//               by a joint that does not match the given
//               joint_type, returns 0 otherwise. This is useful
//               for deciding whether to add contact joints between
//               two bodies: if they are already connected by
//               non-contact joints then it may not be appropriate
//               to add contacts, however it is okay to add more
//               contact between bodies that already have contacts.
////////////////////////////////////////////////////////////////////
int OdeUtil::
are_connected_excluding(const OdeBody &body1,
                        const OdeBody &body2,
                        const int joint_type) {
  return dAreConnectedExcluding(body1.get_id(),
        body2.get_id(),
        joint_type);
}

