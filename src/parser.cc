#include "parser.h"

#include <cctype>
#include <optional>
#include <string_view>
#include <vector>

#include "board.h"
#include "move.h"
#include "piece.h"
#include "square.h"

namespace chess {

namespace {

std::optional<PieceType> PromotionFromChar(char c) {
  switch (std::tolower(static_cast<unsigned char>(c))) {
    case 'q':
      return PieceType::kQueen;
    case 'r':
      return PieceType::kRook;
    case 'b':
      return PieceType::kBishop;
    case 'n':
      return PieceType::kKnight;
    default:
      return std::nullopt;
  }
}

std::optional<PieceType> PieceTypeFromLetter(char c) {
  switch (c) {
    case 'K':
      return PieceType::kKing;
    case 'Q':
      return PieceType::kQueen;
    case 'R':
      return PieceType::kRook;
    case 'B':
      return PieceType::kBishop;
    case 'N':
      return PieceType::kKnight;
    default:
      return std::nullopt;
  }
}

// Strict UCI shape check: e.g. "e2e4" or "e7e8q".
bool LooksLikeUCI(std::string_view s) {
  if (s.size() != 4 && s.size() != 5) {
    return false;
  }
  return std::islower(static_cast<unsigned char>(s[0])) &&
         std::isdigit(static_cast<unsigned char>(s[1])) &&
         std::islower(static_cast<unsigned char>(s[2])) &&
         std::isdigit(static_cast<unsigned char>(s[3]));
}

// Find a pseudo-legal move matching SAN constraints. Pseudo-legal only —
// the caller (Game) re-checks full legality via Board::IsLegal.
std::optional<Move> FindMatchingMove(const Board& board, PieceType type,
                                     Square to,
                                     std::optional<PieceType> promotion,
                                     std::optional<int> from_file,
                                     std::optional<int> from_rank) {
  std::vector<Move> matches;
  Color side = board.ToMove();
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      const Piece* p = board.At(row, col);
      if (!p || p->GetColor() != side || p->GetType() != type) {
        continue;
      }
      if (from_file && *from_file != col) {
        continue;
      }
      if (from_rank && *from_rank != row) {
        continue;
      }
      for (const Square& dest : p->ValidMoves({row, col}, board)) {
        if (dest == to) {
          matches.push_back(Move{Square{row, col}, dest, promotion});
        }
      }
    }
  }
  if (matches.size() == 1) {
    return matches[0];
  }
  return std::nullopt;
}

}  // namespace

std::optional<Move> ParseUCI(std::string_view input) {
  if (!LooksLikeUCI(input)) {
    return std::nullopt;
  }
  auto from = Square::FromAlgebraic(input.substr(0, 2));
  auto to = Square::FromAlgebraic(input.substr(2, 2));
  if (!from || !to) {
    return std::nullopt;
  }
  std::optional<PieceType> promotion;
  if (input.size() == 5) {
    auto p = PromotionFromChar(input[4]);
    if (!p) {
      return std::nullopt;
    }
    promotion = *p;
  }
  return Move{*from, *to, promotion};
}

std::optional<Move> ParseSAN(std::string_view s, const Board& board) {
  if (s.empty()) {
    return std::nullopt;
  }

  // Strip trailing check/mate markers.
  while (!s.empty() && (s.back() == '+' || s.back() == '#')) {
    s.remove_suffix(1);
  }
  if (s.empty()) {
    return std::nullopt;
  }

  // Castling.
  if (s == "O-O" || s == "0-0") {
    int row = board.ToMove() == Color::kWhite ? 0 : 7;
    return Move{Square{row, 4}, Square{row, 6}, std::nullopt};
  }
  if (s == "O-O-O" || s == "0-0-0") {
    int row = board.ToMove() == Color::kWhite ? 0 : 7;
    return Move{Square{row, 4}, Square{row, 2}, std::nullopt};
  }

  // Promotion suffix: "...=Q" or "...Q" (uppercase). Detect before extracting
  // destination so the trailing piece letter doesn't confuse parsing.
  std::optional<PieceType> promotion;
  if (s.size() >= 3) {
    char last = s.back();
    if (last >= 'A' && last <= 'Z') {
      auto p = PromotionFromChar(last);
      if (p) {
        char prev = s[s.size() - 2];
        if (prev == '=') {
          promotion = *p;
          s.remove_suffix(2);
        } else if (prev >= '1' && prev <= '8') {
          promotion = *p;
          s.remove_suffix(1);
        }
      }
    }
  }

  // Piece type from leading uppercase letter (default pawn).
  PieceType type = PieceType::kPawn;
  size_t i = 0;
  if (!s.empty() && std::isupper(static_cast<unsigned char>(s[0]))) {
    auto t = PieceTypeFromLetter(s[0]);
    if (!t) {
      return std::nullopt;
    }
    type = *t;
    i = 1;
  }

  // Destination = last 2 chars (file letter + rank digit).
  if (s.size() < i + 2) {
    return std::nullopt;
  }
  auto to = Square::FromAlgebraic(s.substr(s.size() - 2));
  if (!to) {
    return std::nullopt;
  }
  s.remove_suffix(2);

  // Optional 'x' capture marker (purely decorative).
  if (!s.empty() && s.back() == 'x') {
    s.remove_suffix(1);
  }

  // Remaining middle = disambiguator: file letter and/or rank digit.
  std::string_view disamb = s.substr(i);
  std::optional<int> from_file, from_rank;
  for (char c : disamb) {
    if (c >= 'a' && c <= 'h') {
      from_file = c - 'a';
    } else if (c >= '1' && c <= '8') {
      from_rank = c - '1';
    } else {
      return std::nullopt;
    }
  }

  return FindMatchingMove(board, type, *to, promotion, from_file, from_rank);
}

Command Parse(std::string_view input, const Board& board) {
  if (input.empty()) {
    return ParseError::kEmpty;
  }
  if (input == "quit" || input == "exit") {
    return QuitCmd{};
  }
  if (input == "reset") {
    return ResetCmd{};
  }
  if (input == "undo") {
    return UndoCmd{};
  }
  if (input == "resign") {
    return ResignCmd{};
  }
  if (input == "history") {
    return HistoryCmd{};
  }
  if (input == "help") {
    return HelpCmd{};
  }

  // UCI shape match: classify errors specifically (bad square / bad promo).
  if (LooksLikeUCI(input)) {
    auto from = Square::FromAlgebraic(input.substr(0, 2));
    auto to = Square::FromAlgebraic(input.substr(2, 2));
    if (!from || !to) {
      return ParseError::kBadSquare;
    }
    std::optional<PieceType> promotion;
    if (input.size() == 5) {
      auto p = PromotionFromChar(input[4]);
      if (!p) {
        return ParseError::kBadPromotion;
      }
      promotion = *p;
    }
    return Move{*from, *to, promotion};
  }

  if (auto m = ParseSAN(input, board)) {
    return *m;
  }

  return ParseError::kBadSyntax;
}

}  // namespace chess
