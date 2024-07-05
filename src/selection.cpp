#include "selection.h"

void ActiveSelection::on_click(int x, int y) noexcept {
  click_pos = Vec2{.x = x, .y = y};
  on_move(x, y);
}

void ActiveSelection::on_move(int x, int y) noexcept {
  current_pos = Vec2{x, y};
}

void ActiveSelection::on_release(int x, int y) noexcept {
  release_pos = Vec2{.x = x, .y = y};
}

Selection ActiveSelection::selection() const noexcept {
  return Selection{dimensions(), origin()};
}

Selection ActiveSelection::current_selection() const noexcept {
  const auto [start_x, start_y] = click_pos.value();
  const auto [end_x, end_y] = current_pos;
  int width = abs(end_x - start_x);
  int height = abs(end_y - start_y);
  const auto dimensions = Vec2{.x = width, .y = height};
  const auto origin =
      Vec2{.x = std::min(start_x, end_x), .y = std::min(start_y, end_y)};
  return Selection {dimensions, origin};
}

Vec2 ActiveSelection::dimensions() const noexcept {
  if (!click_pos || !release_pos) {
    std::cerr << "[FATAL]: Begin or end positions not set." << std::endl;
    exit(1);
  }
  const auto [start_x, start_y] = click_pos.value();
  const auto [end_x, end_y] = release_pos.value();
  int width = abs(end_x - start_x);
  int height = abs(end_y - start_y);

  return Vec2{.x = width, .y = height};
}

Vec2 ActiveSelection::origin() const noexcept {
  if (!click_pos || !release_pos) {
    std::cerr << "[FATAL]: Begin or end positions not set." << std::endl;
    exit(1);
  }
  const auto [start_x, start_y] = click_pos.value();
  const auto [end_x, end_y] = release_pos.value();
  return Vec2{.x = std::min(start_x, end_x), .y = std::min(start_y, end_y)};
}