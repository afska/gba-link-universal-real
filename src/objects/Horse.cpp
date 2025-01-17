#include "Horse.h"

#include "../utils/Math.h"
#include "bn_sprite_items_start_3dhorse.h"

#define HITBOX_Y 8

const bn::fixed SCREEN_LIMIT_X = 10;
const bn::array<bn::fixed, 18> JUMP_Y_OFFSET = {-3,  -7,  -10, -13, -15, -13,
                                                -12, -11, -10, -9,  -8,  -7,
                                                -6,  -5,  -4,  -3,  -2,  -1};

const unsigned GUN_OFFSET[2] = {35, 33};
const int GUN_PIVOT_OFFSET[2] = {-12, -1};
const int GUN_SHOOTING_POINT_OFFSET[2] = {10, -1};
const int GUN_FLIPPED_OFFSET_X = -14;
const unsigned GUN_ANIMATION_WAIT = 0;
const bn::fixed GUN_ROTATION_SPEED = 10;

Horse::Horse(bn::fixed_point _topLeftPosition)
    : TopLeftGameObject(bn::sprite_items::start_3dhorse.create_sprite(0, 0)) {
  boundingBox.set_dimensions(bn::fixed_size(24, 32));
  // boundingBoxPreview = SpriteProvider::hitbox().create_sprite(0, HITBOX_Y);
  // DEBUG (uncomment this to see the bounding box ^^^^^)

  setPosition(_topLeftPosition, false);
  setIdleState();
  mainSprite.set_bg_priority(1);
  mainSprite.set_blending_enabled(true);
}

void Horse::update() {
  updateAngle();
  updateAnimations();

  bounceFrame = bn::max(bounceFrame - 1, 0);

  if (disappearPosition.has_value()) {
    auto newScale = mainSprite.horizontal_scale() - 0.005;
    if (newScale <= 0) {
      newScale = 0.005;
      mainSprite.set_visible(false);
    }
    mainSprite.set_scale(newScale);
    mainSprite.set_rotation_angle(
        Math::normalizeAngle(mainSprite.rotation_angle() + 5));
    Math::moveSpriteTowards(mainSprite, disappearPosition.value(), 1, 1, false);
    setCenteredPosition(mainSprite.position());
  } else {
  }

  if (hurtCooldown > 0) {
    hurtCooldown--;
    if (!isHurt() && !isGhost)
      mainSprite.set_visible(hurtCooldown % 2 == 0);
  }

  setPosition(topLeftPosition, isMoving);
}

void Horse::bounce() {
  if (disappearPosition.has_value())
    return;

  bounceFrame = Math::BOUNCE_STEPS.size() - 1;
}

void Horse::shoot() {}

void Horse::jump() {
  if (isBusy())
    return;

  setJumpingState();
}

void Horse::hurt() {
  if (isHurt())
    return;
  setHurtState();
}

void Horse::aim(bn::fixed_point newDirection) {
  targetAngle =
      Math::ANGLE_MATRIX[(int)newDirection.y() + 1][(int)newDirection.x() + 1];
}

void Horse::setPosition(bn::fixed_point newPosition, bool isNowMoving) {
  if (newPosition.x() < SCREEN_LIMIT_X)
    newPosition.set_x(SCREEN_LIMIT_X);
  if (newPosition.x() >
      Math::SCREEN_WIDTH - mainSpriteDimensions.width() - SCREEN_LIMIT_X)
    newPosition.set_x(Math::SCREEN_WIDTH - mainSpriteDimensions.width() -
                      SCREEN_LIMIT_X);

  int bounceOffsetY = -Math::BOUNCE_STEPS[bounceFrame];

  setTopLeftPosition(newPosition);
  mainSprite.set_y(getCenteredPosition().y() + bounceOffsetY);

  setIsMoving(isNowMoving);

  boundingBox.set_position(getCenteredPosition() +
                           bn::fixed_point(0, HITBOX_Y));
}

void Horse::setFlipX(bool flipX) {
  mainSprite.set_horizontal_flip(flipX);
}

bn::fixed_point Horse::getShootingPoint() {
  return {0, 0};
}

bn::fixed_point Horse::getShootingDirection() {
  auto angle = targetAngle;
  return {bn::degrees_cos(angle), -bn::degrees_sin(angle)};
}

void Horse::setIsMoving(bool isNowMoving) {
  if (isNowMoving != this->isMoving) {
    this->isMoving = isNowMoving;
    if (!isBusy())
      setIdleOrRunningState();
  }
}

void Horse::updateAngle() {}

void Horse::updateAnimations() {
  if (idleAnimation.has_value())
    idleAnimation->update();

  if (runningAnimation.has_value())
    runningAnimation->update();

  if (jumpingAnimation.has_value()) {
    jumpingAnimation->update();
    if (jumpingAnimation->done())
      setIdleOrRunningState();

    if (jumpFrame < JUMP_Y_OFFSET.size() && fakeJump)
      topLeftPosition.set_y(topLeftPosition.y() + JUMP_Y_OFFSET[jumpFrame] * 2);
    jumpFrame++;
  }

  if (hurtAnimation.has_value()) {
    hurtAnimation->update();
    if (hurtAnimation->done())
      setIdleOrRunningState();

    if (hurtFrame < Math::SCALE_STEPS.size() && !customScale) {
      bn::fixed scale =
          Math::SCALE_STEPS[Math::SCALE_STEPS.size() - 1 - hurtFrame];
      mainSprite.set_scale(scale);
      mainSprite.set_rotation_angle(Math::normalizeAngle(
          mainSprite.rotation_angle() +
          bn::fixed(1.5) * (mainSprite.horizontal_flip() ? 1 : -1)));
    }
    hurtFrame++;
  }
}

void Horse::setIdleOrRunningState() {
  if (isMoving)
    setRunningState();
  else
    setIdleState();
}

void Horse::setIdleState() {
  resetAnimations();
  idleAnimation = bn::create_sprite_animate_action_forever(
      mainSprite, 5, bn::sprite_items::start_3dhorse.tiles_item(), 8, 9);
}

void Horse::setRunningState() {
  resetAnimations();

  if (useAlternativeRunAnimation) {
    runningAnimation = bn::create_sprite_animate_action_forever(
        mainSprite, 3, bn::sprite_items::start_3dhorse.tiles_item(), 14, 15, 16,
        17, 18, 19, 20, 21);
  } else {
    runningAnimation = bn::create_sprite_animate_action_forever(
        mainSprite, 3, bn::sprite_items::start_3dhorse.tiles_item(), 0, 1, 2, 3,
        4, 5, 6, 7);
  }
}

void Horse::setJumpingState() {
  resetAnimations();
  jumpingAnimation = bn::create_sprite_animate_action_once(
      mainSprite, 5, bn::sprite_items::start_3dhorse.tiles_item(), 10, 11, 12,
      12);
  jumpFrame = 0;
}

void Horse::setHurtState() {
  if (disappearPosition.has_value())
    return;

  resetAnimations();
  hurtAnimation = bn::create_sprite_animate_action_once(
      mainSprite, 2, bn::sprite_items::start_3dhorse.tiles_item(), 13, 10, 13,
      10, 13, 10, 13, 10);
  hurtFrame = 0;
  hurtCooldown = 90;
}

void Horse::resetAnimations() {
  if (!isGhost)
    mainSprite.set_visible(true);
  if (!customScale) {
    mainSprite.set_scale(1.0);
    mainSprite.set_rotation_angle(0.0);
  }

  idleAnimation.reset();
  runningAnimation.reset();
  jumpingAnimation.reset();
  hurtAnimation.reset();
}
