#ifndef SCENE_H
#define SCENE_H

#include "../utils/gbfs/gbfs.h"

#include "bn_affine_bg_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_display.h"
#include "bn_log.h"
#include "bn_random.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprites.h"
#include "bn_string.h"
#include "bn_unique_ptr.h"
#include "bn_vector.h"

class Scene {
 public:
  Scene(const GBFS_FILE* _fs) : fs(_fs) {}

  virtual void init() = 0;
  virtual void update() = 0;

  virtual ~Scene() = default;

 protected:
  const GBFS_FILE* fs;

  template <typename F, typename Type, int MaxSize>
  inline void iterate(bn::vector<Type, MaxSize>& vector, F action) {
    for (auto it = vector.begin(); it != vector.end();) {
      bool erase = action(it->get());

      if (erase)
        it = vector.erase(it);
      else
        ++it;
    }
  }
};

#endif  // SCENE_H
