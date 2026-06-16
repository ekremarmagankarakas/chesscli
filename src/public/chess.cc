#include "chess.h"

#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "board.h"
#include "command.h"
#include "engine.h"
#include "game_result.h"
#include "minimax_engine.h"
#include "move.h"
#include "parse_error.h"
#include "parser.h"
#include "piece.h"
#include "text_view.h"
#include "unicode_view.h"
#include "view.h"

namespace chess {

namespace {

Side ToPublicSide(Color c) {
  return c == Color::kWhite ? Side::kWhite : Side::kBlack;
}

Color ToInternalColor(Side s) {
  return s == Side::kWhite ? Color::kWhite : Color::kBlack;
}

Result ToPublicResult(GameResult r) {
  switch (r) {
    case GameResult::kWhiteWins:
      return Result::WhiteWins;
    case GameResult::kBlackWins:
      return Result::BlackWins;
    case GameResult::kStalemateDraw:
    case GameResult::kFiftyMoveDraw:
    case GameResult::kThreefoldDraw:
    case GameResult::kInsufficientMaterialDraw:
      return Result::Draw;
  }
  return Result::Ongoing;
}

std::unique_ptr<View> MakeView(ViewMode mode) {
  if (mode == ViewMode::Unicode) {
    return std::make_unique<UnicodeView>();
  }
  return std::make_unique<TextView>();
}

const char* kHelpText =
    "Commands:\n"
    "  <from><to>[promo]  Make a move (e.g. e2e4 or e7e8q)\n"
    "  undo               Undo last ply\n"
    "  reset              Reset to starting position\n"
    "  history            Show move history\n"
    "  resign             Resign current side";

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}  // namespace

struct Session::Impl {
  Config cfg;
  Board board;
  std::unique_ptr<View> view;
  std::unique_ptr<Engine> engine;
  std::vector<std::string> history;  // UCI strings, newest last
  bool game_over = false;
  Result outcome = Result::Ongoing;
};

Session::Session(Config cfg) : impl_(std::make_unique<Impl>()) {
  impl_->cfg = cfg;
  impl_->view = MakeView(cfg.view);
  if (cfg.engine_depth > 0) {
    impl_->engine = std::make_unique<MinimaxEngine>(cfg.engine_depth);
  }
}

Session::~Session() = default;
Session::Session(Session&&) noexcept = default;
Session& Session::operator=(Session&&) noexcept = default;

std::string Session::Render() const {
  return impl_->view->RenderToString(impl_->board, impl_->game_over);
}

bool Session::IsOver() const { return impl_->game_over; }

Result Session::Outcome() const { return impl_->outcome; }

Side Session::ToMove() const { return ToPublicSide(impl_->board.SideToMove()); }

std::vector<std::string> Session::History() const { return impl_->history; }

CommandResult Session::Apply(std::string_view input) {
  Command cmd = Parse(input, impl_->board);
  CommandResult result{true, ""};

  std::visit(
      Overloaded{
          [&](const Move& m) {
            if (impl_->game_over) {
              result = {false, "Game is over"};
              return;
            }
            if (!impl_->board.IsLegal(m)) {
              result = {false, "Illegal move"};
              return;
            }
            impl_->board.Apply(m);
            impl_->history.push_back(MoveToUCI(m));
            if (auto r = impl_->board.Result()) {
              impl_->game_over = true;
              impl_->outcome = ToPublicResult(*r);
              return;
            }
            // Engine reply if configured and now its turn.
            if (impl_->engine && impl_->board.SideToMove() ==
                                     ToInternalColor(impl_->cfg.engine_side)) {
              Move em = impl_->engine->Choose(impl_->board);
              impl_->board.Apply(em);
              impl_->history.push_back(MoveToUCI(em));
              result.message = "Engine plays " + MoveToUCI(em);
              if (auto r = impl_->board.Result()) {
                impl_->game_over = true;
                impl_->outcome = ToPublicResult(*r);
              }
            }
          },
          [&](QuitCmd) {
            result = {false, "quit not supported via library API"};
          },
          [&](UndoCmd) {
            impl_->board.Undo();
            if (!impl_->history.empty()) {
              impl_->history.pop_back();
            }
            // Undo can revive an over game.
            impl_->game_over = false;
            impl_->outcome = Result::Ongoing;
            result.message = "Undone";
          },
          [&](ResetCmd) {
            impl_->board.Reset();
            impl_->history.clear();
            impl_->game_over = false;
            impl_->outcome = Result::Ongoing;
            result.message = "Reset";
          },
          [&](HistoryCmd) {
            std::ostringstream oss;
            for (const auto& m : impl_->history) {
              oss << m << '\n';
            }
            result.message = oss.str();
          },
          [&](HelpCmd) { result.message = kHelpText; },
          [&](ResignCmd) {
            impl_->game_over = true;
            impl_->outcome = ToPublicResult(impl_->board.HandleResign());
            result.message = "Resigned";
          },
          [&](ParseError e) {
            result.ok = false;
            switch (e) {
              case ParseError::kEmpty:
                result.message = "Empty input";
                break;
              case ParseError::kBadSyntax:
                result.message = "Bad syntax";
                break;
              case ParseError::kBadSquare:
                result.message = "Bad square";
                break;
              case ParseError::kBadPromotion:
                result.message = "Bad promotion";
                break;
            }
          },
      },
      cmd);

  return result;
}

}  // namespace chess
