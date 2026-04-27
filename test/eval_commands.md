# Eval Demo Commands

Quick-reference recipes for the project review. Each section maps to a spec
requirement and is self-contained — copy-paste a block.

> **Setup:** start the server in one terminal, run the curl commands in
> another. Use `-i` to show response headers.
>
> ```bash
> make            # build
> ./webserv       # config: resources/default.conf (Settings::DEFAULT_CONFIG_PATH)
> make test       # GoogleTest unit tests (run separately)
> ```
>
> The bundled `resources/default.conf` exposes:
> - `127.0.0.1:8080` — `/` (portfolio, GET only), `/upload` (GET POST DELETE),
>   `/cgi-bin` (GET POST), `/redirect` (302 → `/`).
> - `0.0.0.0:9090` — `/potato` (autoindex on, custom `error_page 404`).

---

## 1. HTTP status codes are accurate

All 17 codes returned by the server map to the canonical RFC 9110 reason phrases
(see `src/Config.cpp:166-233`). Quick sanity:

```bash
curl -I http://127.0.0.1:8080/                   # 200 OK
curl -I http://127.0.0.1:8080/does-not-exist     # 404 Not Found
curl -I -X DELETE http://127.0.0.1:8080/         # 405 Method Not Allowed
curl -I http://127.0.0.1:8080/redirect           # 302 Found
```

---

## 2. Multiple servers / different ports

Two `listen` blocks, two distinct sites:

```bash
curl -i http://127.0.0.1:8080/                   # portfolio (8080 block)
curl -i http://127.0.0.1:9090/potato/            # static_test (9090 block, autoindex)
```

---

## 3. Multiple servers / different hostnames

Spec treats name-based virtual hosts as out of scope. The eval line is satisfied
by IP/port-based hosting plus `--resolve` (curl bypasses DNS):

```bash
# Two fake hostnames, each pinned to a different port -> hit different blocks.
curl -i --resolve foo.local:8080:127.0.0.1 http://foo.local:8080/
curl -i --resolve bar.local:9090:127.0.0.1 http://bar.local:9090/potato/
```

For a more "vhost-shaped" demo (Linux only — `127.0.0.0/8` is all loopback),
edit the config so the two server blocks share a port but bind to different IPs
(e.g. `listen 127.0.0.1:8080;` and `listen 127.0.0.2:8080;`), then:

```bash
curl -i --resolve site-a.local:8080:127.0.0.1 http://site-a.local:8080/
curl -i --resolve site-b.local:8080:127.0.0.2 http://site-b.local:8080/
```

---

## 4. Custom error page

The 9090 block has `error_page 404 /…/resources/www/404.html`; the 8080 block
doesn't. Compare the response body sizes:

```bash
curl -s -o /dev/null -w "9090 (custom):  %{http_code} %{size_download}B\n" \
    http://127.0.0.1:9090/potato/missing       # ~5 KB custom page
curl -s -o /dev/null -w "8080 (default): %{http_code} %{size_download}B\n" \
    http://127.0.0.1:8080/missing              # ~1.7 KB default page
```

---

## 5. Limit the client request body (`client_max_body_size`)

Demonstrate the contrast: under-limit POST passes, over-limit POST returns
**413 Content Too Large**. Easiest target is the CGI route (regular POST works
there; `/upload` requires chunked):

```bash
grep client_max_body_size resources/default.conf
# e.g. client_max_body_size 100;

# Under the limit -> CGI runs, 200
curl -i -X POST -H "Content-Type: text/plain" \
  --data "short body, under the limit" \
  http://127.0.0.1:8080/cgi-bin/echo.py

# Over the limit -> 413, before any handler runs
curl -i -X POST -H "Content-Type: text/plain" \
  --data "$(printf 'A%.0s' {1..200})" \
  http://127.0.0.1:8080/cgi-bin/echo.py
```

Same check on the chunked upload path:

