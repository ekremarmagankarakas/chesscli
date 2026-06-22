#include "tui_app.h"

#include <algorithm>
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "board.h"
#include "command.h"
#include "config.h"
#include "game_result.h"
#include "move.h"
#include "parser.h"
#include "piece.h"
#include "square.h"

namespace chess {

namespace {

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

// ---- Braille pixel art for pieces ----
//
// Each piece is drawn on an 8-wide × 16-tall pixel grid. Braille block
// (U+2800-U+28FF) packs a 2×4 pixel sub-cell into one glyph, so the grid
// maps to 4 chars wide × 4 chars tall — four text lines that sit inside
// the cell's vbox.

constexpr int kArtRows = 16;
constexpr int kArtBraille = 4;  // braille chars per side (4×4 grid)

struct PieceArt {
  const char* rows[kArtRows];
};

constexpr PieceArt kKingArt = {{
    "...##...",
    "...##...",
    ".######.",
    ".######.",
    "##....##",
    "########",
    ".######.",
    "...##...",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
}};

constexpr PieceArt kQueenArt = {{
    "##.##.##",
    "########",
    "########",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
}};

constexpr PieceArt kRookArt = {{
    "##.##.##",
    "##.##.##",
    "########",
    "########",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
    "########",
    "########",
}};

constexpr PieceArt kBishopArt = {{
    "...##...",
    "...##...",
    "..####..",
    ".######.",
    ".######.",
    ".######.",
    "##.##.##",
    "########",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
}};

constexpr PieceArt kKnightArt = {{
    ".######.",
    "########",
    "########",
    "##.#####",
    "########",
    "########",
    "########",
    "...#####",
    "..######",
    "..######",
    "..######",
    ".#######",
    "########",
    "########",
    "########",
    "########",
}};

constexpr PieceArt kPawnArt = {{
    "........",
    "...##...",
    "..####..",
    ".######.",
    ".######.",
    "..####..",
    "...##...",
    "..####..",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    ".######.",
    "########",
    "########",
}};

// UTF-8 encoding of braille codepoint 0x2800 + mask (mask is 0-255).
std::string BrailleChar(uint8_t mask) {
  std::string s;
  s += static_cast<char>(0xE2);
  s += static_cast<char>(0xA0 + (mask >> 6));
  s += static_cast<char>(0x80 + (mask & 0x3F));
  return s;
}

// Render a 16×8 art grid to four lines of 4 braille glyphs each.
std::vector<std::string> RenderArt(const PieceArt& art) {
  std::vector<std::string> lines;
  lines.reserve(kArtBraille);
  for (int row_block = 0; row_block < kArtBraille; ++row_block) {
    std::string line;
    for (int col_block = 0; col_block < kArtBraille; ++col_block) {
      int r = row_block * 4;
      int c = col_block * 2;
      uint8_t mask = 0;
      if (art.rows[r + 0][c + 0] == '#') {
        mask |= 0x01;
      }
      if (art.rows[r + 1][c + 0] == '#') {
        mask |= 0x02;
      }
      if (art.rows[r + 2][c + 0] == '#') {
        mask |= 0x04;
      }
      if (art.rows[r + 3][c + 0] == '#') {
        mask |= 0x40;
      }
      if (art.rows[r + 0][c + 1] == '#') {
        mask |= 0x08;
      }
      if (art.rows[r + 1][c + 1] == '#') {
        mask |= 0x10;
      }
      if (art.rows[r + 2][c + 1] == '#') {
        mask |= 0x20;
      }
      if (art.rows[r + 3][c + 1] == '#') {
        mask |= 0x80;
      }
      line += BrailleChar(mask);
    }
    lines.push_back(line);
  }
  return lines;
}

std::vector<std::string> ArtForPiece(const Piece* p) {
  if (!p) {
    std::string blank;
    for (int i = 0; i < kArtBraille; ++i) {
      blank += BrailleChar(0);
    }
    return std::vector<std::string>(kArtBraille, blank);
  }
  switch (p->GetType()) {
    case PieceType::kKing:
      return RenderArt(kKingArt);
    case PieceType::kQueen:
      return RenderArt(kQueenArt);
    case PieceType::kRook:
      return RenderArt(kRookArt);
    case PieceType::kBishop:
      return RenderArt(kBishopArt);
    case PieceType::kKnight:
      return RenderArt(kKnightArt);
    case PieceType::kPawn:
      return RenderArt(kPawnArt);
  }
  return {};
}

const char* ResultLabel(GameResult r) {
  switch (r) {
    case GameResult::kWhiteWins:
      return "White wins";
    case GameResult::kBlackWins:
      return "Black wins";
    case GameResult::kStalemateDraw:
      return "Draw (stalemate)";
    case GameResult::kFiftyMoveDraw:
      return "Draw (50-move rule)";
    case GameResult::kThreefoldDraw:
      return "Draw (threefold repetition)";
    case GameResult::kInsufficientMaterialDraw:
      return "Draw (insufficient material)";
  }
  return "";
}

std::string ParseErrorLabel(ParseError e, const std::string& raw) {
  switch (e) {
    case ParseError::kEmpty:
      return "Empty input";
    case ParseError::kBadSyntax:
      return "Bad input: " + raw;
    case ParseError::kBadSquare:
      return "Unknown square: " + raw;
    case ParseError::kBadPromotion:
      return "Unknown promotion piece: " + raw;
  }
  return "";
}

}  // namespace

struct TuiApp::Impl {
  Board board;
  std::unique_ptr<Engine> engine;
  Color engine_side;
  bool game_over = false;
  std::optional<GameResult> result;

