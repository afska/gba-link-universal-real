#ifndef MULTIBOOT_SCENE_H
#define MULTIBOOT_SCENE_H

#include "VideoScene.h"

class MultibootScene : public VideoScene {
 public:
  enum class Mode { CABLE, WIRELESS };

  MultibootScene(Mode mode, const GBFS_FILE* _fs);

  void init() override;
  void destroy() override;
  void update() override;

 private:
  Mode mode;
  bn::vector<bn::sprite_ptr, 256> uiTextSprites;

  void sendRomViaCable(bool normal = false);
  void sendRomViaWirelessAdapter();
  void printInstructions();
  unsigned playerCount();
  unsigned getPercentage();
  void markReady();
  void reset();
  void launch();
};

#endif  // MULTIBOOT_SCENE_H
