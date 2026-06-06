#include <iostream>
#include <memory>
#include <string_view>

#include "game.h"
#include "stdin_input.h"
#include "text_view.h"
#include "unicode_view.h"

namespace {

const char* kUsage =
    "Usage: chesscli [--unicode] [--help]\n"
    "  --unicode   Render board with Unicode pieces and colors\n"
    "  --help, -h  Show this help\n"
    "\n"
    "In-game commands: move (e.g. e2e4 or e7e8q), undo, reset, history,\n"
    "                  resign, quit, exit, help.\n";

}  // namespace

int main(int argc, char* argv[]) {
  bool unicode = false;
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg == "--unicode") {
      unicode = true;
    } else if (arg == "--help" || arg == "-h") {
      std::cout << kUsage;
      return 0;
    } else {
      std::cerr << "Unknown option: " << arg << '\n' << kUsage;
      return 2;
    }
  }

  std::unique_ptr<View> view =
      unicode ? std::unique_ptr<View>(std::make_unique<UnicodeView>())
              : std::unique_ptr<View>(std::make_unique<TextView>());
  Game(std::move(view), std::make_unique<StdinInput>()).Play();
  return 0;
}
