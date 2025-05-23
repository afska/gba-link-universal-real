#ifndef START_SCENE_H
#define START_SCENE_H

#include "VideoScene.h"

class StartScene : public VideoScene {
 public:
  void init() override;
  void destroy() override;
  void update() override;

 private:
  bn::vector<bn::sprite_ptr, 256> uiTextSprites;
  bn::vector<bn::sprite_ptr, 256> creditsSprites;

  bool isConnected = false;
  unsigned counter = 0;
  unsigned received = 0;
  bool error = false;
  bool credits = false;

  void send();
  void onConnected();
  void onDisconnected();
  void onError(unsigned expected, unsigned actual);
  void print(bn::string<128> text);
  void printCredits();
  void showCredits();
  void hideCredits();
};

#endif  // START_SCENE_H
