#pragma once

#include <memory>

#include "board.h"
#include "input_source.h"
#include "view.h"

class Game {
 public:
  explicit Game(std::unique_ptr<View> view,
                std::unique_ptr<InputSource> input_source);
  void Play();

 private:
  Board board_;
  std::unique_ptr<View> view_;
  std::unique_ptr<InputSource> input_source_;
  bool is_game_over_ = false;
};
