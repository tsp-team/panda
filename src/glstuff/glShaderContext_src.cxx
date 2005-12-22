// Filename: glShaderContext_src.cxx
// Created by: jyelon (01Sep05)
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

TypeHandle CLP(ShaderContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Constructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
CLP(ShaderContext)(ShaderExpansion *s, GSG *gsg) : ShaderContext(s) {
  string header;
  s->parse_init();
  s->parse_line(header, true, true);
  
#ifdef HAVE_CGGL
  _cg_context = (CGcontext)0;
  _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
  _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
  _cg_program[SHADER_type_vert] = (CGprogram)0;
  _cg_program[SHADER_type_frag] = (CGprogram)0;
  
  if (header == "//Cg") {
    // Create the Cg context.
    _cg_context = cgCreateContext();
    if (_cg_context == 0) {
      release_resources();
      cerr << "Cg not supported by this video card.\n";
      return;
    }
    
    // Parse any directives in the source.
    string directive;
    while (!s->parse_eof()) {
      s->parse_line(directive, true, true);
      vector_string pieces;
      tokenize(directive, pieces, " \t");
      if ((pieces.size()==4)&&(pieces[0]=="//Cg")&&(pieces[1]=="profile")) {
        suggest_cg_profile(pieces[2], pieces[3]);
      }
    }
    
    // Select a profile if no preferred profile specified in the source.
    if (_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_vert] = cgGLGetLatestProfile(CG_GL_VERTEX);
    }
    if (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN) {
      _cg_profile[SHADER_type_frag] = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    }
    
    // If we still haven't chosen a profile, give up.
    if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
        (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
      release_resources();
      cerr << "Cg not supported by this video card.\n";
      return;
    }
    
    // Compile the program.
    try_cg_compile(s, gsg);
    cerr << _cg_errors;
    return;
  }
#endif
  
  cerr << s->get_name() << ": unrecognized shader language " << header << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::suggest_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
void CLP(ShaderContext)::
suggest_cg_profile(const string &vpro, const string &fpro)
{
  // If a good profile has already been suggested, ignore suggestion.
  if ((_cg_profile[SHADER_type_vert] != CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] != CG_PROFILE_UNKNOWN)) {
    return;
  }
  
  // Parse the suggestion. If not parseable, print error and ignore.
  _cg_profile[SHADER_type_vert] = parse_cg_profile(vpro, true);
  _cg_profile[SHADER_type_frag] = parse_cg_profile(fpro, false);
  if ((_cg_profile[SHADER_type_vert] == CG_PROFILE_UNKNOWN)||
      (_cg_profile[SHADER_type_frag] == CG_PROFILE_UNKNOWN)) {
    cerr << "Cg: unrecognized profile name: " << vpro << " " << fpro << "\n";
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }

  // If the suggestion is parseable, but not supported, ignore silently.
  if ((!cgGLIsProfileSupported(_cg_profile[SHADER_type_vert]))||
      (!cgGLIsProfileSupported(_cg_profile[SHADER_type_frag]))) {
    _cg_profile[SHADER_type_vert] = CG_PROFILE_UNKNOWN;
    _cg_profile[SHADER_type_frag] = CG_PROFILE_UNKNOWN;
    return;
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::parse_cg_profile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
CGprofile CLP(ShaderContext)::
parse_cg_profile(const string &id, bool vertex)
{
  int nvprofiles = 4;
  int nfprofiles = 4;
  CGprofile vprofiles[] = { CG_PROFILE_ARBVP1, CG_PROFILE_VP20, CG_PROFILE_VP30, CG_PROFILE_VP40 };
  CGprofile fprofiles[] = { CG_PROFILE_ARBFP1, CG_PROFILE_FP20, CG_PROFILE_FP30, CG_PROFILE_FP40 };
  if (vertex) {
    for (int i=0; i<nvprofiles; i++) {
      if (id == cgGetProfileString(vprofiles[i])) {
        return vprofiles[i];
      }
    }
  } else {
    for (int i=0; i<nfprofiles; i++) {
      if (id == cgGetProfileString(fprofiles[i])) {
        return fprofiles[i];
      }
    }
  }
  return CG_PROFILE_UNKNOWN;
}
#endif  // HAVE_CGGL

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::try_cg_compile
//       Access: Private
//  Description: xyz
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
bool CLP(ShaderContext)::
try_cg_compile(ShaderExpansion *s, GSG *gsg)
{
  _cg_errors = "";
  
  cgGetError();
  _cg_program[0] =
    cgCreateProgram(_cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[0], "vshader", (const char**)NULL);
  print_cg_compile_errors(s->get_name(), _cg_context);
  
  cgGetError();
  _cg_program[1] =
    cgCreateProgram(_cg_context, CG_SOURCE, s->_text.c_str(),
                    _cg_profile[1], "fshader", (const char**)NULL);
  print_cg_compile_errors(s->get_name(), _cg_context);
  
  if ((_cg_program[SHADER_type_vert]==0)||(_cg_program[SHADER_type_frag]==0)) {
    release_resources();
    return false;
  }
  
  // The following code is present to work around a bug in the Cg compiler.
  // It does not generate correct code for shadow map lookups when using arbfp1.
  // This is a particularly onerous limitation, given that arbfp1 is the only
  // Cg target that works on radeons.  I suspect this is an intentional
  // omission on nvidia's part.  The following code fetches the output listing,
  // detects the error, repairs the code, and resumbits the repaired code to Cg.
  if ((_cg_profile[1] == CG_PROFILE_ARBFP1) && (gsg->_supports_shadow_filter)) {
    bool shadowunit[32];
    bool anyshadow = false;
    memset(shadowunit, 0, sizeof(shadowunit));
    vector_string lines;
    tokenize(cgGetProgramString(_cg_program[1], CG_COMPILED_PROGRAM), lines, "\n");
    // figure out which texture units contain shadow maps.
    for (int lineno=0; lineno<(int)lines.size(); lineno++) {
      if (lines[lineno].compare(0,21,"#var sampler2DSHADOW ")) {
        continue;
      }
      vector_string fields;
      tokenize(lines[lineno], fields, ":");
      if (fields.size()!=5) {
        continue;
      }
      vector_string words;
      tokenize(trim(fields[2]), words, " ");
      if (words.size()!=2) {
        continue;
      }
      int unit = atoi(words[1].c_str());
      if ((unit < 0)||(unit >= 32)) {
        continue;
      }
      anyshadow = true;
      shadowunit[unit] = true;
    }
    // modify all TEX statements that use the relevant texture units.
    if (anyshadow) {
      for (int lineno=0; lineno<(int)lines.size(); lineno++) {
        if (lines[lineno].compare(0,4,"TEX ")) {
          continue;
        }
        vector_string fields;
        tokenize(lines[lineno], fields, ",");
        if ((fields.size()!=4)||(trim(fields[3]) != "2D;")) {
          continue;
        }
        vector_string texunitf;
        tokenize(trim(fields[2]), texunitf, "[]");
        if ((texunitf.size()!=3)||(texunitf[0] != "texture")||(texunitf[2]!="")) {
          continue;
        }
        int unit = atoi(texunitf[1].c_str());
        if ((unit < 0) || (unit >= 32) || (shadowunit[unit]==false)) {
          continue;
        }
        lines[lineno] = fields[0]+","+fields[1]+","+fields[2]+", SHADOW2D;";
      }
      string result = "!!ARBfp1.0\nOPTION ARB_fragment_program_shadow;\n";
      for (int lineno=1; lineno<(int)lines.size(); lineno++) {
        result += (lines[lineno] + "\n");
      }
      cgDestroyProgram(_cg_program[1]);
      _cg_program[1] =
        cgCreateProgram(_cg_context, CG_OBJECT, result.c_str(),
                        _cg_profile[1], "fshader", (const char**)NULL);
      print_cg_compile_errors(s->get_name(), _cg_context);
      if (_cg_program[SHADER_type_frag]==0) {
        release_resources();
        return false;
      }
    }
  }
  
  bool success = true;
  CGparameter parameter;
  for (int progindex=0; progindex<2; progindex++) {
    for (parameter = cgGetFirstLeafParameter(_cg_program[progindex],CG_PROGRAM);
         parameter != 0;
         parameter = cgGetNextLeafParameter(parameter)) {
      success &= compile_cg_parameter(parameter);
    }
  }
  if (!success) {
    release_resources();
    return false;
  }
  
  cgGLLoadProgram(_cg_program[SHADER_type_vert]);
  cgGLLoadProgram(_cg_program[SHADER_type_frag]);
  
  _cg_errors = s->get_name() + ": compiled to "
    + cgGetProfileString(_cg_profile[SHADER_type_vert]) + " "
    + cgGetProfileString(_cg_profile[SHADER_type_frag]) + "\n";
  return true;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::Destructor
//       Access: Public
//  Description: xyz
////////////////////////////////////////////////////////////////////
CLP(ShaderContext)::
~CLP(ShaderContext)() {
  release_resources();
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::release_resources
//       Access: Public
//  Description: Should deallocate all system resources (such as
//               vertex program handles or Cg contexts).
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
release_resources() {
#ifdef HAVE_CGGL
  if (_cg_context) {
    cgDestroyContext(_cg_context);
    _cg_context = (CGcontext)0;
    _cg_profile[SHADER_type_vert] = (CGprofile)0;
    _cg_profile[SHADER_type_frag] = (CGprofile)0;
    _cg_program[SHADER_type_vert] = (CGprogram)0;
    _cg_program[SHADER_type_frag] = (CGprogram)0;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind
//       Access: Public
//  Description: This function is to be called to enable a new
//               shader.  It also initializes all of the shader's
//               input parameters.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind(GSG *gsg) {
#ifdef HAVE_CGGL
  if (_cg_context != 0) {

    // Pass in k-parameters and transform-parameters
    issue_parameters(gsg);
    
    // Bind the shaders.
    cgGLEnableProfile(_cg_profile[SHADER_type_vert]);
    cgGLBindProgram(_cg_program[SHADER_type_vert]);
    cgGLEnableProfile(_cg_profile[SHADER_type_frag]);
    cgGLBindProgram(_cg_program[SHADER_type_frag]);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::unbind
//       Access: Public
//  Description: This function disables a currently-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
unbind()
{
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    cgGLDisableProfile(_cg_profile[SHADER_type_vert]);
    cgGLDisableProfile(_cg_profile[SHADER_type_frag]);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_cg_auto_bind
//       Access: Public
//  Description: Pass a single system parameter into the shader.
////////////////////////////////////////////////////////////////////
#ifdef HAVE_CGGL
void CLP(ShaderContext)::
issue_cg_auto_bind(const ShaderAutoBind &bind, GSG *gsg)
{
  LVecBase4f t; float xhi,yhi; int px,py;

  CGparameter p = bind.parameter;
  switch(bind.value) {
  case SIC_mat_modelview:
  case SIC_inv_modelview:
  case SIC_tps_modelview:
  case SIC_itp_modelview:
  case SIC_mat_projection:
  case SIC_inv_projection:
  case SIC_tps_projection:
  case SIC_itp_projection:
  case SIC_mat_texture:
  case SIC_inv_texture:
  case SIC_tps_texture:
  case SIC_itp_texture:
  case SIC_mat_modelproj:
  case SIC_inv_modelproj:
  case SIC_tps_modelproj:
  case SIC_itp_modelproj:
    cgGLSetStateMatrixParameter(p, (CGGLenum)((bind.value >> 2)+4), (CGGLenum)(bind.value & 3));
    return;
  case SIC_sys_windowsize:
    t[0] = gsg->_current_display_region->get_pixel_width();
    t[1] = gsg->_current_display_region->get_pixel_height();
    t[2] = 1;
    t[3] = 1;
    cgGLSetParameter4fv(p, t.get_data());
    return;
  case SIC_sys_pixelsize:
    t[0] = 1.0 / gsg->_current_display_region->get_pixel_width();
    t[1] = 1.0 / gsg->_current_display_region->get_pixel_height();
    t[2] = 1;
    t[3] = 1;
    cgGLSetParameter4fv(p, t.get_data());
    return;
  case SIC_sys_cardcenter:
    px = gsg->_current_display_region->get_pixel_width();
    py = gsg->_current_display_region->get_pixel_height();
    xhi = (px*1.0) / Texture::up_to_power_2(px);
    yhi = (py*1.0) / Texture::up_to_power_2(py);
    t[0] = xhi*0.5;
    t[1] = yhi*0.5;
    t[2] = 1;
    t[3] = 1;
    cgGLSetParameter4fv(p, t.get_data());
    return;
  }
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_parameters
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               has changed, but the ShaderExpansion itself has not
//               changed.  It loads new parameters into the
//               already-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_parameters(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    // Pass in k-float parameters.
    for (int i=0; i<(int)_cg_fbind.size(); i++) {
      InternalName *id = _cg_fbind[i].name;
      const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
      cgGLSetParameter4fv(_cg_fbind[i].parameter, input->get_vector().get_data());
    }
    
    // Pass in k-float4x4 parameters.
    for (int i=0; i<(int)_cg_npbind.size(); i++) {
      InternalName *id = _cg_npbind[i].name;
      const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
      const float *dat;
      if (input->get_nodepath().is_empty()) {
        dat = LMatrix4f::ident_mat().get_data();
      } else {
        dat = input->get_nodepath().node()->get_transform()->get_mat().get_data();
      }
      cgGLSetMatrixParameterfc(_cg_npbind[i].parameter, dat);
    }
    
    // Pass in system parameters
    for (int i=0; i<(int)_cg_auto_param.size(); i++) {
      issue_cg_auto_bind(_cg_auto_param[i], gsg);
    }
    
    // Pass in trans,tpose,row,col,xvec,yvec,zvec,pos parameters
    for (int i=0; i<(int)_cg_parameter_bind.size(); i++) {
      bind_cg_transform(_cg_parameter_bind[i], gsg);
    }
  }
#endif
  issue_transform(gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::issue_transform
//       Access: Public
//  Description: This function gets called whenever the RenderState
//               or the TransformState has changed, but the
//               ShaderExpansion itself has not changed.  It loads
//               new parameters into the already-bound shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
issue_transform(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context != 0) {
    // Pass in modelview, projection, etc.
    for (int i=0; i<(int)_cg_auto_trans.size(); i++) {
      issue_cg_auto_bind(_cg_auto_trans[i], gsg);
    }
    // Pass in trans,tpose,row,col,xvec,yvec,zvec,pos parameters
    for (int i=0; i<(int)_cg_transform_bind.size(); i++) {
      bind_cg_transform(_cg_transform_bind[i], gsg);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_vertex_arrays
//       Access: Public
//  Description: Disable all the vertex arrays used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_vertex_arrays(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context)
    for (int i=0; i<(int)_cg_varying.size(); i++)
      cgGLDisableClientState(_cg_varying[i].parameter);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_vertex_arrays
//       Access: Public
//  Description: Disables all vertex arrays used by the previous
//               shader, then enables all the vertex arrays needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable arrays then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
update_shader_vertex_arrays(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_vertex_arrays(gsg);
#ifdef HAVE_CGGL
  if (_cg_context) {
#ifdef SUPPORT_IMMEDIATE_MODE
    if (gsg->_use_sender) {
      cerr << "immediate mode shaders not implemented yet\n";
    } else
#endif // SUPPORT_IMMEDIATE_MODE
    {
      const GeomVertexArrayData *array_data;
      Geom::NumericType numeric_type;
      int start, stride, num_values;
      int nvarying = _cg_varying.size();
      for (int i=0; i<nvarying; i++) {
        InternalName *name = _cg_varying[i].name;
        int texslot = _cg_varying[i].append_uv;
        if (texslot >= 0) {
          const Geom::ActiveTextureStages &active_stages = 
            gsg->_state._texture->get_on_stages();
          if (texslot < (int)active_stages.size()) {
            TextureStage *stage = active_stages[texslot];
            InternalName *texname = stage->get_texcoord_name();
            if (name == InternalName::get_texcoord()) {
              name = texname;
            } else if (texname != InternalName::get_texcoord()) {
              name = name->append(texname->get_basename());
            }
          }
        }
        if (gsg->_vertex_data->get_array_info(name,
                                              array_data, num_values, numeric_type, 
                                              start, stride)) {
          const unsigned char *client_pointer = gsg->setup_array_data(array_data);
          cgGLSetParameterPointer(_cg_varying[i].parameter,
                                  num_values, gsg->get_numeric_type(numeric_type),
                                  stride, client_pointer + start);
          cgGLEnableClientState(_cg_varying[i].parameter);
        } else {
          cgGLDisableClientState(_cg_varying[i].parameter);
        }
      }
    }
  }
#endif // HAVE_CGGL
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::disable_shader_texture_bindings
//       Access: Public
//  Description: Disable all the texture bindings used by this shader.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
disable_shader_texture_bindings(GSG *gsg)
{
#ifdef HAVE_CGGL
  if (_cg_context) {
    for (int i=0; i<(int)_cg_texbind.size(); i++) {
      int texunit = cgGetParameterResourceIndex(_cg_texbind[i].parameter);
      gsg->_glActiveTexture(GL_TEXTURE0 + texunit);
      GLP(Disable)(GL_TEXTURE_1D);
      GLP(Disable)(GL_TEXTURE_2D);
      if (gsg->_supports_3d_texture) {
        GLP(Disable)(GL_TEXTURE_3D);
      }
      if (gsg->_supports_cube_map) {
        GLP(Disable)(GL_TEXTURE_CUBE_MAP);
      }
      // This is probably faster - but maybe not as safe?
      // cgGLDisableTextureParameter(_cg_texbind[i].parameter);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::update_shader_texture_bindings
//       Access: Public
//  Description: Disables all texture bindings used by the previous
//               shader, then enables all the texture bindings needed
//               by this shader.  Extracts the relevant vertex array
//               data from the gsg.
//               The current implementation is inefficient, because
//               it may unnecessarily disable textures then immediately
//               reenable them.  We may optimize this someday.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
update_shader_texture_bindings(CLP(ShaderContext) *prev, GSG *gsg)
{
  if (prev) prev->disable_shader_texture_bindings(gsg);
#ifdef HAVE_CGGL
  if (_cg_context) {
    for (int i=0; i<(int)_cg_texbind.size(); i++) {
      Texture *tex = 0;
      InternalName *id = _cg_texbind[i].name;
      if (id != 0) {
        const ShaderInput *input = gsg->_target._shader->get_shader_input(id);
        tex = input->get_texture();
      } else {
        if (_cg_texbind[i].stage >= gsg->_target._texture->get_num_on_stages()) {
          continue;
        }
        TextureStage *stage = gsg->_target._texture->get_on_stage(_cg_texbind[i].stage);
        tex = gsg->_target._texture->get_on_texture(stage);
      }
      if (_cg_texbind[i].suffix != 0) {
        // The suffix feature is inefficient. It is a temporary hack.
        if (tex == 0) {
          continue;
        }
        tex = tex->load_related(_cg_texbind[i].suffix);
      }
      if ((tex == 0) || (tex->get_texture_type() != _cg_texbind[i].desiredtype)) {
        continue;
      }
      TextureContext *tc = tex->prepare_now(gsg->_prepared_objects, gsg);
      if (tc == (TextureContext*)NULL) {
        continue;
      }
      int texunit = cgGetParameterResourceIndex(_cg_texbind[i].parameter);
      gsg->_glActiveTexture(GL_TEXTURE0 + texunit);

      GLenum target = gsg->get_texture_target(tex->get_texture_type());
      if (target == GL_NONE) {
        // Unsupported texture mode.
        continue;
      }
      GLP(Enable)(target);
      
      gsg->apply_texture(tc);
    }
  }
#endif
}

#ifdef HAVE_CGGL
////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::bind_cg_transform
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
bind_cg_transform(const ShaderTransBind &stb, GSG *gsg)
{
  const float *data;
  CPT(TransformState) src;
  CPT(TransformState) rel;
  
  if (stb.src_name == InternalName::get_camera()) {
    src = TransformState::make_identity();
  } else if (stb.src_name == InternalName::get_view()) {
    src = gsg->_inv_cs_transform;
  } else if (stb.src_name == InternalName::get_model()) {
    src = gsg->get_transform();
  } else if (stb.src_name == InternalName::get_world()) {
    src = gsg->get_scene()->get_world_transform();
  } else {
    const ShaderInput *input = gsg->_target._shader->get_shader_input(stb.src_name);
    if (input->get_nodepath().is_empty()) {
      src = gsg->get_scene()->get_world_transform();
    } else {
      src = gsg->get_scene()->get_world_transform()->
        compose(input->get_nodepath().get_net_transform());
    }
  }

  if (stb.rel_name == InternalName::get_camera()) {
    rel = TransformState::make_identity();
  } else if (stb.rel_name == InternalName::get_view()) {
    rel = gsg->_inv_cs_transform;
  } else if (stb.rel_name == InternalName::get_model()) {
    rel = gsg->get_transform();
  } else if (stb.rel_name == InternalName::get_world()) {
    rel = gsg->get_scene()->get_world_transform();
  } else {
    const ShaderInput *input = gsg->_target._shader->get_shader_input(stb.rel_name);
    if (input->get_nodepath().is_empty()) {
      rel = gsg->get_scene()->get_world_transform();
    } else {
      rel = gsg->get_scene()->get_world_transform()->
        compose(input->get_nodepath().get_net_transform());
    }
  }
  
  CPT(TransformState) total = rel->invert_compose(src);

  //  data = src->get_mat().get_data();
  //  cerr << "Src for " << cgGetParameterName(stb.parameter) << " is\n" << 
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = rel->get_mat().get_data();
  //  cerr << "Rel for " << cgGetParameterName(stb.parameter) << " is\n" << 
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = total->get_mat().get_data();
  //  cerr << "Total for " << cgGetParameterName(stb.parameter) << " is\n" << 
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = gsg->_cs_transform->get_mat().get_data();
  //  cerr << "cs_transform is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";
  //  data = gsg->_internal_transform->get_mat().get_data();
  //  cerr << "internal_transform is\n" <<
  //    data[ 0] << " " << data[ 1] << " " << data[ 2] << " " << data[ 3] << "\n" <<
  //    data[ 4] << " " << data[ 5] << " " << data[ 6] << " " << data[ 7] << "\n" <<
  //    data[ 8] << " " << data[ 9] << " " << data[10] << " " << data[11] << "\n" <<
  //    data[12] << " " << data[13] << " " << data[14] << " " << data[15] << "\n";

  data = total->get_mat().get_data();
  switch (stb.trans_piece) {
  case SHADER_data_matrix: cgGLSetMatrixParameterfc(stb.parameter, data); break;
  case SHADER_data_transpose:  cgGLSetMatrixParameterfr(stb.parameter, data); break;
  case SHADER_data_row0: cgGLSetParameter4fv(stb.parameter, data+ 0); break;
  case SHADER_data_row1: cgGLSetParameter4fv(stb.parameter, data+ 4); break;
  case SHADER_data_row2: cgGLSetParameter4fv(stb.parameter, data+ 8); break;
  case SHADER_data_row3: cgGLSetParameter4fv(stb.parameter, data+12); break;
  case SHADER_data_col0: cgGLSetParameter4f(stb.parameter, data[0], data[4], data[ 8], data[12]); break;
  case SHADER_data_col1: cgGLSetParameter4f(stb.parameter, data[1], data[5], data[ 9], data[13]); break;
  case SHADER_data_col2: cgGLSetParameter4f(stb.parameter, data[2], data[6], data[10], data[14]); break;
  case SHADER_data_col3: cgGLSetParameter4f(stb.parameter, data[3], data[7], data[11], data[15]); break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_words
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter contains 
//               the specified number of words.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_words(CGparameter p, int len)
{
  vector_string words;
  tokenize(cgGetParameterName(p), words, "_");
  if (words.size() != len) {
    errchk_cg_output(p, "parameter name has wrong number of words");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_direction
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct direction.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_direction(CGparameter p, CGenum dir)
{
  if (cgGetParameterDirection(p) != dir) {
    if (dir == CG_IN)    errchk_cg_output(p, "parameter should be declared 'in'");
    if (dir == CG_OUT)   errchk_cg_output(p, "parameter should be declared 'out'");
    if (dir == CG_INOUT) errchk_cg_output(p, "parameter should be declared 'inout'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_variance
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct variance.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_variance(CGparameter p, CGenum var)
{
  if (cgGetParameterVariability(p) != var) {
    if (var == CG_UNIFORM) 
      errchk_cg_output(p, "parameter should be declared 'uniform'");
    if (var == CG_VARYING) 
      errchk_cg_output(p, "parameter should be declared 'varying'");
    if (var == CG_CONSTANT)
      errchk_cg_output(p, "parameter should be declared 'const'");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_prog
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter is a part
//               of the correct program.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_prog(CGparameter p, CGprogram prog, const string &msg)
{
  if (cgGetParameterProgram(p) != prog) {
    string fmsg = "parameter can only be used in a ";
    errchk_cg_output(p, fmsg+msg+" program");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_type
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has the
//               correct type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_type(CGparameter p, CGtype dt)
{
  if (cgGetParameterType(p) != dt) {
    string msg = "parameter should be of type ";
    errchk_cg_output(p, msg + cgGetTypeString(dt));
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_float
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a floating point type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_float(CGparameter p)
{
  CGtype t = cgGetParameterType(p);
  if ((t != CG_FLOAT1)&&(t != CG_FLOAT2)&&(t != CG_FLOAT3)&&(t != CG_FLOAT4)) {
    errchk_cg_output(p, "parameter should have a float type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_parameter_sampler
//       Access: Public, Static
//  Description: Make sure the provided Cg parameter has
//               a texture type.  If not, print
//               error message and return false.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
errchk_cg_parameter_sampler(CGparameter p)
{
  CGtype t = cgGetParameterType(p);
  if ((t!=CG_SAMPLER1D)&&
      (t!=CG_SAMPLER2D)&&
      (t!=CG_SAMPLER3D)&&
      (t!=CG_SAMPLERCUBE)) {
    errchk_cg_output(p, "parameter should have a 'sampler' type");
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::errchk_cg_output
//       Access: Public, Static
//  Description: Print an error message including a description
//               of the specified Cg parameter.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
errchk_cg_output(CGparameter p, const string &msg)
{
  string vstr;
  CGenum v = cgGetParameterVariability(p);
  if (v == CG_UNIFORM)  vstr = "uniform ";
  if (v == CG_VARYING)  vstr = "varying ";
  if (v == CG_CONSTANT) vstr = "const ";

  string dstr;
  CGenum d = cgGetParameterDirection(p);
  if (d == CG_IN)    dstr = "in ";
  if (d == CG_OUT)   dstr = "out ";
  if (d == CG_INOUT) dstr = "inout ";
  
  const char *ts = cgGetTypeString(cgGetParameterType(p));
  
  string err;
  string fn = _shader_expansion->get_name();
  err = fn + ": " + msg + " (" + vstr + dstr + ts + " " + cgGetParameterName(p) + ")\n";
  _cg_errors = _cg_errors + err + "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::print_cg_compile_errors
//       Access: Public, Static
//  Description: Used only after a Cg compile command, to print
//               out any error messages that may have occurred
//               during the Cg shader compilation.  The 'file'
//               is the name of the file containing the Cg code.
////////////////////////////////////////////////////////////////////
void CLP(ShaderContext)::
print_cg_compile_errors(const string &file, CGcontext ctx)
{
  CGerror err = cgGetError();
  if (err != CG_NO_ERROR) {
    if (err == CG_COMPILER_ERROR) {
      string listing = cgGetLastListing(ctx);
      vector_string errlines;
      tokenize(listing, errlines, "\n");
      for (int i=0; i<(int)errlines.size(); i++) {
        string line = trim(errlines[i]);
        if (line != "") {
          _cg_errors += file + " " + errlines[i] + "\n";
        }
      }
    } else {
      _cg_errors += file + ": " + cgGetErrorString(err) + "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GLShaderContext::compile_cg_parameter
//       Access: Public
//  Description: Analyzes a Cg parameter and decides how to
//               bind the parameter to some part of panda's
//               internal state.  Updates one of the cg bind
//               arrays to cause the binding to occur.
////////////////////////////////////////////////////////////////////
bool CLP(ShaderContext)::
compile_cg_parameter(CGparameter p)
{
  string pname = cgGetParameterName(p);
  if (pname.size() == 0) return true;
  if (pname[0] == '$') return true;
  vector_string pieces;
  tokenize(pname, pieces, "_");

  if (pieces.size() < 2) {
    errchk_cg_output(p,"invalid parameter name");
    return false;
  }
  
  if (pieces[0] == "vtx") {
    if ((!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_VARYING)) ||
        (!errchk_cg_parameter_float(p)) ||
        (!errchk_cg_parameter_prog(p, _cg_program[SHADER_type_vert], "vertex")))
      return false;
    ShaderVarying bind;
    bind.parameter = p;
    if (pieces.size() == 2) {
      if (pieces[1]=="position") {
        bind.name = InternalName::get_vertex();
        bind.append_uv = -1;
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="texcoord") {
        bind.name = InternalName::get_texcoord();
        bind.append_uv = atoi(pieces[1].c_str()+8);
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,7)=="tangent") {
        bind.name = InternalName::get_tangent();
        bind.append_uv = atoi(pieces[1].c_str()+7);
        _cg_varying.push_back(bind);
        return true;
      }
      if (pieces[1].substr(0,8)=="binormal") {
        bind.name = InternalName::get_binormal();
        bind.append_uv = atoi(pieces[1].c_str()+8);
        _cg_varying.push_back(bind);
        return true;
      }
    }
    bind.name = InternalName::get_root();
    bind.append_uv = -1;
    for (int i=1; i<(int)(pieces.size()-0); i++)
      bind.name = bind.name->append(pieces[i]);
    _cg_varying.push_back(bind);
    return true;
  }
  
  if ((pieces[0] == "trans")||
      (pieces[0] == "tpose")||
      (pieces[0] == "row0")||
      (pieces[0] == "row1")||
      (pieces[0] == "row2")||
      (pieces[0] == "row3")||
      (pieces[0] == "col0")||
      (pieces[0] == "col1")||
      (pieces[0] == "col2")||
      (pieces[0] == "col3")) {
    if ((!errchk_cg_parameter_words(p,4)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    if ((pieces[2] != "rel")&&(pieces[2] != "to")) {
      errchk_cg_output(p, "syntax error");
      return false;
    }
    ShaderTransBind bind;
    bind.parameter = p;

    if      (pieces[0]=="trans") bind.trans_piece = SHADER_data_matrix;
    else if (pieces[0]=="tpose") bind.trans_piece = SHADER_data_transpose;
    else if (pieces[0]=="row0") bind.trans_piece = SHADER_data_row0;
    else if (pieces[0]=="row1") bind.trans_piece = SHADER_data_row1;
    else if (pieces[0]=="row2") bind.trans_piece = SHADER_data_row2;
    else if (pieces[0]=="row3") bind.trans_piece = SHADER_data_row3;
    else if (pieces[0]=="col0") bind.trans_piece = SHADER_data_col0;
    else if (pieces[0]=="col1") bind.trans_piece = SHADER_data_col1;
    else if (pieces[0]=="col2") bind.trans_piece = SHADER_data_col2;
    else if (pieces[0]=="col3") bind.trans_piece = SHADER_data_col3;

    bind.src_name = InternalName::make(pieces[1]);
    bind.rel_name = InternalName::make(pieces[3]);

    if ((bind.trans_piece == SHADER_data_matrix)||(bind.trans_piece == SHADER_data_transpose)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }

    _cg_parameter_bind.push_back(bind);
    if ((bind.src_name == InternalName::get_model()) ||
        (bind.rel_name == InternalName::get_model())) {
      _cg_transform_bind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mstrans")||
      (pieces[0]=="cstrans")||
      (pieces[0]=="wstrans")||
      (pieces[0]=="mspos")||
      (pieces[0]=="cspos")||
      (pieces[0]=="wspos")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    ShaderTransBind bind;
    bind.parameter = p;
    
    if      (pieces[0]=="wstrans") { bind.rel_name = InternalName::get_world();  bind.trans_piece = SHADER_data_matrix; }
    else if (pieces[0]=="vstrans") { bind.rel_name = InternalName::get_view();   bind.trans_piece = SHADER_data_matrix; }
    else if (pieces[0]=="cstrans") { bind.rel_name = InternalName::get_camera(); bind.trans_piece = SHADER_data_matrix; }
    else if (pieces[0]=="mstrans") { bind.rel_name = InternalName::get_model();  bind.trans_piece = SHADER_data_matrix; }
    else if (pieces[0]=="wspos")   { bind.rel_name = InternalName::get_world();  bind.trans_piece = SHADER_data_row3; }
    else if (pieces[0]=="vspos")   { bind.rel_name = InternalName::get_view();   bind.trans_piece = SHADER_data_row3; }
    else if (pieces[0]=="cspos")   { bind.rel_name = InternalName::get_camera(); bind.trans_piece = SHADER_data_row3; }
    else if (pieces[0]=="mspos")   { bind.rel_name = InternalName::get_model();  bind.trans_piece = SHADER_data_row3; }

    bind.src_name = InternalName::make(pieces[1]);
    
    if ((bind.trans_piece == SHADER_data_matrix)||(bind.trans_piece == SHADER_data_transpose)) {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4x4)) return false;
    } else {
      if (!errchk_cg_parameter_type(p, CG_FLOAT4)) return false;
    }
    _cg_parameter_bind.push_back(bind);
    if ((bind.src_name == InternalName::get_model()) ||
        (bind.rel_name == InternalName::get_model())) {
      _cg_transform_bind.push_back(bind);
    }
    return true;
  }

  if ((pieces[0]=="mat")||
      (pieces[0]=="inv")||
      (pieces[0]=="tps")||
      (pieces[0]=="itp")) {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_type(p, CG_FLOAT4x4)))
      return false;
    ShaderAutoBind bind;
    bind.parameter = p;
    bind.value = 0;
    if      (pieces[1] == "modelview")  bind.value += 0;
    else if (pieces[1] == "projection") bind.value += 4;
    else if (pieces[1] == "texmatrix")  bind.value += 8;
    else if (pieces[1] == "modelproj")  bind.value += 12;
    else {
      errchk_cg_output(p, "unrecognized matrix name");
      return false;
    }
    if      (pieces[0]=="mat") bind.value += 0;
    else if (pieces[0]=="inv") bind.value += 1;
    else if (pieces[0]=="tps") bind.value += 2;
    else if (pieces[0]=="itp") bind.value += 3;
    _cg_auto_trans.push_back(bind);
    return true;
  }

  if (pieces[0] == "sys") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM))) {
      return false;    
    }
    ShaderAutoBind bind;
    bind.parameter = p;
    if (pieces[1] == "pixelsize") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_pixelsize;
      _cg_auto_param.push_back(bind);
      return true;
    }
    if (pieces[1] == "windowsize") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_windowsize;
      _cg_auto_param.push_back(bind);
      return true;
    }
    if (pieces[1] == "cardcenter") {
      if (!errchk_cg_parameter_type(p, CG_FLOAT2)) {
        return false;
      }
      bind.value = SIC_sys_cardcenter;
      _cg_auto_param.push_back(bind);
      return true;
    }
    errchk_cg_output(p,"unknown system parameter");
    return false;
  }

  if (pieces[0] == "tex") {
    if ((!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)) ||
        (!errchk_cg_parameter_sampler(p)))
      return false;
    if ((pieces.size() != 2)&&(pieces.size() != 3)) {
      errchk_cg_output(p, "Invalid parameter name");
      return false;
    }
    ShaderTexBind bind;
    bind.parameter = p;
    bind.name = 0;
    bind.stage = atoi(pieces[1].c_str());
    switch (cgGetParameterType(p)) {
    case CG_SAMPLER1D:   bind.desiredtype = Texture::TT_1d_texture; break;
    case CG_SAMPLER2D:   bind.desiredtype = Texture::TT_2d_texture; break;
    case CG_SAMPLER3D:   bind.desiredtype = Texture::TT_3d_texture; break;
    case CG_SAMPLERCUBE: bind.desiredtype = Texture::TT_cube_map; break;
    default:
      errchk_cg_output(p, "Invalid type for a tex-parameter");
      return false;
    }
    if (pieces.size()==3) {
      bind.suffix = InternalName::make(((string)"-") + pieces[2]);
    }
    _cg_texbind.push_back(bind);
    return true;
  }

  if (pieces[0] == "k") {
    if ((!errchk_cg_parameter_words(p,2)) ||
        (!errchk_cg_parameter_direction(p, CG_IN)) ||
        (!errchk_cg_parameter_variance(p, CG_UNIFORM)))
      return false;
    switch (cgGetParameterType(p)) {
    case CG_FLOAT4: {
      ShaderArgBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      _cg_fbind.push_back(bind);
      break;
    }
    case CG_FLOAT4x4: {
      ShaderArgBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      _cg_npbind.push_back(bind);
      break;
    }
    case CG_SAMPLER1D: { 
      ShaderTexBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      bind.desiredtype=Texture::TT_1d_texture;
      _cg_texbind.push_back(bind);
      break;
    }      
    case CG_SAMPLER2D: {
      ShaderTexBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      bind.desiredtype=Texture::TT_2d_texture;
      _cg_texbind.push_back(bind);
      break;
    }
    case CG_SAMPLER3D: {
      ShaderTexBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      bind.desiredtype=Texture::TT_3d_texture;
      _cg_texbind.push_back(bind);
      break;
    }
    case CG_SAMPLERCUBE: {
      ShaderTexBind bind;
      bind.parameter = p;
      bind.name = InternalName::make(pieces[1]);
      bind.desiredtype = Texture::TT_cube_map;
      _cg_texbind.push_back(bind);
      break;
    }
    default:
      errchk_cg_output(p, "Invalid type for a k-parameter");
      return false;
    }
    return true;
  }
  
  if (pieces[0] == "l") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }
  
  if (pieces[0] == "o") {
    // IMPLEMENT THE ERROR CHECKING
    return true; // Cg handles this automatically.
  }
  
  errchk_cg_output(p, "unrecognized parameter name");
  return false;
}
#endif