```bash
curl -i -X POST -H "Transfer-Encoding: chunked" \
  --data "small chunked body" \
  http://127.0.0.1:8080/upload/test.txt          # 201 Created

curl -i -X POST -H "Transfer-Encoding: chunked" \
  --data "$(printf 'B%.0s' {1..200})" \
  http://127.0.0.1:8080/upload/test.txt          # 413
```

> Note: `client_max_body_size` only caps the **request body** (what the client
> uploads). Large *response* bodies (HTML, CSS, images served via GET) are
> unaffected — same semantics as NGINX.

---

## 6. Routes mapped to different directories

Each `location` block has its own `root`:

```bash
curl -s http://127.0.0.1:8080/         | head -5   # → resources/www/portfolio
curl -s http://127.0.0.1:8080/upload/  | head -5   # → resources/upload
```

---

## 7. Default file when requesting a directory

`/` has `index index.html;`. Hitting the directory serves it:

```bash
curl -i http://127.0.0.1:8080/        # serves portfolio/index.html
```

Compare to `/potato` (no `index`, `autoindex on`) which returns the directory
listing instead.

---

## 8. Allowed methods enforced per route

```bash
curl -i -X DELETE http://127.0.0.1:8080/upload/test.txt   # 200/204 (allowed)
curl -i -X DELETE http://127.0.0.1:8080/                  # 405 (GET-only route)
curl -i -X POST   http://127.0.0.1:8080/                  # 405 (GET-only route)
```

(Run the upload POST first if `test.txt` doesn't exist yet; see §5 chunked
example.)

---

## 9. Static website served end-to-end

```bash
curl -i http://127.0.0.1:8080/                       # HTML
curl -i http://127.0.0.1:8080/assets/css/main.css    # CSS (whatever path the
                                                     # portfolio uses)
```

Open `http://127.0.0.1:8080/` in any browser to confirm full page renders.

---

## 10. File upload from client

```bash
curl -i -X POST -H "Transfer-Encoding: chunked" \
  --data-binary @/etc/hostname \
  http://127.0.0.1:8080/upload/hostname.txt        # 201
ls resources/upload/hostname.txt                   # file appeared
```

Then DELETE it:
```bash
curl -i -X DELETE http://127.0.0.1:8080/upload/hostname.txt
```

---

## 11. CGI execution

Scripts under `resources/cgi-bin/` (configured via `cgi_pass .py /usr/bin/python3`
in `default.conf`):

- `hello.py` — GET demo; prints query string + key env vars (`REQUEST_METHOD`,
  `SCRIPT_NAME`, `SERVER_NAME`, `cwd`).
- `echo.py` — POST demo; reads stdin and echoes it with `CONTENT_LENGTH`.
- `crash.py` — exits with an exception **before** writing any headers → 502.
- `loop.py` — infinite loop → killed by timeout (`Settings::CGI_TIMEOUT_MS`,
  10 s) → 504.
- `relpath.py` — opens a sibling file via `open("hello.py")` to prove the
  child runs with cwd = script directory.

### 11.1 Happy path — GET and POST

```bash
# GET, query string echoed
curl -i "http://127.0.0.1:8080/cgi-bin/hello.py?name=42"

# POST, body forwarded via stdin
curl -i -X POST -H "Content-Type: text/plain" \
  --data "echo this back" \
  http://127.0.0.1:8080/cgi-bin/echo.py
```

### 11.2 CGI runs in the correct directory (relative paths work)

```bash
curl -i http://127.0.0.1:8080/cgi-bin/relpath.py
# Response shows:
#   cwd: /home/.../resources/cgi-bin
#   read sibling 'hello.py' OK, first line: #!/usr/bin/env python3
```

### 11.3 Error handling — script that crashes / loops / is missing

```bash
curl -i http://127.0.0.1:8080/cgi-bin/crash.py     # 502 Bad Gateway
curl -i http://127.0.0.1:8080/cgi-bin/loop.py      # 504 (after ~10s)
curl -i http://127.0.0.1:8080/cgi-bin/nope.py      # 404
```

