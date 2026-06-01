#include "game.h"
#include "text_view.h"

int main() {
  Game game(std::make_unique<TextView>());
  game.Play();
  return 0;
}
