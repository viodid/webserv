#!/usr/bin/env python3
"""Crash demo: exits with an exception and produces no headers, so the
server should respond 502 Bad Gateway."""
import sys
sys.stderr.write("crash.py: deliberate failure\n")
raise RuntimeError("crash.py: deliberate failure")
