#pragma once

namespace chess {

enum class GameResult {
  kWhiteWins,
  kBlackWins,
  kStalemateDraw,
  kFiftyMoveDraw,
  kThreefoldDraw,
  kInsufficientMaterialDraw
};

}  // namespace chess
