#ifndef MULTIBOOT_SCENE_H
#define MULTIBOOT_SCENE_H

#include "VideoScene.h"

#include "../utils/gba-link-connection/LinkCableMultiboot.hpp"
#include "../utils/gba-link-connection/LinkWirelessMultiboot.hpp"

class MultibootScene : public VideoScene {
 public:
  enum class Mode { CABLE, WIRELESS };

  MultibootScene(Mode mode);

  void init() override;
  void destroy() override;
  void update() override;

 private:
  Mode mode;
  bn::vector<bn::sprite_ptr, 256> uiTextSprites;

  Link::AsyncMultiboot* instance();
  void sendRomViaCable(bool normal = false);
  void sendRomViaWirelessAdapter();
  void printInstructions();
  void launch();
};

#endif  // MULTIBOOT_SCENE_H
