#include "player/player.h"
#include "scenes/StartScene.h"
#include "utils/gbfs/gbfs.h"

#include "bn_bg_palettes.h"
#include "bn_bgs_mosaic.h"
#include "bn_blending.h"
#include "bn_core.h"
#include "bn_optional.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprites_mosaic.h"
#include "bn_unique_ptr.h"

static const GBFS_FILE* fs = find_first_gbfs_file(0);
bn::optional<bn::unique_ptr<Scene>> scene;

void ISR_VBlank();

void update() {
  bn::core::update();
  player_update(0, [](unsigned current) {});
}

int main() {
  bn::core::init(ISR_VBlank);

  BN_ASSERT(fs != NULL,
            "GBFS file not found.\nUse the ROM that ends with .out.gba!");

  player_init();

  scene = bn::unique_ptr{(Scene*)new StartScene(fs)};
  scene->get()->init();

  while (true) {
    scene->get()->update();
    update();
  }
}

BN_CODE_IWRAM void ISR_VBlank() {
  player_onVBlank();
  bn::core::default_vblank_handler();
}
