#include "text_view.h"

#include <iostream>

#include "board.h"
#include "piece.h"

void TextView::Render(const Board& board) {
  for (int row = Board::kSize - 1; row >= 0; --row) {
    std::cout << (row + 1) << ' ';
    for (int col = 0; col < Board::kSize; ++col) {
      const Piece* piece = board.At(row, col);
      if (piece) {
        std::cout << piece->GetSymbol() << ' ';
      } else {
        std::cout << ". ";
      }
    }
    std::cout << '\n';
  }
  std::cout << "  a b c d e f g h\n";
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
    case GameResult::kStalemate:
      std::cout << "STALEMATE!\n";
      return;
  }
}
