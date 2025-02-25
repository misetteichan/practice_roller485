// Copyright (c) 2025 misetteichan
// Licensed under the MIT License. See LICENSE file for details.

#include <M5Unified.h>
#include <Avatar.h>
#include <unit_rolleri2c.hpp>

static UnitRollerI2C roller;
int32_t origin, position;
int32_t speed = 500;
m5avatar::Avatar avatar;
float rotation = 0.f;

SemaphoreHandle_t xSemaphore;

void move(float degree) {
  if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
    position = roller.getPos() + static_cast<int32_t>(degree * 100.f);
    xSemaphoreGive(xSemaphore);
  }
}

void apply_angle() {
  if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
    auto curr = -(origin - roller.getPosReadback()) / 100.f;
    xSemaphoreGive(xSemaphore);
    curr = std::fmod(curr, 360.0f); 
    if (curr < 0) {
      curr += 360.f;
    }
    if (rotation != curr) {
      avatar.setRotation(curr);
      rotation = curr;
    }
  }
}

void setup() {
  M5.begin();

  vSemaphoreCreateBinary(xSemaphore);
  xTaskCreate(
    [](void *pvParameter) {
      const auto sda = M5.getPin(m5::pin_name_t::port_a_sda);
      const auto scl = M5.getPin(m5::pin_name_t::port_a_scl);
      while (!roller.begin(&Wire, 0x64, sda, scl, 400000)) {
        M5.delay(1000);
      }
      roller.setMode(ROLLER_MODE_POSITION);
      roller.setPosMaxCurrent(100000);
      roller.setOutput(1);
      origin = position = roller.getPos();

      while(true) {
        if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
          auto curr = roller.getPos();
          if (curr != position) {
            if (abs(position - curr) > speed) {
              const auto dir = position - curr > 0 ? 1 : -1;
              curr += speed * dir;
            } else {
              curr += position - curr;
            }
            roller.setPos(curr);
          }
          xSemaphoreGive(xSemaphore);
        }
        delay(10);
      }
    }, "Roller", 2048, nullptr, 1, nullptr);

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
  apply_angle();
  M5.update();
  if (M5.BtnA.wasClicked()) {
    move(90.f);
  }
  if (M5.BtnB.wasClicked()) {
    move(-90.f);
  }
  if (M5.BtnC.wasClicked()) {
    speed += 100;
  }
}
