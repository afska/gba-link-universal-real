// (0) Include the header
#include "utils/gba-link-connection/LinkUniversal.hpp"

#include "player/player.h"
#include "scenes/StartScene.h"
#include "utils/gbfs/gbfs.h"

#include "../butano/hw/include/bn_hw_irq.h"
#include "bn_bg_palettes.h"
#include "bn_bgs_mosaic.h"
#include "bn_blending.h"
#include "bn_core.h"
#include "bn_memory.h"
#include "bn_optional.h"
#include "bn_sprite_palettes.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprites_mosaic.h"
#include "bn_unique_ptr.h"

// (1) Create a LinkUniversal instance
LinkUniversal* linkUniversal = new LinkUniversal(
    LinkUniversal::Protocol::AUTODETECT,
    "LinkUNI",
    {.baudRate = LinkCable::BAUD_RATE_1,
     .timeout = LINK_CABLE_DEFAULT_TIMEOUT,
     .interval = LINK_CABLE_DEFAULT_INTERVAL,
     .sendTimerId = 1},
    {.retransmission = true,
     .maxPlayers = 2,
     .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
     .interval = LINK_WIRELESS_DEFAULT_INTERVAL,
     .sendTimerId = LINK_WIRELESS_DEFAULT_SEND_TIMER_ID,
     .asyncACKTimerId = LINK_WIRELESS_DEFAULT_ASYNC_ACK_TIMER_ID});

static const GBFS_FILE* fs = find_first_gbfs_file(0);
bn::optional<bn::unique_ptr<Scene>> scene;

void ISR_VBlank();

void update() {
  bn::core::update();
  player_update(0, [](unsigned current) {});
}

int main() {
  bn::core::init(ISR_VBlank);  // << call LINK_UNIVERSAL_ISR_VBLANK()

  // (2) Add the required interrupt service routines
  bn::hw::irq::set_isr(bn::hw::irq::id::SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
  bn::hw::irq::set_isr(bn::hw::irq::id::TIMER1, LINK_UNIVERSAL_ISR_TIMER);
  bn::hw::irq::enable(bn::hw::irq::id::SERIAL);
  bn::hw::irq::enable(bn::hw::irq::id::TIMER1);

  // (3) Initialize the library
  linkUniversal->activate();

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
  LINK_UNIVERSAL_ISR_VBLANK();
  bn::core::default_vblank_handler();
}
