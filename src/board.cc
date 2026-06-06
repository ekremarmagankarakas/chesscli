#include "board.h"

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"

Board::Board() { Setup(); }

void Board::Setup() {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      grid_[row][col] = nullptr;
    }
  }
  side_to_move_ = Color::kWhite;
  castling_ = CastlingRights{};
  ep_file_ = -1;
  halfmove_clock_ = 0;
  history_.clear();

  for (int col = 0; col < kSize; ++col) {
    grid_[1][col] = std::make_unique<Pawn>(Color::kWhite);
    grid_[6][col] = std::make_unique<Pawn>(Color::kBlack);
  }
  grid_[0][0] = std::make_unique<Rook>(Color::kWhite);
  grid_[0][7] = std::make_unique<Rook>(Color::kWhite);
  grid_[7][0] = std::make_unique<Rook>(Color::kBlack);
  grid_[7][7] = std::make_unique<Rook>(Color::kBlack);
  grid_[0][1] = std::make_unique<Knight>(Color::kWhite);
  grid_[0][6] = std::make_unique<Knight>(Color::kWhite);
  grid_[7][1] = std::make_unique<Knight>(Color::kBlack);
  grid_[7][6] = std::make_unique<Knight>(Color::kBlack);
  grid_[0][2] = std::make_unique<Bishop>(Color::kWhite);
  grid_[0][5] = std::make_unique<Bishop>(Color::kWhite);
  grid_[7][2] = std::make_unique<Bishop>(Color::kBlack);
  grid_[7][5] = std::make_unique<Bishop>(Color::kBlack);
  grid_[0][3] = std::make_unique<Queen>(Color::kWhite);
  grid_[7][3] = std::make_unique<Queen>(Color::kBlack);
  grid_[0][4] = std::make_unique<King>(Color::kWhite);
  grid_[7][4] = std::make_unique<King>(Color::kBlack);
}

BoardState Board::Snapshot() const {
  std::array<uint8_t, 64> squares = {};
  for (auto it = grid_.begin(); it != grid_.end(); ++it) {
    for (auto jt = it->begin(); jt != it->end(); ++jt) {
      if (*jt) {
        uint8_t piece = 0;
        switch ((*jt)->GetType()) {
          case PieceType::kPawn:
            piece = 1;
            break;
          case PieceType::kKnight:
            piece = 2;
            break;
          case PieceType::kBishop:
            piece = 3;
            break;
          case PieceType::kRook:
            piece = 4;
            break;
          case PieceType::kQueen:
            piece = 5;
            break;
          case PieceType::kKing:
            piece = 6;
            break;
        }
        if ((*jt)->GetColor() == Color::kBlack) {
          piece += 6;  // black pieces encoded as 7-12
        }
        squares[(it - grid_.begin()) * kSize + (jt - it->begin())] = piece;
      } else {
        squares[(it - grid_.begin()) * kSize + (jt - it->begin())] = 0;
      }
    }
  }
  return BoardState(squares, side_to_move_ == Color::kWhite ? 0 : 1,
                    castling_.Snapshot(), ep_file_, halfmove_clock_);
}

void Board::Restore(const BoardState& board_state) {
  for (int sq = 0; sq < 64; ++sq) {
    int row = sq / kSize;
    int col = sq % kSize;
    uint8_t code = board_state.squares[sq];
    if (code == 0) {
      grid_[row][col] = nullptr;
      continue;
    }
    Color color = code <= 6 ? Color::kWhite : Color::kBlack;
    uint8_t type = code <= 6 ? code : code - 6;
    switch (type) {
      case 1:
        grid_[row][col] = std::make_unique<Pawn>(color);
        break;
      case 2:
        grid_[row][col] = std::make_unique<Knight>(color);
        break;
      case 3:
        grid_[row][col] = std::make_unique<Bishop>(color);
        break;
      case 4:
        grid_[row][col] = std::make_unique<Rook>(color);
        break;
      case 5:
        grid_[row][col] = std::make_unique<Queen>(color);
        break;
      case 6:
        grid_[row][col] = std::make_unique<King>(color);
        break;
      default:
        grid_[row][col] = nullptr;
        break;  // corrupt input
    }
  }
  side_to_move_ = board_state.side == 0 ? Color::kWhite : Color::kBlack;
  castling_.Restore(board_state.castling);
  ep_file_ = board_state.ep_file;
  halfmove_clock_ = board_state.halfmove_clock;
}

const Piece* Board::At(const Square& square) const {
  return At(square.row, square.col);
}

const Piece* Board::At(int row, int col) const {
  if (!InBounds(row, col)) {
    return nullptr;
  }
  return grid_[row][col].get();
}

