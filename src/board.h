#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>

#include "board_state.h"
#include "castling_rights.h"
#include "game_result.h"
#include "history_entry.h"
#include "move.h"
#include "piece.h"
#include "square.h"

class Board {
 public:
  static constexpr int kSize = 8;

  Board();
  Board(const Board&) = delete;
  Board& operator=(const Board&) = delete;
  Board(Board&&) = default;
  Board& operator=(Board&&) = default;

  void Reset();

  const Piece* At(const Square& square) const;
  const Piece* At(int row, int col) const;

  void Apply(const Move& move);
  void Undo();

  Color ToMove() const { return side_to_move_; }

  static bool InBounds(const Square& square);
  static bool InBounds(int row, int col);

  bool IsLegal(const Move& move);
  bool IsSquareAttacked(Square s, Color color) const;
  std::optional<GameResult> Result();
  GameResult HandleResign() const;

  bool CanCastleKingside(Color color) const;
  bool CanCastleQueenside(Color color) const;

  bool IsThreefold() const;
  bool IsInsufficientMaterial() const;

  int8_t EpFile() const;

  BoardState Snapshot() const;
  void Restore(const BoardState& board_state);

 private:
  static constexpr int kFiftyMoveLimit = 100;  // plies (50 full moves)

  std::array<std::array<std::unique_ptr<Piece>, kSize>, kSize> grid_;
  Color side_to_move_ = Color::kWhite;
  int halfmove_clock_ = 0;
  CastlingRights castling_;
  std::vector<HistoryEntry> history_;
  int8_t ep_file_ = -1;

  void Setup();
  void ApplyNoHistory(const Move& move);
  bool IsInCheck(Color color) const;
  bool IsValidMove(const Move& move, const Piece& piece) const;
  bool IsYourMove(const Piece& piece) const;
  bool HasAnyLegalMove(Color side);
  std::optional<Square> FindPiece(const PieceType ptype,
                                  const Color color) const;
};
