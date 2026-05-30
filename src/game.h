#pragma once

#include "board.h"

class Game {
 public:
  Game();
  void Play();

 private:
  Board board_ = Board();
  bool is_game_over_ = false;
};
