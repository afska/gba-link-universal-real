#ifndef MULTIBOOT_CABLE_SCENE_SCENE_H
#define MULTIBOOT_CABLE_SCENE_SCENE_H

#include "VideoScene.h"

class MultibootCableScene : public VideoScene {
 public:
  static inline bool IS_SENDING = false;

  MultibootCableScene(const GBFS_FILE* _fs);

  void init() override;
  void update() override;

 private:
  bn::vector<bn::sprite_ptr, 256> uiTextSprites;

  void sendRom();
};

#endif  // MULTIBOOT_CABLE_SCENE_SCENE_H
