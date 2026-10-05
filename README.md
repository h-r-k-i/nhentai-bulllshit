# nhentai bulllshit

Takes a numeric gallery code, generates every distinct permutation of its digits,
and asks the nhentai API which of those permutations are also real galleries.

That's it. That's the whole project. You type `177013`, it tells you what the
other 179 arrangements of those digits are.

## Why

No reason was given. None is needed.

## Dependencies

- `gcc` (or any C99 compiler)
- `libcurl` with dev headers (`libcurl4-openssl-dev` on Debian/Ubuntu,
  `curl-devel` on Fedora, `mingw-w64-ucrt-x86_64-curl` on MSYS2)
- [cJSON](https://github.com/DaveGamble/cJSON). Drop `cJSON.c` and `cJSON.h` into
  this directory. They're gitignored, so a fresh clone won't have them.

## Build & run

The `Makefile` detects Windows (`OS=Windows_NT`) and uses MSYS2/UCRT64 paths and `del`
there. On everything else it uses plain `gcc` and `rm -f`.

```sh
make build              # compile to ./porn (porn.exe on Windows)
make run CODE=177013    # run it against a code
make CODE=177013        # build, run, then delete the binary
make clean              # delete the binary
make clean1             # delete the binary and out.txt
```

If you'd rather run it by hand:

```sh
gcc -Wall -Wextra cJSON.c pornography.c -o porn -lcurl
./porn 177013
```

## Usage

```sh
./porn <code>
```

You must pass exactly one argument, and it must be digits only. Anything else prints a
usage or error line and exits with `-1` (which shows up as 255).

Example output:

```
177013 has 180 permutations
code 235778 corresponds to gallery SAKURA FLEET
code 325778 corresponds to gallery 好きな人に好きな人がいた話のまとめ
code 523778 corresponds to gallery Sunao ni Narenai Seito no Honne
code 713017 does not correspond to a valid gallery
```

Titles are printed as `pretty`, falling back to `english`, then to `japanese` (with
a "(translate yourself)" note).

## How it works

1. **Count.** `calculatePermutationCount` computes `n! / Π cᵢ!` over the digit
   counts, so repeated digits don't inflate the count (`177013` gives 180, not 720).
2. **Sort.** `sort` puts the digits in order with a top-down merge sort (borrowed
   from Wikipedia, as the comment admits).
3. **Permute.** `permutations` runs Heap's algorithm. Each result goes to
   `addToPermutationList`, which converts it to an integer, skips it if it's already
   in the array, and otherwise writes it to the first slot that holds `0`.
4. **Query.** Each code becomes a `GET https://nhentai.net/api/v2/galleries/<code>`,
   and the response is parsed with cJSON.
   - `200`: print the title.
   - `404`: print "does not correspond to a valid gallery".
   - `429`: read `Retry-After`, sleep at least one second, and retry. After
     `MAX_RETRIES` (6) retries on the same code, give up and exit `100`.
   - Anything else is logged to stderr, and the program moves on to the next code.

### Dry run

`main` has an `#if true` / `#else` block. Change it to `#if false` and the program
prints the URL it *would* request for each permutation, without making any network
requests.

## Portability

`pornography.c` builds on Windows (MSYS2/UCRT64) and on Linux:

| | Windows | POSIX |
| --- | --- | --- |
| Sleep | `Sleep()` | `nanosleep()` |
| `max()` | from `<windows.h>` | `static inline` defined in the file |
| `uint64_t` format | `%llu` | `%lu` |
| UTF-8 output | `SetConsoleOutputCP(CP_UTF8)` | `setlocale(LC_ALL, "")` |

## Known rough edges

- **The 429 retry path reads the wrong slot.** `i--` runs before
  `perms[i]` is used for the retry bookkeeping and the give-up message, so those
  refer to the *previous* code. If the very first request (`i == 0`) gets a 429,
  `i` wraps to `UINT64_MAX` and `perms[i]` is an out-of-bounds read. The fix is to
  move the `i--` to just before `continue`.
- `#define _POSIX_C_SOURCE` comes after `#include <stdint.h>`, so glibc has already
  set it, and gcc warns that it's being redefined. Move the define to line 1.
- The `LLU` / `LLD` macros assume `uint64_t` is `unsigned long` on POSIX. That's
  true on 64-bit Linux but not on 32-bit targets. `PRIu64` / `PRId64` from
  `<inttypes.h>` would work everywhere.
- `addToPermutationList` treats `0` as an empty slot, so a code made entirely of
  zeros is never stored. Leading zeros are dropped when a permutation is converted
  to an integer, so `012345` gets queried as `12345`.
- Deduplication is a linear scan per permutation, which makes it O(n²) in the
  number of permutations. That's fine for six digits and painful for ten.
- `calculatePermutationCount` returns `-1` into a `uint64_t` on empty input. Input
  validation in `main` means this never happens. The comment on that line is
  "fuck you".
- `factorial` overflows `uint64_t` for inputs longer than 20 digits.
- `printf` usage/error lines in `main` have no trailing newline.
- Exit codes are vibes: `-1`, `-69`, `-67`, `19`, `100`, `420`, `-676767`.
- No `User-Agent` is set, and requests are sent back to back with no delay, so the
  rate limiter will notice you. 180 sequential requests is not subtle.

## Files

| File | What |
| --- | --- |
| `pornography.c` | The whole program. |
| `Makefile` | Cross-platform build/run/clean. |
| `LICENSE` | MIT. |
| `cJSON.c`, `cJSON.h` | Vendored JSON parser. Gitignored. |
| `nhentai-check.js`, `package.json` | Abandoned Node prototype using `nhentai-js`. Gitignored. |
| `gallist.txt`, `out.txt`, `file*.txt` | Saved output from earlier runs. Gitignored. |

## License

MIT. See [`LICENSE`](LICENSE). Copyright 2026 Renee Graves.
