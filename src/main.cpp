// Copyright (c) 2025 misetteichan
// Licensed under the MIT License. See LICENSE file for details.

#include <M5Unified.h>
#include <Avatar.h>
#include <unit_rolleri2c.hpp>

static UnitRollerI2C roller;
int32_t origin = 0;
m5avatar::Avatar avatar;

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
  origin = roller.getCurrentReadback();

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

void track_angle() {
  auto curr = -(origin - roller.getPosReadback()) / 100.f;
  curr = fmod(curr, 360.0f); 
  if (curr < 0) {
    curr += 360.f;
  }
  avatar.setRotation(curr);
}

void loop() {
  track_angle();
}
