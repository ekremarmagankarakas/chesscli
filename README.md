# chesscli

Terminal chess engine in C++20. Two humans play via UCI move notation.

## Build & run

```
make build
./build/chesscli
```

Type moves in UCI format:

```
e2e4          # pawn e2 to e4
g1f3          # knight g1 to f3
e1g1          # kingside castle
e7e8q         # promote to queen
undo          # revert last move
quit          # exit
```

## Make targets

| Target | Action |
|--------|--------|
| `make build` | Configure + compile |
| `make run` | Build + launch |
| `make test` | Run unit tests (doctest + perft) |
| `make format` | clang-format in place |
| `make clean` | Remove build dir |

Requires CMake ≥ 3.20, a C++20 compiler, and `clang-format`.

## Architecture

Clean MVC. Model has zero I/O.

- **Model:** `board`, `piece`, `move`, `square`, `castling_rights`, `history_entry`.
- **View:** `view` (abstract), `text_view` (concrete).
- **Input:** `input_source` (abstract), `stdin_input` (concrete).
- **Controller:** `game`, `parser`.
- **Composition root:** `main.cc` injects concrete view + input source.

Swap renderers or input sources by adding a class and one line in `main.cc`.

## Features

- All standard moves, castling, en passant, promotion.
- Check / checkmate / stalemate detection.
- Move history with `undo`.
- Perft-tested rules.

## Roadmap

- 50-move rule, threefold repetition, insufficient-material draw.
- SAN notation (`Nf3`, `O-O`, `e8=Q`).
- FEN load/dump.
- Random + minimax AI for black.
- Unicode piece view.
- PGN export.

## License

MIT. See `LICENSE.md`.
