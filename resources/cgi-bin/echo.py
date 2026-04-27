#!/usr/bin/env python3
"""POST demo: reads stdin and echoes it back, alongside metadata."""
import os
import sys

length = 0
try:
    length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
except ValueError:
    length = 0

body = b""
if length > 0:
    body = sys.stdin.buffer.read(length)

print("Content-Type: text/plain")
print("X-Echo-Bytes: {}".format(len(body)))
print()
print("method={}".format(os.environ.get("REQUEST_METHOD", "")))
print("content_type={}".format(os.environ.get("CONTENT_TYPE", "")))
print("content_length={}".format(length))
print("--- body ---")
sys.stdout.flush()
sys.stdout.buffer.write(body)
sys.stdout.buffer.write(b"\n")
sys.stdout.flush()
