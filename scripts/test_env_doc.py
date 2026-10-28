#!/usr/bin/env python3
"""Tests for docs/env.md: every key in the game env table
(src/env/recomp_env_game.h) has a row there, and every row names a key the
table has, so the page keeps up as more game keys arrive.

  python3 scripts/test_env_doc.py
  python3 -m pytest scripts/test_env_doc.py

Exit 0 when every test passes.
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TABLE = os.path.join(ROOT, "src", "env", "recomp_env_game.h")
DOC = os.path.join(ROOT, "docs", "env.md")


def table_keys(text):
    """The NAME column of the X(ID, TIER, NAME, LEGACY, HELP) rows."""
    return set(re.findall(r'X\(\s*\w+\s*,\s*\w+\s*,\s*"(RECOMP_\w+)"', text))


def doc_keys(text):
    """The key column of the table rows: | `RECOMP_X` | ..."""
    return set(re.findall(r"^\| `(RECOMP_\w+)` \|", text, re.M))


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def test_every_game_key_has_a_row():
    keys, rows = table_keys(read(TABLE)), doc_keys(read(DOC))
    assert keys, "no keys parsed from recomp_env_game.h"
    assert keys - rows == set(), f"keys without a docs/env.md row: {sorted(keys - rows)}"
    assert rows - keys == set(), f"docs/env.md rows for no key: {sorted(rows - keys)}"


def test_parse():
    h = '#define K(X) \\\n    X(FOO, CONFIG, "RECOMP_FOO", NULL, "x") \\\n    X(BAR, TRACE, "RECOMP_BAR", NULL, "y")\n'
    md = "| key | tier |\n|---|---|\n| `RECOMP_FOO` | config |\n"
    assert table_keys(h) == {"RECOMP_FOO", "RECOMP_BAR"}
    assert doc_keys(md) == {"RECOMP_FOO"}


if __name__ == "__main__":
    fails = 0
    for name, fn in list(globals().items()):
        if name.startswith("test_") and callable(fn):
            try:
                fn()
                print(f"ok   {name}")
            except AssertionError as e:
                fails += 1
                print(f"FAIL {name}: {e}")
    sys.exit(1 if fails else 0)
