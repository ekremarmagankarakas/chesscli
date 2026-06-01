#include "game.h"

#include <optional>
#include <string>

#include "move.h"
#include "parser.h"

Game::Game(std::unique_ptr<View> view,
           std::unique_ptr<InputSource> input_source)
    : view_(std::move(view)), input_source_(std::move(input_source)) {}

void Game::Play() {
  while (!is_game_over_) {
    view_->Render(board_);
    std::optional<std::string> raw_input = input_source_->ReadLine();
    if (!raw_input) {
      break;
    }
    const std::string input = *raw_input;
    if (input == "quit" || input == "exit") {
      break;
    }
    if (input == "undo") {
      board_.Undo();
      continue;
    }

    Parser parser(input);
    std::optional<Move> move = parser.parse();
    if (!move) {
      view_->ShowMessage("Bad Input " + input);
      continue;
    }
    if (!board_.IsLegal(*move)) {
      view_->ShowMessage("Illegal Move " + input);
      continue;
    }

    board_.Apply(*move);

    if (board_.IsCheckmate()) {
      Color opponent =
          board_.ToMove() == Color::kWhite ? Color::kBlack : Color::kWhite;
      if (opponent == Color::kWhite) {
        view_->ShowGameOver("WHITE WON!");
      } else {
        view_->ShowGameOver("BLACK WON!");
      }
      is_game_over_ = true;
    } else if (board_.IsStalemate()) {
      view_->ShowGameOver("STALEMATE!");
      is_game_over_ = true;
    }
  }
}
