#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#begin lib_target

  #define TARGET dxgsg
  #define LOCAL_LIBS \
    cull gsgmisc gsgbase gobj sgattrib sgraphutil graph display light \
    putil linmath sgraph mathutil pnmimage event
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     
  
  #define INSTALL_HEADERS \
    config_dxgsg.h dxGraphicsStateGuardian.I dxGraphicsStateGuardian.h \
    dxTextureContext.h dxGeomNodeContext.h dxGeomNodeContext.I

  #define SOURCES \
    dxGraphicsStateGuardian.cxx dxSavedFrameBuffer.I dxSavedFrameBuffer.h $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_dxgsg.cxx dxSavedFrameBuffer.cxx dxTextureContext.cxx dxGeomNodeContext.cxx

#end lib_target
