// Filename: baseParticle.C
// Created by:  charles (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(int lifespan, bool alive) :
  _age(0.0f), _lifespan(lifespan), _alive(alive) {
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticle
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticle::
BaseParticle(const BaseParticle &copy) {
  _age = copy._age;
  _lifespan = copy._lifespan;
  _alive = copy._alive;
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticle
//      Access : Public
// Description : Default Destructor
////////////////////////////////////////////////////////////////////
BaseParticle::
~BaseParticle(void) {
}

////////////////////////////////////////////////////////////////////
//    Function : get_theta
//      Access : Public
// Description : for spriteParticleRenderer
////////////////////////////////////////////////////////////////////
float BaseParticle::
get_theta(void) const {
  return 0.0f;
}