bool Board::IsThreefold() const {
  BoardState current = Snapshot();
  int count = 1;
  for (const auto& entry : history_) {
    if (entry.pre_state == current) {
      ++count;
      if (count >= 3) {
        return true;
      }
    }
  }
  return false;
}

bool Board::IsInsufficientMaterial() const {
  int white_knights = 0, black_knights = 0;
  int white_bishops_light = 0, white_bishops_dark = 0;
  int black_bishops_light = 0, black_bishops_dark = 0;

  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      const Piece* p = At(row, col);
      if (!p) {
        continue;
      }
      PieceType t = p->GetType();

      // Any pawn/rook/queen = sufficient material. Bail out.
      if (t == PieceType::kPawn || t == PieceType::kRook ||
          t == PieceType::kQueen) {
        return false;
      }
      if (t == PieceType::kKing) {
        continue;
      }

      bool light_square = (row + col) % 2 == 1;
      if (t == PieceType::kKnight) {
        if (p->GetColor() == Color::kWhite) {
          ++white_knights;
        } else {
          ++black_knights;
        }
      } else {  // bishop
        if (p->GetColor() == Color::kWhite) {
          if (light_square) {
            ++white_bishops_light;
          } else {
            ++white_bishops_dark;
          }
        } else {
          if (light_square) {
            ++black_bishops_light;
          } else {
            ++black_bishops_dark;
          }
        }
      }
    }
  }

  int white_minors = white_knights + white_bishops_light + white_bishops_dark;
  int black_minors = black_knights + black_bishops_light + black_bishops_dark;

  // K vs K.
  if (white_minors == 0 && black_minors == 0) {
    return true;
  }
  // K+minor vs K.
  if (white_minors == 1 && black_minors == 0) {
    return true;
  }
  if (white_minors == 0 && black_minors == 1) {
    return true;
  }
  // K+B vs K+B with same-color bishops only (no knights).
  if (white_knights == 0 && black_knights == 0) {
    bool all_on_light = white_bishops_dark == 0 && black_bishops_dark == 0;
    bool all_on_dark = white_bishops_light == 0 && black_bishops_light == 0;
    if (all_on_light || all_on_dark) {
      return true;
    }
  }
  return false;
}

bool Board::InBounds(const Square& s) { return InBounds(s.row, s.col); }

bool Board::InBounds(int row, int col) {
  return row >= 0 && row < kSize && col >= 0 && col < kSize;
}

std::optional<Square> Board::FindPiece(const PieceType ptype,
                                       const Color color) const {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (piece && piece->GetType() == ptype && piece->GetColor() == color) {
        return Square{row, col};
      }
    }
  }
  return std::nullopt;
}

bool Board::IsLegal(const Move& move) {
  if (!InBounds(move.from) || !InBounds(move.to)) {
    return false;
  }

  const Piece* piece = At(move.from);
  if (!piece) {
    return false;
  }
  if (!IsYourMove(*piece)) {
    return false;
  }
  if (!IsValidMove(move, *piece)) {
    return false;
  }

  // Promotion required iff pawn reaches last rank.
  int last_rank = piece->GetColor() == Color::kWhite ? 7 : 0;
  bool reaches_last =
      piece->GetType() == PieceType::kPawn && move.to.row == last_rank;
  if (reaches_last != move.promotion.has_value()) {
    return false;
  }

  // Simulate and reject if own king attacked after.
  BoardState snap = Snapshot();
  ApplyNoHistory(move);
  bool ok = !IsInCheck(piece->GetColor());
  Restore(snap);
  return ok;
}

int8_t Board::EpFile() const { return ep_file_; }

bool Board::IsYourMove(const Piece& piece) const {
  return (piece.GetColor() == side_to_move_);
}

bool Board::IsValidMove(const Move& move, const Piece& piece) const {
  auto moves = piece.ValidMoves(move.from, *this);
  for (const auto& sq : moves) {
    if (sq == move.to) {
      return true;
    }
  }
  return false;
}

bool Board::IsInCheck(Color color) const {
  std::optional<Square> king_square = FindPiece(PieceType::kKing, color);
  if (!king_square) {
    return false;
  }
  return IsSquareAttacked(*king_square, color);
}

bool Board::IsSquareAttacked(Square s, Color color) const {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (piece && piece->GetColor() != color) {
        auto attacks = piece->Attacks({row, col}, *this);
        if (std::find(attacks.begin(), attacks.end(), s) != attacks.end()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool Board::CanCastleKingside(Color color) const {
  if (!castling_.Kingside(color)) {
    return false;
  }
  int row = color == Color::kWhite ? 0 : 7;
  if (At(row, 5) || At(row, 6)) {
    return false;
  }
  if (IsSquareAttacked({row, 4}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 5}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 6}, color)) {
    return false;
  }
  return true;
}

bool Board::CanCastleQueenside(Color color) const {
  if (!castling_.Queenside(color)) {
    return false;
  }
  int row = color == Color::kWhite ? 0 : 7;
  if (At(row, 1) || At(row, 2) || At(row, 3)) {
    return false;
  }
  if (IsSquareAttacked({row, 4}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 3}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 2}, color)) {
    return false;
  }
  return true;
}

