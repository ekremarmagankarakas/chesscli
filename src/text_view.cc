#include "text_view.h"

#include <iostream>
#include <sstream>

#include "board.h"
#include "game_result.h"
#include "piece.h"

namespace chess {

std::string TextView::RenderToString(const Board& board, bool game_over) const {
  std::ostringstream oss;
  for (int row = Board::kSize - 1; row >= 0; --row) {
    oss << (row + 1) << ' ';
    for (int col = 0; col < Board::kSize; ++col) {
      const Piece* piece = board.At(row, col);
      if (piece) {
        oss << piece->GetSymbol() << ' ';
      } else {
        oss << ". ";
      }
    }
    oss << '\n';
  }
  oss << "  a b c d e f g h\n";
  if (!game_over) {
    oss << (board.SideToMove() == Color::kWhite ? "White" : "Black")
        << " to move\n";
  }
  return oss.str();
}

void TextView::Render(const Board& board, bool game_over) {
  std::cout << RenderToString(board, game_over);
}

void TextView::ShowMessage(std::string_view msg) { std::cout << msg << '\n'; }

void TextView::ShowHistory(const Board& board) {
  for (const auto& entry : board.History()) {
    std::cout << MoveToUCI(entry.move) << '\n';
  }
}

void TextView::ShowIllegalMove(std::string_view input) {
  std::cout << "Illegal move: " << input << '\n';
}

void TextView::ShowParseError(ParseError err, std::string_view input) {
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

void TextView::ShowResult(GameResult result) {
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

}  // namespace chess
