#pragma once

#include <string_view>
class Board;  // forward declaration

class View {
 public:
  virtual ~View() = default;
  virtual void Render(const Board& board) = 0;
  virtual void ShowMessage(std::string_view msg) = 0;
  virtual void ShowGameOver(std::string_view reason) = 0;
};
