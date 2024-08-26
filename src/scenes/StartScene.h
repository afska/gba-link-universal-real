#ifndef START_SCENE_H
#define START_SCENE_H

#include "Scene.h"

#include "../objects/Horse.h"
#include "../utils/PixelBlink.h"

class StartScene : public Scene {
 public:
  StartScene(const GBFS_FILE* _fs);

  void init() override;
  void update() override;

 private:
  bn::optional<bn::regular_bg_ptr> background;
  bn::unique_ptr<Horse> horse;
  bn::vector<bn::sprite_ptr, 64> textSprites;
  bn::sprite_text_generator textGenerator;
  bn::sprite_text_generator textGeneratorAccent;
  bn::fixed videoFrame = 0;
  int lastBeat = 0;
  bn::fixed extraSpeed = 0;
  bn::unique_ptr<PixelBlink> pixelBlink;

  bool isConnected = false;
  unsigned counter = 0;
  unsigned received = 0;
  bool error = false;

  void onConnected();
  void onDisconnected();
  void onError(unsigned expected, unsigned actual);
  void updateVideo();
  void print(bn::string<128> text);
};

#endif  // START_SCENE_H
