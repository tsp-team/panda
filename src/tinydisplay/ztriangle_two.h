void FNAME(ZB_fillTriangleFlat) (ZBuffer *zb,
                                 ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  int color;
  int or, og, ob, oa;

#define INTERP_Z

#define DRAW_INIT()                                     \
  {                                                     \
    if (!ACMP(zb, p2->a)) {                             \
      return;                                           \
    }                                                   \
    or = p2->r;                                         \
    og = p2->g;                                         \
    ob = p2->b;                                         \
    oa = p2->a;                                         \
    color=RGBA_TO_PIXEL(or, og, ob, oa);                \
  }
 
#define PUT_PIXEL(_a)                           \
  {                                             \
    zz=z >> ZB_POINT_Z_FRAC_BITS;               \
    if (ZCMP(pz[_a], zz)) {                     \
      STORE_PIX(pp[_a], color, or, og, ob, oa); \
      STORE_Z(pz[_a], zz);                      \
    }                                           \
    z+=dzdx;                                    \
  }

#include "ztriangle.h"
}

/*
 * Smooth filled triangle.
 * The code below is very tricky :)
 */

void FNAME(ZB_fillTriangleSmooth) (ZBuffer *zb,
                                   ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() 				\
  {						\
  }

#define PUT_PIXEL(_a)                                           \
  {                                                             \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                               \
    if (ZCMP(pz[_a], zz)) {                                     \
      if (ACMP(zb, oa1)) {                                      \
        STORE_PIX(pp[_a], RGBA_TO_PIXEL(or1, og1, ob1, oa1), or1, og1, ob1, oa1); \
        STORE_Z(pz[_a], zz);                                    \
      }                                                         \
    }                                                           \
    z+=dzdx;                                                    \
    og1+=dgdx;                                                  \
    or1+=drdx;                                                  \
    ob1+=dbdx;                                                  \
    oa1+=dadx;                                                  \
  }

#include "ztriangle.h"
}

void FNAME(ZB_fillTriangleMapping) (ZBuffer *zb,
                                    ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;

#define INTERP_Z
#define INTERP_ST

#define DRAW_INIT()				\
  {						\
    texture = &zb->current_texture;                \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                              \
      if (ACMP(zb, PIXEL_A(tmp))) {                                     \
        STORE_PIX(pp[_a], tmp, PIXEL_R(tmp), PIXEL_G(tmp), PIXEL_B(tmp), PIXEL_A(tmp)); \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#include "ztriangle.h"
}

void FNAME(ZB_fillTriangleMappingFlat) (ZBuffer *zb,
                                        ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;
  int or, og, ob, oa;

#define INTERP_Z
#define INTERP_ST

#define DRAW_INIT()				\
  {						\
    texture = &zb->current_texture;                \
    or = p2->r;                                 \
    og = p2->g;                                 \
    ob = p2->b;                                 \
    oa = p2->a;                                 \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                              \
      int a = oa * PIXEL_A(tmp) >> 16;                                  \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(or * PIXEL_R(tmp) >> 16,                \
                                og * PIXEL_G(tmp) >> 16,                \
                                ob * PIXEL_B(tmp) >> 16,                \
                                a),                                     \
                  or * PIXEL_R(tmp) >> 16,                              \
                  og * PIXEL_G(tmp) >> 16,                              \
                  ob * PIXEL_B(tmp) >> 16,                              \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#include "ztriangle.h"
}

void FNAME(ZB_fillTriangleMappingSmooth) (ZBuffer *zb,
                                          ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;

#define INTERP_Z
#define INTERP_ST
#define INTERP_RGB

#define DRAW_INIT()				\
  {						\
    texture = &zb->current_texture;                \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                                    \
      int a = oa1 * PIXEL_A(tmp) >> 16;                                 \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(or1 * PIXEL_R(tmp) >> 16,               \
                                og1 * PIXEL_G(tmp) >> 16,               \
                                ob1 * PIXEL_B(tmp) >> 16,               \
                                a),                                     \
                  or1 * PIXEL_R(tmp) >> 16,                             \
                  og1 * PIXEL_G(tmp) >> 16,                             \
                  ob1 * PIXEL_B(tmp) >> 16,                             \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#include "ztriangle.h"
}

/*
 * Texture mapping with perspective correction.
 * We use the gradient method to make less divisions.
 * TODO: pipeline the division
 */
void FNAME(ZB_fillTriangleMappingPerspective) (ZBuffer *zb,
                                               ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;
  float fdzdx,fndzdx,ndszdx,ndtzdx;

#define INTERP_Z
#define INTERP_STZ

#define NB_INTERP 8

#define DRAW_INIT()				\
  {						\
    texture = &zb->current_texture;                \
    fdzdx=(float)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
  }


#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                                    \
      if (ACMP(zb, PIXEL_A(tmp))) {                                     \
        STORE_PIX(pp[_a], tmp, PIXEL_R(tmp), PIXEL_G(tmp), PIXEL_B(tmp), PIXEL_A(tmp)); \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                     \
  {                                                     \
    register unsigned short *pz;                        \
    register PIXEL *pp;                                 \
    register unsigned int s,t,z,zz;                     \
    register int n,dsdx,dtdx;                           \
    float sz,tz,fz,zinv;                                \
    n=(x2>>16)-x1;                                      \
    fz=(float)z1;                                       \
    zinv=1.0 / fz;                                      \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);              \
    pz=pz1+x1;                                          \
    z=z1;						\
    sz=sz1;                                             \
    tz=tz1;                                             \
    while (n>=(NB_INTERP-1)) {                          \
      {                                                 \
        float ss,tt;                                    \
        ss=(sz * zinv);                                 \
        tt=(tz * zinv);                                 \
        s=(int) ss;                                     \
        t=(int) tt;                                     \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );         \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );         \
        fz+=fndzdx;                                     \
        zinv=1.0 / fz;                                  \
      }                                                 \
      PUT_PIXEL(0);                                     \
      PUT_PIXEL(1);                                     \
      PUT_PIXEL(2);                                     \
      PUT_PIXEL(3);                                     \
      PUT_PIXEL(4);                                     \
      PUT_PIXEL(5);                                     \
      PUT_PIXEL(6);                                     \
      PUT_PIXEL(7);                                     \
      pz+=NB_INTERP;                                    \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);      \
      n-=NB_INTERP;                                     \
      sz+=ndszdx;                                       \
      tz+=ndtzdx;                                       \
    }                                                   \
    {                                                   \
      float ss,tt;                                      \
      ss=(sz * zinv);                                   \
      tt=(tz * zinv);                                   \
      s=(int) ss;                                       \
      t=(int) tt;                                       \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );           \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );           \
    }                                                   \
    while (n>=0) {                                      \
      PUT_PIXEL(0);                                     \
      pz+=1;                                            \
      pp=(PIXEL *)((char *)pp + PSZB);                  \
      n-=1;                                             \
    }                                                   \
  }
  
