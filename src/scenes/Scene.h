#ifndef SCENE_H
#define SCENE_H

#include "../utils/gbfs/gbfs.h"

#include "bn_affine_bg_ptr.h"
#include "bn_bg_palettes.h"
#include "bn_bgs_mosaic.h"
#include "bn_blending.h"
#include "bn_camera_ptr.h"
#include "bn_core.h"
#include "bn_display.h"
#include "bn_log.h"
#include "bn_memory.h"
#include "bn_optional.h"
#include "bn_random.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_item.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprites.h"
#include "bn_sprites_mosaic.h"
#include "bn_string.h"
#include "bn_unique_ptr.h"
#include "bn_vector.h"

enum Screen {
  NO,
  MAIN,
  MULTIBOOT_CABLE,
  MULTIBOOT_WIRELESS,
};

class Scene {
 public:
  Scene(const GBFS_FILE* _fs) : fs(_fs) {}

  virtual void init() = 0;
  virtual void destroy() = 0;
  virtual void update() = 0;

  virtual ~Scene() = default;

  bool hasNextScreen() { return nextScreen != Screen::NO; }
  Screen getNextScreen() { return nextScreen; }

 protected:
  const GBFS_FILE* fs;
  Screen nextScreen = Screen::NO;

  void setNextScreen(Screen screen) { nextScreen = screen; }

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
