// Filename: eggGroup.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggGroup.h"
#include "eggMiscFuncs.h"
#include "eggVertexPool.h"

#include <indent.h>
#include <string_utils.h>
#include <lmatrix.h>

extern int eggyyparse(void);
#include "parserDefs.h"
#include "lexerDefs.h"

TypeHandle EggGroup::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggGroup::
EggGroup(const string &name) : EggGroupNode(name) {
  _flags = 0;
  _flags2 = 0;
  _transform = LMatrix4d::ident_mat();
  _fps = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggGroup::
EggGroup(const EggGroup &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggGroup &EggGroup::
operator = (const EggGroup &copy) {
  _flags = copy._flags;
  _flags2 = copy._flags2;
  _transform = copy._transform;
  _objecttype = copy._objecttype;
  _collision_name = copy._collision_name;
  _fps = copy._fps;

  unref_all_vertices();
  _vref = copy._vref;

  // We must walk through the vertex ref list, and flag each vertex as
  // now reffed by this group.
  VertexRef::iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;

    bool inserted = vert->_gref.insert(this).second;
    // Did the group not exist previously in the vertex's gref list?
    // If it was there already, we must be out of sync between
    // vertices and groups.
    nassertr(inserted, *this);
  }

  // These must be down here, because the EggNode assignment operator
  // will force an update_under().  Therefore, we can't call it until
  // all the attributes that affect adjust_under() are in place.
  EggGroupNode::operator = (copy);
  EggAlphaMode::operator = (copy);

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroup::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggGroup::
~EggGroup() {
  unref_all_vertices();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_group_type
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggGroup::
set_group_type(GroupType type) {
  if (type != get_group_type()) {
    // Make sure the user didn't give us any stray bits.
    nassertv((type & ~F_group_type)==0);
    _flags = (_flags & ~F_group_type) | type;

    // Now we might have changed the type to or from an instance node,
    // so we have to recompute the under_flags.
    update_under(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::set_transform
//       Access: Public
//  Description: Sets the indicated transformation matrix on the node.
////////////////////////////////////////////////////////////////////
void EggGroup::
set_transform(const LMatrix4d &transform) {
  _transform = transform;
  if (!has_transform()) {
    _flags |= F_has_transform;

    // Now we have to update the under_flags.
    update_under(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::clear_transform
//       Access: Public
//  Description: Removes any transformation matrix from the node.
////////////////////////////////////////////////////////////////////
void EggGroup::
clear_transform() {
  _transform = LMatrix4d::ident_mat();
  if (has_transform()) {
    _flags &= ~F_has_transform;

    // Now we have to update the under_flags.
    update_under(0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::write
//       Access: Public, Virtual
//  Description: Writes the group and all of its children to the
//               indicated output stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggGroup::
write(ostream &out, int indent_level) const {
  test_under_integrity();

  switch (get_group_type()) {
  case GT_group:
    write_header(out, indent_level, "<Group>");
    break;

  case GT_instance:
    write_header(out, indent_level, "<Instance>");
    break;

  case GT_joint:
    write_header(out, indent_level, "<Joint>");
    break;

  default:
    // invalid group type
    nassertv(false);
  }

  if (has_lod()) {
    get_lod().write(out, indent_level + 2);
  }

  if (get_billboard_type() != BT_none) {
    indent(out, indent_level + 2) 
      << "<Billboard> { " << get_billboard_type() << " }\n";
  }

  if (get_cs_type() != CST_none) {
    indent(out, indent_level + 2) << "<Collide> ";
    if (has_collision_name()) {
      enquote_string(out, get_collision_name()) << " ";
    }
    out << "{ " << get_cs_type();
    if (get_collide_flags() != CF_none) {
      out << " " << get_collide_flags();
    }
    out << " }\n";
  }

  if (has_collide_mask()) {
    indent(out, indent_level + 2) 
      << "<Scalar> collide-mask { 0x";
    get_collide_mask().output_hex(out, 0);
    out << " }\n";
  }

  if (has_from_collide_mask()) {
    indent(out, indent_level + 2) 
      << "<Scalar> from-collide-mask { 0x";
    get_from_collide_mask().output_hex(out, 0);
    out << " }\n";
  }

  if (has_into_collide_mask()) {
    indent(out, indent_level + 2) 
      << "<Scalar> into-collide-mask { 0x";
    get_into_collide_mask().output_hex(out, 0);
    out << " }\n";
  }

  if (get_dcs_flag()) {
    indent(out, indent_level + 2) << "<DCS> { 1 }\n";
  }

  if (get_dart_type() != DT_none) {
    indent(out, indent_level + 2)
      << "<Dart> { " << get_dart_type() << " }\n";
  }

  if (get_switch_flag()) {
    indent(out, indent_level + 2) << "<Switch> { 1 }\n";
    if (get_switch_fps() != 0.0) {
      indent(out, indent_level + 2)
	<< "<Scalar> fps { " << get_switch_fps() << " }\n";
    }
  }

  if (has_transform()) {
    write_transform(out, _transform, indent_level + 2);
  }

  if (has_objecttype()) {
    indent(out, indent_level + 2) 
      << "<ObjectType> { ";
    enquote_string(out, get_objecttype()) << " }\n";
  }

  if (get_model_flag()) {
    indent(out, indent_level + 2) << "<Model> { 1 }\n";
  }

  if (get_texlist_flag()) {
    indent(out, indent_level + 2) << "<TexList> { 1 }\n";
  }

  if (get_nofog_flag()) {
    indent(out, indent_level + 2) << "<Scalar> no-fog { 1 }\n";
  }

  if (get_decal_flag()) {
    indent(out, indent_level + 2) << "<Scalar> decal { 1 }\n";
  }

  if (get_direct_flag()) {
    indent(out, indent_level + 2) << "<Scalar> direct { 1 }\n";
  }

  EggAlphaMode::write(out, indent_level + 2);

  write_vertex_ref(out, indent_level + 2);

  EggGroupNode::write(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::determine_alpha_mode
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this group that has an alpha_mode other than
//               AM_unspecified.  Returns a valid EggAlphaMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggAlphaMode *EggGroup::
determine_alpha_mode() {
  if (get_alpha_mode() != AM_unspecified) {
    return this;
  }
  return EggGroupNode::determine_alpha_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::determine_draw_order
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this group that has a draw_order specified.
//               Returns a valid EggAlphaMode pointer if one is found,
//               or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggAlphaMode *EggGroup::
determine_draw_order() {
  if (has_draw_order()) {
    return this;
  }
  return EggGroupNode::determine_draw_order();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::determine_bin
//       Access: Public, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this group that has a bin specified.  Returns a
//               valid EggAlphaMode pointer if one is found, or NULL
//               otherwise.
////////////////////////////////////////////////////////////////////
EggAlphaMode *EggGroup::
determine_bin() {
  if (has_bin()) {
    return this;
  }
  return EggGroupNode::determine_bin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::parse_egg
//       Access: Public
//  Description: Parses the egg syntax given in the indicate string as
//               if it had been read from the egg file within the
//               <Group> { ... } definition.  Updates the EggGroup
//               accordingly.  Returns true if successful, false if
//               there was some parse error.
////////////////////////////////////////////////////////////////////
bool EggGroup::
parse_egg(const string &egg_syntax) {
  istringstream in(egg_syntax);
  egg_init_parser(in, "", this, this);
  egg_start_group_body();
  eggyyparse();
  egg_cleanup_parser();

  return (egg_error_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::ref_vertex
//       Access: Public
//  Description: Adds the vertex to the set of those referenced by the
//               group, at the indicated membership level.  If the
//               vertex is already being referenced, increases the
//               membership amount by the indicated amount.
////////////////////////////////////////////////////////////////////
void EggGroup::
ref_vertex(EggVertex *vert, double membership) {
  VertexRef::iterator vri = _vref.find(vert);

  if (vri != _vref.end()) {
    // The vertex was already being reffed; increment its membership
    // amount.
    (*vri).second += membership;

    // If that takes us down to zero, go ahead and unref the vertex.
    if ((*vri).second == 0.0) {
      unref_vertex(vert);
    }

  } else {
    // The vertex was not already reffed; ref it.
    if (membership != 0.0) {
      _vref[vert] = membership;

      bool inserted = vert->_gref.insert(this).second;
      // Did the group not exist previously in the vertex's gref list?
      // If it was there already, we must be out of sync between
      // vertices and groups.
      nassertv(inserted);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroup::unref_vertex
//       Access: Public
//  Description: Removes the vertex from the set of those referenced
//               by the group.  Does nothing if the vertex is not
//               already reffed.
////////////////////////////////////////////////////////////////////
void EggGroup::
unref_vertex(EggVertex *vert) {
  VertexRef::iterator vri = _vref.find(vert);

  if (vri != _vref.end()) {
    _vref.erase(vri);
    int count = vert->_gref.erase(this);
    // Did the group exist in the vertex's gref list?  If it didn't,
    // we must be out of sync between vertices and groups.
    nassertv(count == 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::unref_all_vertices
//       Access: Public
//  Description: Removes all vertices from the reference list.
////////////////////////////////////////////////////////////////////
void EggGroup::
unref_all_vertices() {
  // We must walk through the vertex ref list, and flag each vertex as
  // unreffed in its own structure.
  VertexRef::iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;
    int count = vert->_gref.erase(this);
    // Did the group exist in the vertex's gref list?  If it didn't,
    // we must be out of sync between vertices and groups.
    nassertv(count == 1);
  }

  _vref.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroup::get_vertex_membership
//       Access: Public
//  Description: Returns the amount of membership of the indicated
//               vertex in this group.  If the vertex is not reffed by
//               the group, returns 0.
////////////////////////////////////////////////////////////////////
double EggGroup::
get_vertex_membership(const EggVertex *vert) const {
  VertexRef::const_iterator vri = _vref.find((EggVertex *)vert);

  if (vri != _vref.end()) {
    return (*vri).second;
  } else {
    return 0.0;
  }
}


#ifndef NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::test_vref_integrity
//       Access: Public
//  Description: Verifies that each vertex in the group exists and
//               that it knows it is referenced by the group.
////////////////////////////////////////////////////////////////////
void EggGroup::
test_vref_integrity() const {
  test_ref_count_integrity();

  VertexRef::const_iterator vri;
  for (vri = vref_begin(); vri != vref_end(); ++vri) {
    const EggVertex *vert = (*vri).first;
    vert->test_ref_count_integrity();

    nassertv(vert->has_gref(this));
  }
}

#endif  // NDEBUG




////////////////////////////////////////////////////////////////////
//     Function: EggGroup::string_group_type
//       Access: Public, Static
//  Description: Returns the GroupType value associated with the given
//               string representation, or GT_invalid if the string
//               does not match any known GroupType value.
////////////////////////////////////////////////////////////////////
EggGroup::GroupType EggGroup::
string_group_type(const string &string) {
  if (cmp_nocase_uh(string, "group") == 0) {
    return GT_group;
  } else if (cmp_nocase_uh(string, "instance") == 0) {
    return GT_instance;
  } else if (cmp_nocase_uh(string, "joint") == 0) {
    return GT_joint;
  } else {
    return GT_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::string_dart_type
//       Access: Public, Static
//  Description: Returns the DartType value associated with the given
//               string representation, or DT_none if the string
//               does not match any known DartType value.
////////////////////////////////////////////////////////////////////
EggGroup::DartType EggGroup::
string_dart_type(const string &string) {
  if (cmp_nocase_uh(string, "sync") == 0) {
    return DT_sync;
  } else if (cmp_nocase_uh(string, "nosync") == 0) {
    return DT_nosync;
  } else if (cmp_nocase_uh(string, "default") == 0) {
    return DT_default;
  } else {
    return DT_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::string_billboard_type
//       Access: Public, Static
//  Description: Returns the BillboardType value associated with the
//               given string representation, or BT_none if the string
//               does not match any known BillboardType value.
////////////////////////////////////////////////////////////////////
EggGroup::BillboardType EggGroup::
string_billboard_type(const string &string) {
  if (cmp_nocase_uh(string, "axis") == 0) {
    return BT_axis;
  } else if (cmp_nocase_uh(string, "point_eye") == 0) {
    return BT_point_camera_relative;
  } else if (cmp_nocase_uh(string, "point_world") == 0) {
    return BT_point_world_relative;
  } else if (cmp_nocase_uh(string, "point") == 0) {
    return BT_point_world_relative;
  } else {
    return BT_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::string_cs_type
//       Access: Public, Static
//  Description: Returns the CollisionSolidType value associated with the
//               given string representation, or CST_none if the string
//               does not match any known CollisionSolidType value.
////////////////////////////////////////////////////////////////////
EggGroup::CollisionSolidType EggGroup::
string_cs_type(const string &string) {
  if (cmp_nocase_uh(string, "plane") == 0) {
    return CST_plane;
  } else if (cmp_nocase_uh(string, "polygon") == 0) {
    return CST_polygon;
  } else if (cmp_nocase_uh(string, "polyset") == 0) {
    return CST_polyset;
  } else if (cmp_nocase_uh(string, "sphere") == 0) {
    return CST_sphere;
  } else if (cmp_nocase_uh(string, "inversesphere") == 0) {
    return CST_inverse_sphere;
  } else if (cmp_nocase_uh(string, "geode") == 0) {
    return CST_geode;
  } else {
    return CST_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::string_collide_flags
//       Access: Public, Static
//  Description: Returns the CollideFlags value associated with the
//               given string representation, or CF_none if the string
//               does not match any known CollideFlags value.  This
//               only recognizes a single keyword; it does not attempt
//               to parse a string of keywords.
////////////////////////////////////////////////////////////////////
EggGroup::CollideFlags EggGroup::
string_collide_flags(const string &string) {
  if (cmp_nocase_uh(string, "intangible") == 0) {
    return CF_intangible;
  } else if (cmp_nocase_uh(string, "event") == 0) {
    return CF_event;
  } else if (cmp_nocase_uh(string, "descend") == 0) {
    return CF_descend;
  } else if (cmp_nocase_uh(string, "keep") == 0) {
    return CF_keep;
  } else if (cmp_nocase_uh(string, "solid") == 0) {
    return CF_solid;
  } else if (cmp_nocase_uh(string, "center") == 0) {
    return CF_center;
  } else if (cmp_nocase_uh(string, "turnstile") == 0) {
    return CF_turnstile;
  } else {
    return CF_none;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: EggGroup::write_vertex_ref
//       Access: Protected
//  Description: Writes out the vertex ref component of the group body
//               only.  This may consist of a number of <VertexRef>
//               entries, each with its own membership value.
////////////////////////////////////////////////////////////////////
void EggGroup::
write_vertex_ref(ostream &out, int indent_level) const {
  // We want to put the vertices together into groups first by vertex
  // pool, then by membership value.  Each of these groups becomes a
  // separate VertexRef entry.  Within each group, we'll sort the
  // vertices by index number.

  typedef set<int> Indices;
  typedef map<double, Indices> Memberships;
  typedef map<EggVertexPool *, Memberships> Pools;

  Pools _entries;
  bool all_membership_one = true;

  VertexRef::const_iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;
    double membership = (*vri).second;

    if (membership != 1.0) {
      all_membership_one = false;
    }

    _entries[vert->get_pool()][membership].insert(vert->get_index());
  }

  // Now that we've reordered them, we can simply traverse the entries
  // and write them out.
  Pools::const_iterator pi;
  for (pi = _entries.begin(); pi != _entries.end(); ++pi) {
    EggVertexPool *pool = (*pi).first;
    const Memberships &memberships = (*pi).second;
    Memberships::const_iterator mi;
    for (mi = memberships.begin(); mi != memberships.end(); ++mi) {
      double membership = (*mi).first;
      const Indices &indices = (*mi).second;

      indent(out, indent_level) 
	<< "<VertexRef> {\n";
      write_long_list(out, indent_level+2, indices.begin(), indices.end(),
		"", "", 72);
      
      // If all vrefs in this group have membership of 1, don't bother
      // to write out the membership scalar.
      if (!all_membership_one) {
	indent(out, indent_level + 2) 
	  << "<Scalar> membership { " << membership << " }\n";
      }
      indent(out, indent_level + 2)
	<< "<Ref> { " << pool->get_name() << " }\n";
      indent(out, indent_level)
	<< "}\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::adjust_under
//       Access: Protected, Virtual
//  Description: This is called within update_under() after all the
//               various under settings have been inherited directly
//               from the parent node.  It is responsible for
//               adjusting these settings to reflect states local to
//               the current node; for instance, an <Instance> node
//               will force the UF_under_instance bit on.
////////////////////////////////////////////////////////////////////
void EggGroup::
adjust_under() {
  // Billboards are an implicit instance.
  if (get_group_type() == GT_instance ||
      get_billboard_type() != BT_none) {
    _under_flags |= UF_under_instance;
    if (_under_flags & UF_under_transform) {
      // If we've reached an instance node and we're under a
      // transform, that means we've just defined a local coordinate
      // system.
      _under_flags |= UF_local_coord;
    }

    // An instance node means that from this point and below, vertices
    // are defined relative to this node.  Thus, the node frame
    // becomes the vertex frame.
    _vertex_frame = _node_frame;
    _vertex_frame_inv = _node_frame_inv;
    _vertex_to_node = NULL;
  }

  // If we have our own transform, it carries forward, but it doesn't
  // have any effect on the local_coord flag, above.
  if (has_transform()) {
    _under_flags |= UF_under_transform;

    // Our own transform also affects our node frame.
    _node_frame = 
      new MatrixFrame(get_transform() * get_node_frame());
    _node_frame_inv =
      new MatrixFrame(invert(get_node_frame()));
    _vertex_to_node =
      new MatrixFrame(get_vertex_frame() * get_node_frame_inv());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroup::r_transform
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform().  It applies a transformation matrix
//               to the current node in some sensible way, then
//               continues down the tree.
//
//               The first matrix is the transformation to apply; the
//               second is its inverse.  The third parameter is the
//               coordinate system we are changing to, or CS_default
//               if we are not changing coordinate systems.
////////////////////////////////////////////////////////////////////
void EggGroup::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
	    CoordinateSystem to_cs) {
  if (has_transform()) {
    // Since we want to apply this transform to all matrices,
    // including nested matrices, we can't simply premult it in and
    // leave it, because that would leave the rotational component in
    // the scene graph's matrix, and all nested matrices would inherit
    // the same rotational component.  So we have to premult and then
    // postmult by the inverse to undo the rotational component each
    // time.

    LMatrix4d mat1 = mat;
    LMatrix4d inv1 = inv;
    
    // If we have a translation component, we should only apply
    // it to the top matrix.  All subsequent matrices get just the
    // rotational component.
    mat1.set_row(3, LVector3d(0.0, 0.0, 0.0));
    inv1.set_row(3, LVector3d(0.0, 0.0, 0.0));

    _transform = inv1 * _transform * mat;

    EggGroupNode::r_transform(mat1, inv1, to_cs);
  } else {
    EggGroupNode::r_transform(mat, inv, to_cs);
  }

  // Convert the LOD description too.
  if (has_lod()) {
    _lod->transform(mat);
  }
}




////////////////////////////////////////////////////////////////////
//     Function: GroupType output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggGroup::GroupType t) {
  switch (t) {
  case EggGroup::GT_invalid:
    return out << "invalid group";
  case EggGroup::GT_group:
    return out << "group";
  case EggGroup::GT_instance:
    return out << "instance";
  case EggGroup::GT_joint:
    return out << "joint";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: DartType output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggGroup::DartType t) {
  switch (t) {
  case EggGroup::DT_none:
    return out << "none";
  case EggGroup::DT_sync:
    return out << "sync";
  case EggGroup::DT_nosync:
    return out << "nosync";
  case EggGroup::DT_default:
    return out << "1";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardType output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggGroup::BillboardType t) {
  switch (t) {
  case EggGroup::BT_none:
    return out << "none";
  case EggGroup::BT_axis:
    return out << "axis";
  case EggGroup::BT_point_camera_relative:
    return out << "point_eye";
  case EggGroup::BT_point_world_relative:
    return out << "point_world";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolidType output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggGroup::CollisionSolidType t) {
  switch (t) {
  case EggGroup::CST_none:
    return out << "None";
  case EggGroup::CST_plane:
    return out << "Plane";
  case EggGroup::CST_polygon:
    return out << "Polygon";
  case EggGroup::CST_polyset:
    return out << "Polyset";
  case EggGroup::CST_sphere:
    return out << "Sphere";
  case EggGroup::CST_inverse_sphere:
    return out << "InverseSphere";
  case EggGroup::CST_geode:
    return out << "Geode";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: CollideFlags output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggGroup::CollideFlags t) {
  if (t == EggGroup::CF_none) {
    return out << "none";
  }
  int bits = (int)t;
  const char *space = "";

  if (bits & EggGroup::CF_intangible) {
    out << space << "intangible";
    space = " ";
  }
  if (bits & EggGroup::CF_event) {
    out << space << "event";
    space = " ";
  }
  if (bits & EggGroup::CF_descend) {
    out << space << "descend";
    space = " ";
  }
  if (bits & EggGroup::CF_keep) {
    out << space << "keep";
    space = " ";
  }
  if (bits & EggGroup::CF_solid) {
    out << space << "solid";
    space = " ";
  }
  if (bits & EggGroup::CF_center) {
    out << space << "center";
    space = " ";
  }
  if (bits & EggGroup::CF_turnstile) {
    out << space << "turnstile";
    space = " ";
  }
  return out;
}
