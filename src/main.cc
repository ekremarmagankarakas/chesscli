#include <cstdlib>
#include <iostream>
#include <memory>
#include <string_view>

#include "engine.h"
#include "game.h"
#include "minimax_engine.h"
#include "piece.h"
#include "stdin_input.h"
#include "text_view.h"
#include "unicode_view.h"

namespace {

const char* kUsage =
    "Usage: chesscli [--unicode] [--engine DEPTH] [--play-black] [--help]\n"
    "  --unicode        Render board with Unicode pieces and colors\n"
    "  --engine DEPTH   Enable minimax engine at given search depth (e.g. 3)\n"
    "  --play-black     You play black; engine plays white (default: "
    "opposite)\n"
    "  --help, -h       Show this help\n"
    "\n"
    "In-game commands: move (e.g. e2e4 or e7e8q), undo, reset, history,\n"
    "                  resign, quit, exit, help.\n";

}  // namespace

int main(int argc, char* argv[]) {
  bool unicode = false;
  int engine_depth = 0;  // 0 = no engine
  Color engine_side = Color::kBlack;

  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg == "--unicode") {
      unicode = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << kUsage;
      return 0;
    } else if (arg == "--engine") {
      if (i + 1 >= argc) {
        std::cerr << "--engine requires a depth argument\n" << kUsage;
        return 2;
      }
      ++i;
      engine_depth = std::atoi(argv[i]);
      if (engine_depth < 1) {
        std::cerr << "--engine depth must be >= 1\n";
        return 2;
      }
    } else if (arg == "--play-black") {
      engine_side = Color::kWhite;
    } else {
      std::cerr << "Unknown option: " << arg << '\n' << kUsage;
      return 2;
    }
  }

  std::unique_ptr<View> view =
      unicode ? std::unique_ptr<View>(std::make_unique<UnicodeView>())
              : std::unique_ptr<View>(std::make_unique<TextView>());

  std::unique_ptr<Engine> engine;
  if (engine_depth > 0) {
    engine = std::make_unique<MinimaxEngine>(engine_depth);
  }

  Game(std::move(view), std::make_unique<StdinInput>(), std::move(engine),
       engine_side)
      .Play();
  return 0;
}