#include "ztriangle.h"
}

/*
 * Flat shaded triangle, with perspective-correct mapping.
 */

void FNAME(ZB_fillTriangleMappingPerspectiveFlat) (ZBuffer *zb,
                                                   ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;
  float fdzdx,fndzdx,ndszdx,ndtzdx;
  int or, og, ob, oa;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB


#define DRAW_INIT() 				\
  {						\
    texture = &zb->current_texture;                \
    fdzdx=(float)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
    or = p2->r;                                 \
    og = p2->g;                                 \
    ob = p2->b;                                 \
    oa = p2->a;                                 \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                                    \
      int a = oa * PIXEL_A(tmp) >> 16;                                  \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(or * PIXEL_R(tmp) >> 16,                \
                                og * PIXEL_G(tmp) >> 16,                \
                                ob * PIXEL_B(tmp) >> 16,                \
                                a),                                     \
                  or * PIXEL_R(tmp) >> 16,                              \
                  og * PIXEL_G(tmp) >> 16,                              \
                  ob * PIXEL_B(tmp) >> 16,                              \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                     \
  {                                                     \
    register unsigned short *pz;                        \
    register PIXEL *pp;                                 \
    register unsigned int s,t,z,zz;                     \
    register int n,dsdx,dtdx;                           \
    register unsigned int or1,og1,ob1,oa1;              \
    float sz,tz,fz,zinv;                                \
    n=(x2>>16)-x1;                                      \
    fz=(float)z1;                                       \
    zinv=1.0 / fz;                                      \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);              \
    pz=pz1+x1;                                          \
    z=z1;						\
    sz=sz1;                                             \
    tz=tz1;                                             \
    or1 = r1;                                           \
    og1 = g1;                                           \
    ob1 = b1;                                           \
    oa1 = a1;                                           \
    while (n>=(NB_INTERP-1)) {                          \
      {                                                 \
        float ss,tt;                                    \
        ss=(sz * zinv);                                 \
        tt=(tz * zinv);                                 \
        s=(int) ss;                                     \
        t=(int) tt;                                     \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );         \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );         \
        fz+=fndzdx;                                     \
        zinv=1.0 / fz;                                  \
      }                                                 \
      PUT_PIXEL(0);                                     \
      PUT_PIXEL(1);                                     \
      PUT_PIXEL(2);                                     \
      PUT_PIXEL(3);                                     \
      PUT_PIXEL(4);                                     \
      PUT_PIXEL(5);                                     \
      PUT_PIXEL(6);                                     \
      PUT_PIXEL(7);                                     \
      pz+=NB_INTERP;                                    \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);      \
      n-=NB_INTERP;                                     \
      sz+=ndszdx;                                       \
      tz+=ndtzdx;                                       \
    }                                                   \
    {                                                   \
      float ss,tt;                                      \
      ss=(sz * zinv);                                   \
      tt=(tz * zinv);                                   \
      s=(int) ss;                                       \
      t=(int) tt;                                       \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );           \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );           \
    }                                                   \
    while (n>=0) {                                      \
      PUT_PIXEL(0);                                     \
      pz+=1;                                            \
      pp=(PIXEL *)((char *)pp + PSZB);                  \
      n-=1;                                             \
    }                                                   \
  }

