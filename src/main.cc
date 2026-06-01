#include "game.h"
#include "stdin_input.h"
#include "text_view.h"

int main() {
  Game game(std::make_unique<TextView>(), std::make_unique<StdinInput>());
  game.Play();
  return 0;
}
