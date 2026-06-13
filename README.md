# chesscli

Terminal chess with a minimax engine, written in C++20.

## Build & run

```
make
./build/chesscli
```

Launches in TUI mode by default. Use `--no-tui` for a plain CLI.

## Flags

| Flag | Effect |
|------|--------|
| `--tui` / `--no-tui` | Force TUI or CLI mode |
| `--text` | ASCII board instead of Unicode (CLI only) |
| `--engine DEPTH` | Minimax depth (default 3) |
| `--no-engine` | Two-player mode |
| `--play-black` | You play Black; engine plays White |
| `--configure` | Open config editor, save, and exit |
| `--help` | Show usage |

CLI flags override the config file.

## Config file

Stored at `$XDG_CONFIG_HOME/chesscli/config.ini` (falls back to `~/.config/chesscli/config.ini`).

```ini
tui=true
unicode=true
engine_depth=3
play_black=false
```

Run `chesscli --configure` to edit these interactively.

## In-game commands

```
e2e4      move (UCI)
Nf3       move (SAN)
O-O       castle
e7e8q     promote to queen
undo      revert last move
reset     new game
history   show move list
resign    forfeit
quit      exit
```

## Make targets

| Target | Action |
|--------|--------|
| `make` / `make build` | Configure + compile |
| `make run` | Build + launch |
| `make test` | Run unit tests |
| `make format` | clang-format in place |
| `make clean` | Remove build dir |

Requires CMake ≥ 3.20 and a C++20 compiler.

## License

MIT. See `LICENSE.md`.
