# Contributing to webserv

Thanks for your interest. This document describes how to set up the project, the conventions we follow, and the workflow for landing changes.

## Getting set up

```sh
git clone <repo>
cd webserv
make            # builds the webserv binary
./webserv       # serves resources/default.conf on 127.0.0.1:8080 and 127.0.0.1:9090
```

Unit tests require `libgtest-dev`:

```sh
sudo apt install libgtest-dev
make test
```

End-to-end tests run against a live server:

```sh
./webserv &
python3 test/e2e/tests.py http://127.0.0.1:8080
```

## Code style

- **Language:** C++98 only. The Makefile passes `-std=c++98 -Wall -Wextra -Werror -Wpedantic`. No exceptions.
- **No external dependencies** in `src/` or `include/`. The project uses only the C++98 standard library and POSIX. The `test/` tree may use GoogleTest.
- **Headers** live in `include/` and mirror the directory layout under `src/`.
- **No comments that restate what the code does.** Comments belong on the *why* — a non-obvious invariant, a workaround, an RFC reference. Keep them short.
- **Filesystem paths in code** (`include/Settings.hpp`, `resources/default.conf`) must stay relative to the repo root. Absolute, machine-specific paths are rejected.

## Branching and commits

- Branch off `develop`, not `main`. Use a descriptive prefix:
  - `feature/<short-name>` — new functionality
  - `fix/<short-name>` — bug fix
  - `chore/<short-name>` — tooling, dependencies, docs-only changes
  - `docs/<short-name>` — documentation
- **Commit messages:** imperative mood, lowercase, ≤72 chars on the subject line. Prefix with a category when it helps scan the log (`docs:`, `test:`, `fix:`). Example: `fix: stop doubling cgi script path after chdir`.
- Keep each commit focused. Prefer several small commits over one giant one. Squash trivial fixups before opening the PR.

## Pull requests

1. Open the PR against `develop`.
2. Fill in the description: what the change does, why, and how to test it.
3. CI — `make`, `make test`, and `python3 test/e2e/tests.py` — must all pass.
4. Address review feedback in follow-up commits with messages like `address PR #N review: <summary>`. Do not force-push during review; we squash on merge if needed.
5. A maintainer merges once the diff is clean and tests are green.

## Testing requirements

A change is not ready for review until:

- `make re` builds with zero warnings.
- `make test` passes the full GoogleTest suite.
- `python3 test/e2e/tests.py http://127.0.0.1:8080` passes against the default config.
- Any new behavior has a unit test, an e2e test, or both. Bug fixes should include a regression test that fails without the fix.

## Reporting bugs

Open an issue with:

- A minimal reproduction (request / config snippet / commands).
- Expected vs. actual behavior.
- The exact `git rev-parse HEAD` you reproduced on.
- Any relevant server output from stderr.

For security-sensitive issues (memory corruption, RCE in the request parser, etc.), please email the maintainers privately rather than opening a public issue.

## License and AI usage

By contributing you agree your code is licensed under the project's [LICENSE](LICENSE).

If you used AI assistance, follow the same scope the rest of the project does (see the README's *AI usage* note): code review, refactoring help, and bug-spotting are fine; full-feature generation is not. Disclose AI assistance in your PR description.
