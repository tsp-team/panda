// Filename: plane_src.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Plane
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(Plane) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(Plane)(void);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(Plane) &copy);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(LPoint3) &a, const FLOATNAME(LPoint3) &b,
			  const FLOATNAME(LPoint3) &c);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(LVector3) &normal, 
			  const FLOATNAME(LPoint3) &point);

  INLINE_MATHUTIL FLOATNAME(Plane)& operator = (const FLOATNAME(Plane)& copy);

  INLINE_MATHUTIL FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix3) &mat) const;
  INLINE_MATHUTIL FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix4) &mat) const;
 
  FLOATNAME(LMatrix4) get_reflection_mat(void) const;

  INLINE_MATHUTIL FLOATNAME(LVector3) get_normal() const;
  FLOATNAME(LPoint3) get_point() const;

  INLINE_MATHUTIL FLOATTYPE dist_to_plane(const FLOATNAME(LPoint3) &point) const; 
  INLINE_MATHUTIL bool intersects_line(FLOATNAME(LPoint3) &intersection_point,
			      const FLOATNAME(LPoint3) &p1,
			      const FLOATNAME(LPoint3) &p2) const;
  INLINE_MATHUTIL bool intersects_line(FLOATTYPE &t,
			      const FLOATNAME(LPoint3) &from, 
			      const FLOATNAME(LVector3) &delta) const;
  
  INLINE_MATHUTIL void output(ostream &out) const;
  INLINE_MATHUTIL void write(ostream &out, int indent_level = 0) const;

public:
  INLINE_MATHUTIL void write_datagram(Datagram &dest);
  INLINE_MATHUTIL void read_datagram(DatagramIterator &source);

public:
  FLOATTYPE _a, _b, _c, _d;
};

INLINE_MATHUTIL ostream &operator << (ostream &out, const FLOATNAME(Plane) &p) {
  p.output(out);
  return out;
}

#include "plane_src.I"
