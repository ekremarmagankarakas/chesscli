#pragma once

#include <memory>

#include "board.h"
#include "view.h"

class Game {
 public:
  explicit Game(std::unique_ptr<View> view);
  void Play();

 private:
  Board board_;
  std::unique_ptr<View> view_;
  bool is_game_over_ = false;
};
