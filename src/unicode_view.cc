#include "unicode_view.h"

#include <iostream>
#include <optional>

#include "board.h"
#include "game_result.h"
#include "piece.h"

namespace {

// Background colors for board squares.
const char* BG_LIGHT = "\033[48;5;187m";      // warm tan
const char* BG_DARK = "\033[48;5;94m";        // brown
const char* BG_HIGHLIGHT = "\033[48;5;221m";  // yellow — last-moved squares

// Foreground colors for pieces.
const char* FG_WHITE = "\033[1;38;5;231m";  // bold bright white
const char* FG_BLACK = "\033[38;5;16m";     // deep black

// Coordinate labels.
const char* FG_LABEL = "\033[38;5;245m";  // dim gray

const char* RESET = "\033[0m";

// Filled glyphs only; foreground color distinguishes side.
// Avoids the outline-vs-filled font rendering inversion problem.
const char* UnicodePiece(const Piece* p) {
  if (!p) {
    return " ";
  }
  switch (p->GetType()) {
    case PieceType::kKing:
      return "♚";
    case PieceType::kQueen:
      return "♛";
    case PieceType::kRook:
      return "♜";
    case PieceType::kBishop:
      return "♝";
    case PieceType::kKnight:
      return "♞";
    case PieceType::kPawn:
      return "♟";
  }
  return "?";
}

bool IsHighlighted(int row, int col, const std::optional<Move>& last) {
  if (!last) {
    return false;
  }
  return (last->from.row == row && last->from.col == col) ||
         (last->to.row == row && last->to.col == col);
}

}  // namespace

void UnicodeView::Render(const Board& board) {
  std::optional<Move> last;
  const auto& history = board.GetHistory();
  if (!history.empty()) {
    last = history.back().move;
  }

  for (int row = Board::kSize - 1; row >= 0; --row) {
    std::cout << FG_LABEL << (row + 1) << RESET << ' ';
    for (int col = 0; col < Board::kSize; ++col) {
      bool light = (row + col) % 2 == 1;
      const char* bg = IsHighlighted(row, col, last)
                           ? BG_HIGHLIGHT
                           : (light ? BG_LIGHT : BG_DARK);
      std::cout << bg;
      const Piece* p = board.At(row, col);
      if (p) {
        std::cout << (p->GetColor() == Color::kWhite ? FG_WHITE : FG_BLACK);
        std::cout << " " << UnicodePiece(p) << " ";
      } else {
        std::cout << "   ";
      }
      std::cout << RESET;
    }
    std::cout << '\n';
  }
  std::cout << FG_LABEL << "   a  b  c  d  e  f  g  h" << RESET << '\n';
}

void UnicodeView::ShowMessage(std::string_view msg) {
  std::cout << msg << '\n';
}

void UnicodeView::ShowHistory(const Board& board) {
  for (const auto& entry : board.GetHistory()) {
    std::cout << MoveToUCI(entry.move) << '\n';
  }
}

void UnicodeView::ShowIllegalMove(std::string_view input) {
  std::cout << "Illegal move: " << input << '\n';
}

void UnicodeView::ShowParseError(ParseError err, std::string_view input) {
  switch (err) {
    case ParseError::kEmpty:
      std::cout << "Empty input.\n";
      return;
    case ParseError::kBadSyntax:
      std::cout << "Bad input: " << input << '\n';
      return;
    case ParseError::kBadSquare:
      std::cout << "Unknown square: " << input << '\n';
      return;
    case ParseError::kBadPromotion:
      std::cout << "Unknown promotion piece: " << input << '\n';
      return;
  }
}

void UnicodeView::ShowResult(GameResult result) {
  switch (result) {
    case GameResult::kWhiteWins:
      std::cout << "WHITE WON!\n";
      return;
    case GameResult::kBlackWins:
      std::cout << "BLACK WON!\n";
      return;
    case GameResult::kFiftyMoveDraw:
      std::cout << "DRAW BY 50-MOVE RULE!\n";
      return;
    case GameResult::kThreefoldDraw:
      std::cout << "DRAW BY THREEFOLD REPETITION!\n";
      return;
    case GameResult::kInsufficientMaterialDraw:
      std::cout << "DRAW BY INSUFFICIENT MATERIAL!\n";
      return;
    case GameResult::kStalemateDraw:
      std::cout << "DRAW BY STALEMATE!\n";
      return;
  }
}