  int cursor_row = 1;  // start on white's second rank
  int cursor_col = 4;  // e-file

  // Selected origin square for a pending move; nullopt = none.
  std::optional<Square> selected;

  std::string text_input;
  std::string status =
      "arrows: cursor  enter: select/move  esc: cancel  "
      "tab: focus  type: e2e4 / Nf3 / O-O / undo / reset / resign / quit";
  std::vector<std::string> log;

  bool exit_requested = false;

  void AppendLog(std::string s) {
    log.push_back(std::move(s));
    while (log.size() > 200) {
      log.erase(log.begin());
    }
  }

  bool IsLastMoveSquare(int r, int c) const {
    const auto& hist = board.History();
    if (hist.empty()) {
      return false;
    }
    const Move& m = hist.back().move;
    if (m.from.row == r && m.from.col == c) {
      return true;
    }
    if (m.to.row == r && m.to.col == c) {
      return true;
    }
    return false;
  }

  bool IsSelected(int r, int c) const {
    return selected && selected->row == r && selected->col == c;
  }

  // Squares the selected piece can legally move to.
  std::vector<Square> CandidateLandings() {
    std::vector<Square> out;
    if (!selected) {
      return out;
    }
    for (const auto& m : board.LegalMoves()) {
      if (m.from == *selected &&
          std::find(out.begin(), out.end(), m.to) == out.end()) {
        out.push_back(m.to);
      }
    }
    return out;
  }

  bool IsCandidate(int r, int c) {
    if (!selected) {
      return false;
    }
    for (const auto& s : CandidateLandings()) {
      if (s.row == r && s.col == c) {
        return true;
      }
    }
    return false;
  }

  void SelectOrMove(Square cursor) {
    if (game_over) {
      status = "Game is over. 'reset' to play again.";
      return;
    }

    if (!selected) {
      const Piece* p = board.At(cursor.row, cursor.col);
      if (!p) {
        status = "No piece on " + cursor.ToAlgebraic();
        return;
      }
      if (p->GetColor() != board.SideToMove()) {
        status = "Not your piece";
        return;
      }
      // Pre-check: any legal move from this square?
      bool has_move = false;
      for (const auto& m : board.LegalMoves()) {
        if (m.from == cursor) {
          has_move = true;
          break;
        }
      }
      if (!has_move) {
        status = "No legal moves for that piece";
        return;
      }
      selected = cursor;
      status = "selected " + cursor.ToAlgebraic() + " — pick destination";
      return;
    }

    // Selection re-click on same square cancels.
    if (selected->row == cursor.row && selected->col == cursor.col) {
      selected.reset();
      status = "selection cleared";
      return;
    }

    // Try the move. Promotion defaults to Queen if pawn reaches last rank.
    const Piece* p = board.At(selected->row, selected->col);
    std::optional<PieceType> promo;
    if (p && p->GetType() == PieceType::kPawn) {
      int last_rank = p->GetColor() == Color::kWhite ? 7 : 0;
      if (cursor.row == last_rank) {
        promo = PieceType::kQueen;
      }
    }
    Move m{*selected, cursor, promo};
    if (!board.IsLegal(m)) {
      status = "illegal move " + selected->ToAlgebraic() + cursor.ToAlgebraic();
      return;
    }
    CommitMove(m);
  }

