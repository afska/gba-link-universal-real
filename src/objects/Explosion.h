#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "GameObject.h"

class Explosion : public GameObject {
 public:
  Explosion(bn::fixed_point position);

  bool update();

 private:
  bn::optional<bn::sprite_ptr> sprite;
  bn::optional<bn::sprite_animate_action<8>> animation;
  bn::fixed speed = 1;
};

#endif  // EXPLOSION_H
