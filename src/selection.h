#pragma once
#include <optional>
#include <iostream>

struct Vec2 {
  int x, y;
};

struct Selection {
  Vec2 dimensions;
  Vec2 origin;
};

struct ActiveSelection {
  std::optional<Vec2> click_pos{};
  Vec2 current_pos{};
  std::optional<Vec2> release_pos{};

  auto constexpr selecting() const noexcept -> bool {
    return click_pos.has_value() && !release_pos.has_value();
  }

  auto on_click(int x, int y) noexcept -> void;
  auto on_move(int x, int y) noexcept -> void;
  auto on_release(int x, int y) noexcept -> void;
  auto selection() const noexcept -> Selection;
  auto current_selection() const noexcept -> Selection;

private:
  auto dimensions() const noexcept -> Vec2;
  auto origin() const noexcept -> Vec2;
};