  void CommitMove(const Move& m) {
    const char* mover = board.SideToMove() == Color::kWhite ? "W" : "B";
    board.Apply(m);
    AppendLog(std::string(mover) + ": " + MoveToUCI(m));
    selected.reset();
    status = std::string(mover) + " played " + MoveToUCI(m);
    if (auto r = board.Result()) {
      result = *r;
      game_over = true;
      AppendLog(ResultLabel(*r));
      status = ResultLabel(*r);
      return;
    }
    if (engine && board.SideToMove() == engine_side) {
      RunEngine();
    }
  }

  void RunEngine() {
    if (game_over || !engine) {
      return;
    }
    const char* mover = board.SideToMove() == Color::kWhite ? "W" : "B";
    Move em = engine->Choose(board);
    board.Apply(em);
    AppendLog(std::string("Engine ") + mover + ": " + MoveToUCI(em));
    status = std::string("Engine (") + mover + ") played " + MoveToUCI(em);
    if (auto r = board.Result()) {
      result = *r;
      game_over = true;
      AppendLog(ResultLabel(*r));
      status = ResultLabel(*r);
    }
  }

  void ProcessTextCommand(const std::string& raw) {
    if (raw.empty()) {
      return;
    }
    Command cmd = Parse(raw, board);
    std::visit(
        Overloaded{
            [&](const Move& m) {
              if (game_over) {
                status = "Game is over. 'reset' to play again.";
                return;
              }
              if (!board.IsLegal(m)) {
                status = "Illegal: " + raw;
                return;
              }
              selected.reset();
              CommitMove(m);
            },
            [&](QuitCmd) { exit_requested = true; },
            [&](UndoCmd) {
              board.Undo();
              game_over = false;
              result.reset();
              selected.reset();
              AppendLog("undo");
              status = "undone";
            },
            [&](ResetCmd) {
              board.Reset();
              game_over = false;
              result.reset();
              selected.reset();
              log.clear();
              AppendLog("reset");
              status = "reset";
            },
            [&](HistoryCmd) { status = "history on the right panel"; },
            [&](HelpCmd) {
              status =
                  "arrows: cursor  enter: select/move  esc: cancel  "
                  "type: e2e4 / Nf3 / O-O / undo / reset / resign / quit";
            },
            [&](ResignCmd) {
              result = board.HandleResign();
              game_over = true;
              AppendLog(std::string("resigned: ") + ResultLabel(*result));
              status = ResultLabel(*result);
            },
            [&](ParseError e) { status = ParseErrorLabel(e, raw); },
        },
        cmd);
  }
};

namespace {

ftxui::Element RenderCell(TuiApp::Impl& s, int row, int col) {
  using namespace ftxui;

  bool is_cursor = (s.cursor_row == row && s.cursor_col == col);
  bool is_sel = s.IsSelected(row, col);
  // IsCandidate -> CandidateLandings -> Board::LegalMoves -> Board::IsLegal
  // -> Board::Restore re-allocates every Piece on the board, invalidating
  // any Piece* captured beforehand. Read the piece pointer after this call.
  bool is_cand = s.IsCandidate(row, col);
  bool is_last = s.IsLastMoveSquare(row, col);
  bool light = (row + col) % 2 == 1;

  const Piece* p = s.board.At(row, col);

  // All backgrounds chosen at mid-luminance so neither pure-white nor
  // pure-black pieces wash out. Pieces are uniform per side.
  ftxui::Color bg;
  if (is_cursor) {
    bg = ftxui::Color::RGB(80, 130, 180);  // muted steel blue
  } else if (is_sel) {
    bg = ftxui::Color::RGB(95, 145, 95);  // muted green
  } else if (is_cand) {
    bg = ftxui::Color::RGB(110, 160, 155);  // muted teal
  } else if (is_last) {
    bg = ftxui::Color::RGB(180, 145, 70);  // muted amber
  } else if (light) {
    bg = ftxui::Color::RGB(170, 155, 130);  // warm light tan
  } else {
    bg = ftxui::Color::RGB(110, 90, 75);  // warm dark brown
  }

  ftxui::Color fg = ftxui::Color::Default;
  if (p) {
    fg = (p->GetColor() == Color::kWhite) ? ftxui::Color::White
                                          : ftxui::Color::Black;
  }

  auto art = ArtForPiece(p);
  std::vector<ftxui::Element> lines;
  lines.reserve(art.size());
  for (const auto& l : art) {
    lines.push_back(text(l) | color(fg) | bold);
  }
  // Wrap the art in a flex column with filler above and below so the piece
  // sits in the vertical middle of the cell (cells stretch to fill the
  // board's flex space).
  auto art_block = vbox(lines) | hcenter;
  return vbox({
             filler(),
             art_block,
             filler(),
         }) |
         bgcolor(bg) | flex;
}

ftxui::Element RenderBoardElement(TuiApp::Impl& s) {
  using namespace ftxui;

  std::vector<std::vector<Element>> grid;
  for (int row = 7; row >= 0; --row) {
    std::vector<Element> row_cells;
    for (int col = 0; col < 8; ++col) {
      row_cells.push_back(RenderCell(s, row, col));
    }
    grid.push_back(row_cells);
  }
  auto board_grid = gridbox(grid) | flex;

  auto file_row = [] {
    std::vector<Element> cells;
    for (int c = 0; c < 8; ++c) {
      std::string lab(1, char('a' + c));
      cells.push_back(text(lab) | center | dim | flex);
    }
    return hbox({text("  "), hbox(cells) | flex});
  };

  std::vector<Element> rank_cells;
  for (int row = 7; row >= 0; --row) {
    rank_cells.push_back(text(std::to_string(row + 1)) | center | dim | yflex);
  }
  auto rank_col = vbox(rank_cells) | size(WIDTH, EQUAL, 2);

  auto middle = hbox({rank_col, board_grid}) | yflex;

  return vbox({file_row(), middle, file_row()}) | flex | border;
}

ftxui::Element RenderSidePanel(const TuiApp::Impl& s) {
  using namespace ftxui;

  std::string stm;
  if (s.game_over && s.result) {
    stm = std::string("Game over: ") + ResultLabel(*s.result);
  } else {
    stm = "To move: ";
    stm += (s.board.SideToMove() == Color::kWhite) ? "White ♔" : "Black ♚";
  }

  std::vector<Element> hist;
  hist.push_back(text("History") | bold | underlined);
  const auto& h = s.board.History();
  if (h.empty()) {
    hist.push_back(text("(no moves yet)") | dim);
  } else {
    for (std::size_t i = 0; i < h.size(); ++i) {
      std::string line;
      if (i % 2 == 0) {
        line = std::to_string(i / 2 + 1) + ". ";
      } else {
        line = "   ... ";
      }
      line += MoveToUCI(h[i].move);
      hist.push_back(text(line));
    }
  }

  std::vector<Element> log_lines;
  log_lines.push_back(text("Log") | bold | underlined);
  int start = std::max(0, static_cast<int>(s.log.size()) - 8);
  for (int i = start; i < static_cast<int>(s.log.size()); ++i) {
    log_lines.push_back(text(s.log[i]) | dim);
  }
  if (log_lines.size() == 1) {
    log_lines.push_back(text("(empty)") | dim);
  }

  return vbox({
             text("chess") | bold | center,
             separator(),
             text(stm),
             separator(),
             vbox(hist) | yflex,
             separator(),
             vbox(log_lines),
         }) |
         flex;
}

}  // namespace

TuiApp::TuiApp(std::unique_ptr<Engine> engine, Color engine_side)
    : impl_(std::make_unique<Impl>()) {
  impl_->engine = std::move(engine);
  impl_->engine_side = engine_side;
  // If engine plays White and game just started, let it move first.
  if (impl_->engine && impl_->board.SideToMove() == impl_->engine_side) {
    impl_->RunEngine();
  }
}

TuiApp::~TuiApp() = default;

void TuiApp::Run() {
  using namespace ftxui;
  auto screen = ScreenInteractive::Fullscreen();

  auto input = Input(&impl_->text_input, "type a command + Enter");

  auto root = CatchEvent(input, [this, &screen](Event e) {
    if (e == Event::ArrowUp) {
      impl_->cursor_row = std::min(7, impl_->cursor_row + 1);
      return true;
    }
    if (e == Event::ArrowDown) {
      impl_->cursor_row = std::max(0, impl_->cursor_row - 1);
      return true;
    }
    if (e == Event::ArrowLeft) {
      impl_->cursor_col = std::max(0, impl_->cursor_col - 1);
      return true;
    }
    if (e == Event::ArrowRight) {
      impl_->cursor_col = std::min(7, impl_->cursor_col + 1);
      return true;
    }
    if (e == Event::Escape) {
      if (impl_->selected.has_value()) {
        impl_->selected.reset();
        impl_->status = "selection cleared";
      } else if (!impl_->text_input.empty()) {
        impl_->text_input.clear();
        impl_->status = "input cleared";
      } else {
        screen.ExitLoopClosure()();
      }
      return true;
    }
    if (e == Event::Return) {
      // Handle Return ourselves so text commands always fire, regardless of
      // the Input component's focus or on_enter wiring.
      if (impl_->text_input.empty()) {
        impl_->SelectOrMove(Square{impl_->cursor_row, impl_->cursor_col});
        return true;
      }
      std::string cmd = impl_->text_input;
      impl_->text_input.clear();
      if (cmd == "quit" || cmd == "exit") {
        screen.ExitLoopClosure()();
        return true;
      }
      impl_->ProcessTextCommand(cmd);
      if (impl_->exit_requested) {
        screen.ExitLoopClosure()();
      }
      return true;
    }
    return false;
  });

  constexpr int kSidePanelWidth = 36;
  constexpr int kStatusHeight = 3;
  constexpr int kInputHeight = 3;

  auto renderer = Renderer(root, [this, input] {
    auto board_el = RenderBoardElement(*impl_) | flex;
    auto status_el = hbox({text("● ") | color(ftxui::Color::Yellow),
                           text(impl_->status) | ftxui::xflex_shrink}) |
                     border | size(HEIGHT, EQUAL, kStatusHeight);
    auto input_el = hbox({text(" > ") | bold | color(ftxui::Color::Cyan),
                          input->Render() | ftxui::xflex}) |
                    border | size(HEIGHT, EQUAL, kInputHeight);
    auto left = vbox({board_el, status_el, input_el}) | flex;
    auto right = (RenderSidePanel(*impl_) | border) |
                 size(WIDTH, EQUAL, kSidePanelWidth);
    return hbox({left, right}) | flex;
  });

  screen.Loop(renderer);
}

void RunConfigureScreen(const std::string& config_path_str) {
  using namespace ftxui;

  std::filesystem::path config_path(config_path_str);
  Config cfg = LoadConfig(config_path);

  bool tui_on = cfg.tui;
  bool play_black = cfg.play_black;
  int engine_depth = cfg.engine_depth;
  std::string status_msg = "Config: " + config_path_str;

  // 0 = Unicode, 1 = Text
  int view_selected = cfg.unicode ? 0 : 1;
  std::vector<std::string> view_entries = {"Unicode (♔♕♖)", "Text (K Q R)"};

  auto screen = ScreenInteractive::Fullscreen();

  auto tui_cb = Checkbox("Default to TUI mode", &tui_on);
  auto black_cb = Checkbox("Play as Black (engine plays White)", &play_black);
  auto view_radio = Radiobox(&view_entries, &view_selected);
  auto depth_dec = Button(" − ", [&] {
    if (engine_depth > 0) {
      --engine_depth;
    }
  });
  auto depth_inc = Button(" + ", [&] {
    if (engine_depth < 10) {
      ++engine_depth;
    }
  });

  auto save_btn = Button("Save & Exit", [&] {
    Config out;
    out.tui = tui_on;
    out.unicode = (view_selected == 0);
    out.play_black = play_black;
    out.engine_depth = engine_depth;
    try {
      SaveConfig(out, config_path);
    } catch (...) {
      status_msg = "Error: could not write " + config_path_str;
      return;
    }
    screen.ExitLoopClosure()();
  });

  auto cancel_btn = Button("Cancel", [&] { screen.ExitLoopClosure()(); });

  auto container = Container::Vertical({
      tui_cb,
      view_radio,
      black_cb,
      Container::Horizontal({depth_dec, depth_inc}),
      Container::Horizontal({save_btn, cancel_btn}),
  });

  auto renderer = Renderer(container, [&] {
    std::string depth_label =
        engine_depth == 0 ? "disabled" : std::to_string(engine_depth);
    return vbox({
               text("chesscli — Configuration") | bold | center,
               separator(),
               tui_cb->Render() | xflex,
               separator(),
               text("CLI view (when TUI is off)") | dim,
               view_radio->Render(),
               separator(),
               black_cb->Render() | xflex,
               hbox({text("Engine depth: "), depth_dec->Render(),
                     text(" " + depth_label + " ") | bold | center |
                         size(WIDTH, EQUAL, 10),
                     depth_inc->Render()}),
               separator(),
               hbox({save_btn->Render(), text("  "), cancel_btn->Render()}) |
                   center,
               separator(),
               text(status_msg) | dim | center,
           }) |
           border | center;
  });

  screen.Loop(renderer);
}

}  // namespace chess
