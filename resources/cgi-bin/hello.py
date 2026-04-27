#!/usr/bin/env python3
"""Minimal GET demo: echoes QUERY_STRING and a few env vars."""
import os
import sys

print("Content-Type: text/html")
print()
print("<html><head><title>webserv CGI</title></head><body>")
print("<h1>Hello from CGI</h1>")
print("<p>QUERY_STRING: <code>{}</code></p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>REQUEST_METHOD: <code>{}</code></p>".format(os.environ.get("REQUEST_METHOD", "")))
print("<p>SCRIPT_NAME: <code>{}</code></p>".format(os.environ.get("SCRIPT_NAME", "")))
print("<p>SERVER_NAME: <code>{}</code></p>".format(os.environ.get("SERVER_NAME", "")))
print("<p>cwd: <code>{}</code></p>".format(os.getcwd()))
print("</body></html>")
sys.stdout.flush()
