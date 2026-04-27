#!/usr/bin/env python3
"""Reads a sibling file via RELATIVE path to prove cwd is the script dir."""
import os, sys
print("Content-Type: text/plain")
print()
print("cwd:", os.getcwd())
try:
    with open("hello.py") as f:    # relative path, no leading slash
        print("read sibling 'hello.py' OK, first line:", f.readline().rstrip())
except Exception as e:
    print("FAILED:", e); sys.exit(1)
