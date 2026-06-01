#pragma once

#include "view.h"

class TextView : public View {
 public:
  void Render(const Board& board) override;
  void ShowMessage(std::string_view msg) override;
  void ShowGameOver(std::string_view reason) override;
};
