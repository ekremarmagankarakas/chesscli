#include "game.h"

#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "command.h"
#include "move.h"
#include "parser.h"

namespace {
template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

const char* kInGameHelp =
    "Commands:\n"
    "  <from><to>[promo]  Make a move (e.g. e2e4 or e7e8q)\n"
    "  undo               Undo last move\n"
    "  reset              Reset game to starting position\n"
    "  history            Show move history\n"
    "  resign             Resign current side\n"
    "  help               Show this help\n"
    "  quit, exit         Exit";
}  // namespace

Game::Game(std::unique_ptr<View> view,
           std::unique_ptr<InputSource> input_source)
    : view_(std::move(view)), input_source_(std::move(input_source)) {}

void Game::Play() {
  while (!quit_) {
    if (!is_game_over_) {
      view_->Render(board_);
    }
    std::optional<std::string> raw_input = input_source_->ReadLine();
    if (!raw_input) {
      break;
    }
    const std::string& raw = *raw_input;
    Command cmd = Parse(raw);
    std::visit(
        Overloaded{
            [this, &raw](const Move& m) {
              if (is_game_over_) {
                view_->ShowMessage("Game over. Type 'reset' or 'quit'.");
                return;
              }
              HandleMove(m, raw);
            },
            [this](QuitCmd) {
              is_game_over_ = true;
              quit_ = true;
            },
            [this](UndoCmd) { board_.Undo(); },
            [this](ResetCmd) {
              board_.Reset();
              is_game_over_ = false;
              view_->ShowMessage("Game Reset");
            },
            [this](HistoryCmd) { view_->ShowHistory(board_); },
            [this](HelpCmd) { view_->ShowMessage(kInGameHelp); },
            [this](ResignCmd) {
              view_->ShowResult(board_.HandleResign());
              is_game_over_ = true;
            },
            [this, &raw](ParseError e) { view_->ShowParseError(e, raw); },
        },
        cmd);
  }
}

void Game::HandleMove(const Move& move, std::string_view raw) {
  if (!board_.IsLegal(move)) {
    view_->ShowIllegalMove(raw);
    return;
  }
  board_.Apply(move);

  if (auto result = board_.Result()) {
    view_->ShowResult(*result);
    is_game_over_ = true;
  }
}
