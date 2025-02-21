// Copyright (c) 2025 misetteichan
// Licensed under the MIT License. See LICENSE file for details.

#include <M5Unified.h>
#include <Avatar.h>
#include <unit_rolleri2c.hpp>

static UnitRollerI2C roller;
int32_t origin = 0;
m5avatar::Avatar avatar;
int mode = 0;

void apply_angle() {
  auto curr = -(origin - roller.getPosReadback()) / 100.f;
  curr = std::fmod(curr, 360.0f); 
  if (curr < 0) {
    curr += 360.f;
  }
  avatar.setRotation(curr);
}

bool judge_mode() {
  M5.update();
  if (!M5.BtnA.wasClicked()) {
    return false;
  }
  mode = 1 - mode;
  roller.setPos(roller.getPosReadback());

  auto cp = avatar.getColorPalette();
  switch (mode) {
  case 0:
    roller.setOutput(0);
    cp.set(COLOR_PRIMARY, WHITE);
    break;
  case 1:
    roller.setOutput(1);
    cp.set(COLOR_PRIMARY, YELLOW);
    break;
  }
  avatar.setColorPalette(cp);
  return true;
}

bool move(float deg) {
  if (judge_mode()) {
    return false;
  }
  roller.setPos(roller.getPos() + static_cast<int32_t>(deg * 100.f));
  M5.delay(1);
  apply_angle();
  return true;
}

void setup() {
  M5.begin();

  const auto sda = M5.getPin(m5::pin_name_t::port_a_sda);
  const auto scl = M5.getPin(m5::pin_name_t::port_a_scl);
  while (!roller.begin(&Wire, 0x64, sda, scl, 400000)) {
    M5.delay(1000);
  }
  roller.setOutput(0);
  roller.setMode(ROLLER_MODE_POSITION);
  roller.setPosMaxCurrent(100000);
  origin = roller.getPos();

  const auto r = avatar.getFace()->getBoundingRect();
  const auto scale = std::min(
    M5.Display.width() / (float)r->getWidth(),
    M5.Display.height() / (float)r->getHeight());
  avatar.setScale(scale);
  const auto offs_x = (r->getWidth() - M5.Display.width()) / 2;
  const auto offs_y = (r->getHeight() - M5.Display.height()) / 2;
  avatar.setPosition(-offs_y, -offs_x);
  avatar.init();
}

void loop() {
  const auto d = .5f;
  switch (mode) {
  case 0:
    judge_mode();
    apply_angle();
    break;
  case 1:
    while (roller.getPos() < origin + 36000) {
      if (!move(d)) {
        return;
      }
    }
    while (roller.getPos() > origin) {
      if (!move(-d)) {
        return;
      }
    }
    break;
  }
}