std::optional<GameResult> Board::Result() {
  Color side = side_to_move_;
  bool has_legal_move = HasAnyLegalMove(side);
  if (!has_legal_move && IsInCheck(side)) {
    return side == Color::kWhite ? GameResult::kBlackWins
                                 : GameResult::kWhiteWins;
  }
  if (!has_legal_move) {
    return GameResult::kStalemateDraw;
  }
  if (halfmove_clock_ >= kFiftyMoveLimit) {
    return GameResult::kFiftyMoveDraw;
  }
  if (IsThreefold()) {
    return GameResult::kThreefoldDraw;
  }
  if (IsInsufficientMaterial()) {
    return GameResult::kInsufficientMaterialDraw;
  }
  return std::nullopt;
}

GameResult Board::HandleResign() const {
  return side_to_move_ == Color::kWhite ? GameResult::kBlackWins
                                        : GameResult::kWhiteWins;
}

bool Board::HasAnyLegalMove(Color side) {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (!piece || piece->GetColor() != side) {
        continue;
      }
      auto moves = piece->ValidMoves({row, col}, *this);
      for (const auto& to : moves) {
        BoardState snap = Snapshot();
        ApplyNoHistory(Move{Square{row, col}, to, std::nullopt});
        bool in_check = IsInCheck(side);
        Restore(snap);
        if (!in_check) {
          return true;
        }
      }
    }
  }
  return false;
}

void Board::Undo() {
  if (history_.empty()) {
    return;
  }
  Restore(history_.back().pre_state);
  history_.pop_back();
}

void Board::Apply(const Move& move) {
  history_.push_back(HistoryEntry{Snapshot(), move});
  ApplyNoHistory(move);
}

void Board::ApplyNoHistory(const Move& move) {
  const Piece* moving = At(move.from);
  if (!moving) {
    return;
  }
  Color color = moving->GetColor();
  PieceType ptype = moving->GetType();

  // En passant: pawn moves diagonally to empty square -> remove the
  // adjacent pawn that just double-pushed.
  bool is_ep_capture =
      ptype == PieceType::kPawn && move.from.col != move.to.col && !At(move.to);
  bool is_capture = At(move.to) != nullptr || is_ep_capture;
  bool is_pawn_move = ptype == PieceType::kPawn;

  if (is_pawn_move && std::abs(move.to.row - move.from.row) == 2) {
    ep_file_ = move.to.col;
  } else {
    ep_file_ = -1;
  }

  if (is_capture || is_pawn_move) {
    halfmove_clock_ = 0;
  } else {
    ++halfmove_clock_;
  }

  if (is_ep_capture) {
    grid_[move.from.row][move.to.col] = nullptr;
  }

  // Castling: king moves 2 columns.
  bool is_castle =
      ptype == PieceType::kKing && std::abs(move.to.col - move.from.col) == 2;
  if (is_castle) {
    int rook_from = move.to.col > move.from.col ? 7 : 0;
    int rook_to = move.to.col > move.from.col ? 5 : 3;
    grid_[move.from.row][rook_to] = std::move(grid_[move.from.row][rook_from]);
  }

  if (move.promotion) {
    switch (*move.promotion) {
      case PieceType::kQueen:
        grid_[move.to.row][move.to.col] = std::make_unique<Queen>(color);
        break;
      case PieceType::kRook:
        grid_[move.to.row][move.to.col] = std::make_unique<Rook>(color);
        break;
      case PieceType::kBishop:
        grid_[move.to.row][move.to.col] = std::make_unique<Bishop>(color);
        break;
      case PieceType::kKnight:
        grid_[move.to.row][move.to.col] = std::make_unique<Knight>(color);
        break;
      default:
        return;
    }
    grid_[move.from.row][move.from.col] = nullptr;
  } else {
    grid_[move.to.row][move.to.col] =
        std::move(grid_[move.from.row][move.from.col]);
  }

  // Update castling rights.
  if (ptype == PieceType::kKing) {
    castling_.ClearAll(color);
  }
  if (ptype == PieceType::kRook) {
    castling_.ClearByRookSquare(move.from);
  }
  castling_.ClearByRookSquare(move.to);  // captured rook on home square

  side_to_move_ =
      side_to_move_ == Color::kWhite ? Color::kBlack : Color::kWhite;
}