#include "ztriangle.h"
}

/*
 * Smooth filled triangle, with perspective-correct mapping.
 */

void FNAME(ZB_fillTriangleMappingPerspectiveSmooth) (ZBuffer *zb,
                                                     ZBufferPoint *p0,ZBufferPoint *p1,ZBufferPoint *p2)
{
  ZTexture *texture;
  float fdzdx,fndzdx,ndszdx,ndtzdx;

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT() 				\
  {						\
    texture = &zb->current_texture;             \
    fdzdx=(float)dzdx;                          \
    fndzdx=NB_INTERP * fdzdx;                   \
    ndszdx=NB_INTERP * dszdx;                   \
    ndtzdx=NB_INTERP * dtzdx;                   \
  }

#define PUT_PIXEL(_a)                                                   \
  {                                                                     \
    zz=z >> ZB_POINT_Z_FRAC_BITS;                                       \
    if (ZCMP(pz[_a], zz)) {                                             \
      tmp = ZB_LOOKUP_TEXTURE(texture, s, t);                           \
      int a = oa1 * PIXEL_A(tmp) >> 16;                                 \
      if (ACMP(zb, a)) {                                                \
        STORE_PIX(pp[_a],                                               \
                  RGBA_TO_PIXEL(or1 * PIXEL_R(tmp) >> 16,               \
                                og1 * PIXEL_G(tmp) >> 16,               \
                                ob1 * PIXEL_B(tmp) >> 16,               \
                                a),                                     \
                  or1 * PIXEL_R(tmp) >> 16,                             \
                  og1 * PIXEL_G(tmp) >> 16,                             \
                  ob1 * PIXEL_B(tmp) >> 16,                             \
                  a);                                                   \
        STORE_Z(pz[_a], zz);                                            \
      }                                                                 \
    }                                                                   \
    z+=dzdx;                                                            \
    og1+=dgdx;                                                          \
    or1+=drdx;                                                          \
    ob1+=dbdx;                                                          \
    oa1+=dadx;                                                          \
    s+=dsdx;                                                            \
    t+=dtdx;                                                            \
  }

#define DRAW_LINE()                                     \
  {                                                     \
    register unsigned short *pz;                        \
    register PIXEL *pp;                                 \
    register unsigned int s,t,z,zz;                     \
    register int n,dsdx,dtdx;                           \
    register unsigned int or1,og1,ob1,oa1;              \
    float sz,tz,fz,zinv;                                \
    n=(x2>>16)-x1;                                      \
    fz=(float)z1;                                       \
    zinv=1.0 / fz;                                      \
    pp=(PIXEL *)((char *)pp1 + x1 * PSZB);              \
    pz=pz1+x1;                                          \
    z=z1;						\
    sz=sz1;                                             \
    tz=tz1;                                             \
    or1 = r1;                                           \
    og1 = g1;                                           \
    ob1 = b1;                                           \
    oa1 = a1;                                           \
    while (n>=(NB_INTERP-1)) {                          \
      {                                                 \
        float ss,tt;                                    \
        ss=(sz * zinv);                                 \
        tt=(tz * zinv);                                 \
        s=(int) ss;                                     \
        t=(int) tt;                                     \
        dsdx= (int)( (dszdx - ss*fdzdx)*zinv );         \
        dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );         \
        fz+=fndzdx;                                     \
        zinv=1.0 / fz;                                  \
      }                                                 \
      PUT_PIXEL(0);                                     \
      PUT_PIXEL(1);                                     \
      PUT_PIXEL(2);                                     \
      PUT_PIXEL(3);                                     \
      PUT_PIXEL(4);                                     \
      PUT_PIXEL(5);                                     \
      PUT_PIXEL(6);                                     \
      PUT_PIXEL(7);                                     \
      pz+=NB_INTERP;                                    \
      pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);      \
      n-=NB_INTERP;                                     \
      sz+=ndszdx;                                       \
      tz+=ndtzdx;                                       \
    }                                                   \
    {                                                   \
      float ss,tt;                                      \
      ss=(sz * zinv);                                   \
      tt=(tz * zinv);                                   \
      s=(int) ss;                                       \
      t=(int) tt;                                       \
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );           \
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );           \
    }                                                   \
    while (n>=0) {                                      \
      PUT_PIXEL(0);                                     \
      pz+=1;                                            \
      pp=(PIXEL *)((char *)pp + PSZB);                  \
      n-=1;                                             \
    }                                                   \
  }

#include "ztriangle.h"
}

#undef ACMP
#undef ZCMP
#undef STORE_PIX
#undef STORE_Z
#undef FNAME