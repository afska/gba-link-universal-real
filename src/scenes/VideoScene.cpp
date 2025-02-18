#include "VideoScene.h"

#include "../assets/StartVideo.h"
#include "../assets/fonts/common_fixed_8x16_sprite_font.h"
#include "../assets/fonts/common_fixed_8x16_sprite_font_accent.h"
#include "../utils/Math.h"

#include "bn_blending.h"
#include "bn_memory.h"

#include "bn_music_items.h"

#define NUM_EXPLOSIONS 6

VideoScene::VideoScene()
    : Scene(),
      horse(bn::unique_ptr{new Horse({0, 0})}),
      textGenerator(common_fixed_8x16_sprite_font),
      textGeneratorAccent(common_fixed_8x16_sprite_font_accent),
      pixelBlink(bn::unique_ptr{new PixelBlink(0.5)}) {
  horse->showGun = false;
  horse->setPosition({HORSE_X, HORSE_Y}, true);
  horse->update();
  updateVideo();
  textGenerator.set_center_alignment();
  textGeneratorAccent.set_center_alignment();
  textGenerator.set_z_order(-1);
  textGenerator.set_bg_priority(0);
  textGeneratorAccent.set_z_order(-1);
  textGeneratorAccent.set_bg_priority(0);
}

void VideoScene::init() {
  bn::music_items::cyberrid.play(1);
}

void VideoScene::update() {
  // Rhythm
  bool isNewBeat = videoFrame % 16 == 0;
  if (isNewBeat) {
    horse->jump();
    for (int i = 0; i < NUM_EXPLOSIONS; i++)
      addExplosion();
  }

  // Background
  pixelBlink->update();
  updateVideo();

  // Explosions
  iterate(explosions, [](Explosion* it) { return it->update(); });

  // Horse
  horse->setPosition({horse->getPosition().x(), HORSE_Y}, true);
  horse->update();
  horse->setFlipX(horse->getCenteredPosition().x() > 0);
}

void VideoScene::updateVideo() {
  background.reset();
  background = StartVideo::getFrame(videoFrame.floor_integer())
                   .create_bg((256 - Math::SCREEN_WIDTH) / 2,
                              (256 - Math::SCREEN_HEIGHT) / 2);
  background.get()->set_mosaic_enabled(true);
  videoFrame += 1;
  if (videoFrame >= 150)
    videoFrame = 0;

  bn::blending::set_transparency_alpha(videoFrame > 75 ? 0.75 : 0.25);
}

void VideoScene::addExplosion() {
  if (!explosions.full())
    explosions.push_back(bn::unique_ptr{new Explosion(
        {random.get_fixed(-100, 100), random.get_fixed(-100, 100)})});
}
