#include "Explosion.h"
#include "bn_sprite_items_explosion.h"

Explosion::Explosion(bn::fixed_point position)
    : sprite(bn::sprite_items::explosion.create_sprite(position)),
      animation(bn::create_sprite_animate_action_once(
          sprite,
          2,
          bn::sprite_items::explosion.tiles_item(),
          0,
          1,
          2,
          3,
          2,
          1,
          0,
          0)) {
  sprite.set_blending_enabled(true);
}

bool Explosion::update() {
  sprite.set_y(sprite.y() + speed);
  speed += 0.1;

  if (animation.done())
    return true;
  else {
    animation.update();
    return false;
  }
}
