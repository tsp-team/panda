// Filename: guiBackground.h
// Created by:  cary (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIBACKGROUND_H__
#define __GUIBACKGROUND_H__

#include "guiItem.h"
#include "guiManager.h"

class EXPCL_PANDA GuiBackground : public GuiItem {
private:
  PT(GuiLabel) _bg;
  PT(GuiItem) _item;

  INLINE GuiBackground(void);
  virtual void recompute_frame(void);
PUBLISHED:
  GuiBackground(const string&, GuiItem*);
  GuiBackground(const string&, GuiItem*, Texture*);
  ~GuiBackground(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void unmanage(void);

  virtual int freeze(void);
  virtual int thaw(void);

  virtual void set_scale(float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  INLINE void set_color(float, float, float, float);
  INLINE void set_color(const Colorf&);
  INLINE Colorf get_color(void) const;

  virtual void output(ostream&) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiBackground",
		  GuiItem::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "guiBackground.I"

#endif /* __GUIBACKGROUND_H__ */
