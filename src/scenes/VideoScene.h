#ifndef VIDEO_SCENE_H
#define VIDEO_SCENE_H

#include "Scene.h"

#include "../objects/Explosion.h"
#include "../objects/Horse.h"
#include "../utils/PixelBlink.h"

#define HORSE_X 40
#define HORSE_Y 90

class VideoScene : public Scene {
 public:
  VideoScene(const GBFS_FILE* _fs);

  void init() override;
  void update() override;

 protected:
  bn::optional<bn::regular_bg_ptr> background;
  bn::unique_ptr<Horse> horse;
  bn::sprite_text_generator textGenerator;
  bn::sprite_text_generator textGeneratorAccent;
  bn::vector<bn::sprite_ptr, 256> textSprites;
  bn::fixed videoFrame = 0;
  int lastBeat = 0;
  bn::fixed extraSpeed = 0;
  bn::unique_ptr<PixelBlink> pixelBlink;
  bn::vector<bn::unique_ptr<Explosion>, 64> explosions;
  bn::random random;

  void updateVideo();
  void addExplosion();
  void print(bn::string<128> text);
};

#endif  // VIDEO_SCENE_H