Server must still be alive afterwards:
```bash
pgrep -fa '^./webserv$'                            # same PID as before
curl -i http://127.0.0.1:8080/                     # 200
```

### 11.4 Multiple CGI systems (different extensions → different binaries)

The location's `cgi_pass` is a `(extension, handler)` pair stored in a
`std::map<std::string, std::string>`, so a single `location` can declare
as many CGI systems as you want. The bundled config wires up two:

```nginx
location /cgi-bin {
    cgi_pass .php /usr/bin/php-cgi;
    cgi_pass .py  /usr/bin/python3;
}
```

`HandlerFactory.cpp` extracts the request's extension via `extensionOf()` and
looks it up in the map.

```bash
# Python handler — .py
curl -i "http://127.0.0.1:8080/cgi-bin/hello.py?h=python"

# PHP handler — .php (script: resources/cgi-bin/hello.php)
curl -i "http://127.0.0.1:8080/cgi-bin/hello.php?h=php"

# PHP also forwards the request body via stdin
curl -i -X POST -H "Content-Type: text/plain" \
  --data "POSTed body" http://127.0.0.1:8080/cgi-bin/hello.php
```

Both responses come from the same server process — only the upstream binary
changes per request, picked off the `cgi_pass` map.

### 11.5 Server stays responsive while a CGI is hung

In one terminal, fire the looping CGI in the background; in the meantime
hit a normal route — it must return immediately:

```bash
curl -s http://127.0.0.1:8080/cgi-bin/loop.py &        # hangs ~10s
sleep 0.2
for i in 1 2 3 4 5; do
  curl -s -o /dev/null -w "concurrent #$i: %{http_code} (%{time_total}s)\n" \
    http://127.0.0.1:8080/
done
wait                                                   # background loop -> 504
```

Expected: each concurrent GET returns in < 5 ms; the loop returns 504 after
~10 s. Single-poll non-blocking I/O is doing its job.

---

## 12. HTTP redirection

```bash
curl -i http://127.0.0.1:8080/redirect           # 302 + Location: /
curl -iL http://127.0.0.1:8080/redirect          # follow the redirect
```

---

## 13. Directory listing (autoindex)

```bash
curl -i http://127.0.0.1:9090/potato/            # autoindex on -> HTML listing
curl -i http://127.0.0.1:8080/                   # autoindex off + index -> page
```

---

## 14. Config-file validation

The parser rejects malformed configs at startup with a clear error and refuses
to run. Quick demos (each requires editing `resources/default.conf` to that
state, since the binary loads the path baked into `Settings::DEFAULT_CONFIG_PATH`):

| Edit                                                         | Expected error                                    |
|--------------------------------------------------------------|---------------------------------------------------|
| Two `listen` directives in one server block                  | `duplicate 'listen' directive in server block`    |
| `listen 300.0.0.1:8080;`                                     | `invalid IPv4 address: 300.0.0.1`                 |
| Two server blocks on the same port                           | `duplicate port across server blocks: ...`        |
| `root /nonexistent;`                                         | `root: directory not found: /nonexistent`         |
| `upload_store /etc;`                                         | `upload_store: directory not writable: /etc`      |
| `cgi_pass .php /etc/hostname;`                               | `cgi_pass: not executable: /etc/hostname`         |
| `error_page 404 /no/such/file;`                              | `error_page: file not found: /no/such/file`       |
| `index missing.html;` under a real `root`                    | `index: file not readable: <root>/missing.html`   |

---

## 15. Stress / availability

```bash
# Apache bench: 1000 requests, 50 concurrent
ab -n 1000 -c 50 http://127.0.0.1:8080/

# siege equivalent
siege -b -c 50 -t 30s http://127.0.0.1:8080/
```

Server should remain responsive (no 5xx storms, no hangs) — re-curl after the
load test to confirm it's still serving.

---

## 16. Browser smoke test

Just open the site in any standard browser and click around:

```
http://127.0.0.1:8080/
```

Use the network panel to confirm `200`s, `Content-Type` headers, and the
status line.
