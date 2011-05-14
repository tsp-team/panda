// Filename: bulletWorld.h
// Created by:  enn0x (23Jan10)
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

#ifndef __BULLET_WORLD_H__
#define __BULLET_WORLD_H__

#include "pandabase.h"

#include "bullet_includes.h"
#include "bullet_utils.h"

#include "bulletClosestHitRayResult.h"
#include "bulletAllHitsRayResult.h"
#include "bulletClosestHitSweepResult.h"
#include "bulletContactResult.h"
#include "bulletDebugNode.h"
#include "bulletCharacterControllerNode.h"
#include "bulletConstraint.h"
#include "bulletGhostNode.h"
#include "bulletRigidBodyNode.h"
#include "bulletSoftBodyNode.h"
#include "bulletVehicle.h"

#include "typedReferenceCount.h"
#include "transformState.h"
#include "pandaNode.h"
#include "collideMask.h"
#include "luse.h"

class BulletPersistentManifold;
class BulletShape;
class BulletSoftBodyWorldInfo;

////////////////////////////////////////////////////////////////////
//       Class : BulletWorld
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletWorld : public TypedReferenceCount {

PUBLISHED:
  BulletWorld();
  INLINE ~BulletWorld();

  void set_gravity(const LVector3f &gravity);
  void set_gravity(float gx, float gy, float gz);
  const LVector3f get_gravity() const;

  void do_physics(float dt, int substeps=10, float stepsize=1.0f/60.0f);

  void set_debug_node(BulletDebugNode *node);
  void clear_debug_node();

  BulletSoftBodyWorldInfo get_world_info();

  // Ghost object
  void attach_ghost(BulletGhostNode *node);
  void remove_ghost(BulletGhostNode *node);

  INLINE int get_num_ghosts() const;
  INLINE BulletGhostNode *get_ghost(int idx) const;
  MAKE_SEQ(get_ghosts, get_num_ghosts, get_ghost);

  // Rigid body
  void attach_rigid_body(BulletRigidBodyNode *node);
  void remove_rigid_body(BulletRigidBodyNode *node);

  INLINE int get_num_rigid_bodies() const;
  INLINE BulletRigidBodyNode *get_rigid_body(int idx) const;
  MAKE_SEQ(get_rigid_bodies, get_num_rigid_bodies, get_rigid_body);

  // Soft body
  void attach_soft_body(BulletSoftBodyNode *node);
  void remove_soft_body(BulletSoftBodyNode *node);

  INLINE int get_num_soft_bodies() const;
  INLINE BulletSoftBodyNode *get_soft_body(int idx) const;
  MAKE_SEQ(get_soft_bodies, get_num_soft_bodies, get_soft_body);

  // Character controller
  void attach_character(BulletCharacterControllerNode *node);
  void remove_character(BulletCharacterControllerNode *node);

  INLINE int get_num_characters() const;
  INLINE BulletCharacterControllerNode *get_character(int idx) const;
  MAKE_SEQ(get_characters, get_num_characters, get_character);

  // Vehicle
  void attach_vehicle(BulletVehicle *vehicle);
  void remove_vehicle(BulletVehicle *vehicle);

  INLINE int get_num_vehicles() const;
  INLINE BulletVehicle *get_vehicle(int idx) const;
  MAKE_SEQ(get_vehicles, get_num_vehicles, get_vehicle);

  // Constraint
  void attach_constraint(BulletConstraint *constraint);
  void remove_constraint(BulletConstraint *constraint);

  INLINE int get_num_constraints() const;
  INLINE BulletConstraint *get_constraint(int idx) const;
  MAKE_SEQ(get_constraints, get_num_constraints, get_constraint);

  // Raycast and other queries
  BulletClosestHitRayResult ray_test_closest(
    const LPoint3f &from_pos,
    const LPoint3f &to_pos,
    const CollideMask &mask=CollideMask::all_on()) const;

  BulletAllHitsRayResult ray_test_all(
    const LPoint3f &from_pos,
    const LPoint3f &to_pos,
    const CollideMask &mask=CollideMask::all_on()) const;

  BulletClosestHitSweepResult sweep_test_closest(
    BulletShape *shape,
    const TransformState &from_ts,
    const TransformState &to_ts,
    const CollideMask &mask=CollideMask::all_on(),
    float penetration=0.0f) const;

  BulletContactResult contact_test(PandaNode *node) const;
  BulletContactResult contact_test_pair(PandaNode *node0, PandaNode *node1) const;

  // Manifolds
  INLINE int get_num_manifolds() const;
  BulletPersistentManifold *get_manifold(int idx) const;
  MAKE_SEQ(get_manifolds, get_num_manifolds, get_manifold);

  // Configuration
  enum BroadphaseAlgorithm {
    BA_sweep_and_prune,
    BA_dynamic_aabb_tree,
  };

public:
  static btCollisionObject *get_collision_object(PandaNode *node);

  INLINE btDynamicsWorld *get_world() const;
  INLINE btBroadphaseInterface *get_broadphase() const;
  INLINE btDispatcher *get_dispatcher() const;

private:
  typedef PTA(PT(BulletRigidBodyNode)) BulletRigidBodies;
  typedef PTA(PT(BulletSoftBodyNode)) BulletSoftBodies;
  typedef PTA(PT(BulletGhostNode)) BulletGhosts;
  typedef PTA(PT(BulletCharacterControllerNode)) BulletCharacterControllers;
  typedef PTA(PT(BulletVehicle)) BulletVehicles;
  typedef PTA(PT(BulletConstraint)) BulletConstraints;

  static PStatCollector _pstat_physics;
  static PStatCollector _pstat_simulation;
  static PStatCollector _pstat_debug;
  static PStatCollector _pstat_sb;

  struct btFilterCallback : public btOverlapFilterCallback {
    virtual bool needBroadphaseCollision(
      btBroadphaseProxy* proxy0,
      btBroadphaseProxy* proxy1) const;
  };

  btBroadphaseInterface *_broadphase;
  btCollisionConfiguration *_configuration;
  btCollisionDispatcher *_dispatcher;
  btConstraintSolver *_solver;
  btSoftRigidDynamicsWorld *_world;

  btGhostPairCallback *_ghost_cb;
  btFilterCallback *_filter_cb;

  btSoftBodyWorldInfo _info;

  PT(BulletDebugNode) _debug;

  BulletRigidBodies _bodies;
  BulletSoftBodies _softbodies;
  BulletGhosts _ghosts;
  BulletCharacterControllers _characters;
  BulletVehicles _vehicles;
  BulletConstraints _constraints;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BulletWorld", 
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDABULLET ostream &
operator << (ostream &out, BulletWorld::BroadphaseAlgorithm algorithm);
EXPCL_PANDABULLET istream &
operator >> (istream &in, BulletWorld::BroadphaseAlgorithm &algorithm);

#include "bulletWorld.I"

#endif // __BULLET_WORLD_H__
