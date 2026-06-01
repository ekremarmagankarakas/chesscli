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
    if (!std::getline(std::cin, input)) {
      break;
    }
    if (input == "quit" || input == "exit") {
      break;
    }

    Parser parser(input);
    std::optional<Move> move = parser.parse();
    if (!move) {
      std::cout << "Bad input " << input << std::endl;
      continue;
    }
    if (!board_.IsLegal(*move)) {
      std::cout << "Illegal move " << input << std::endl;
      continue;
    }

    board_.Apply(*move);

    if (board_.IsCheckmate()) {
      Color opponent =
          board_.ToMove() == Color::kWhite ? Color::kBlack : Color::kWhite;
      if (opponent == Color::kWhite) {
        std::cout << "WHITE WON!" << std::endl;
      } else {
        std::cout << "BLACK WON!" << std::endl;
      }
      is_game_over_ = true;
    } else if (board_.IsStalemate()) {
      std::cout << "STALEMATE!" << std::endl;
      is_game_over_ = true;
    }
  }
}
