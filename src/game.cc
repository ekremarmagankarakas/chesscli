#include "game.h"

#include <iostream>
#include <optional>
#include <string>

#include "move.h"
#include "parser.h"

Game::Game() = default;

void Game::Play() {
  while (!is_game_over_) {
    board_.Print();
    std::string input;
    std::getline(std::cin, input);
    std::cout << input << std::endl;

    Parser parser(input);
    auto move = parser.parse();
    if (!move) {
      std::cout << "Bad input " << input << std::endl;
      continue;
    }
    Piece* p = board_.At(move->from);
    if (!p) {
      std::cout << "no piece at " << move->from.ToAlgebraic() << std::endl;
      continue;
    }
    if (p->GetColor() != board_.ToMove()) {
      std::cout << "not your turn" << std::endl;
      continue;
    }
    auto moves = p->ValidMoves(move->from, board_);
    bool ok = false;
    for (const auto& sq : moves) {
      if (sq == move->to) {
        ok = true;
        break;
      }
    }
    if (!ok) {
      std::cout << "illegal move " << input << std::endl;
      continue;
    }
    board_.MovePiece(move->from, move->to);
  }
}
