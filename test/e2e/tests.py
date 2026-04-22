#!/usr/bin/env python3

import socket
import sys
import http.client
from urllib.parse import urlparse

# ─────────────────────────────────────────────
#  CONFIGURATION
# ─────────────────────────────────────────────
BASE_URL = "https://www.w3.org/Protocols/HTTP/Performance/microscape/"

TIMEOUT = 5  # timeout seconds per connection

# ── Test Routes ──────────────────────────────
REDIRECT_ROUTE = "/redirect"
CGI_ROUTE = "/cgi-bin/test.cgi"

# ── ANSI Colors ──────────────────────────────
GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
BOLD   = "\033[1m"
RESET  = "\033[0m"
DIM    = "\033[2m"

# ── Global counters ──────────────────────────
results = {"passed": 0, "failed": 0, "skipped": 0}
test_log = []


# ═══════════════════════════════════════════════
#  LOW-LEVEL UTILITIES (http.client HTTP/1.1)
# ═══════════════════════════════════════════════

def parse_url(url: str):
    """Parses BASE_URL and returns (host, port, use_ssl, base_path)."""
    p = urlparse(url)
    host = p.hostname
    use_ssl = p.scheme == "https"
    port = p.port or (443 if use_ssl else 80)
    base_path = p.path if p.path else "/"
    return host, port, use_ssl, base_path


def raw_request(base_url: str, method: str, path: str, headers: dict = None,
                body: str = None, timeout: int = TIMEOUT) -> dict:
    """
    Sends an HTTP/1.1 request using http.client and returns a dict with:
        status_code, status_line, headers (dict), body (str), raw (bytes)
    """
    try:
        host, port, use_ssl, base_path = parse_url(base_url)
        
        # Join base path with request path
        full_path = base_path.rstrip("/") + "/" + path.lstrip("/")
        
        # Create connection
        if use_ssl:
            conn = http.client.HTTPSConnection(host, port, timeout=timeout)
        else:
            conn = http.client.HTTPConnection(host, port, timeout=timeout)
        
        # Prepare headers
        req_headers = headers.copy() if headers else {}
        if body is not None:
            if isinstance(body, str):
                body = body.encode()
            req_headers["Content-Length"] = str(len(body))
        
        # Send request
        conn.request(method, full_path, body=body, headers=req_headers)
        
        # Get response
        response = conn.getresponse()
        status_code = response.status
        status_line = f"HTTP/1.1 {response.status} {response.reason}"
        resp_headers = {k.lower(): v for k, v in response.getheaders()}
        resp_raw = response.read()
        resp_body = resp_raw.decode(errors="replace")
        
        conn.close()
        
        return {
            "status_code": status_code,
            "status_line": status_line,
            "headers": resp_headers,
            "body": resp_body,
            "raw": resp_raw,
            "error": None,
        }
    except socket.timeout:
        return {"error": "timeout", "status_code": None}
    except ConnectionRefusedError:
        return {"error": "connection_refused", "status_code": None}
    except Exception as e:
        return {"error": str(e), "status_code": None}


# ═══════════════════════════════════════════════
#  MINIMAL TEST FRAMEWORK
# ═══════════════════════════════════════════════

def test(name: str, passed: bool, detail: str = "", skip: bool = False):
    """Registers the result of a test."""
    if skip:
        results["skipped"] += 1
        symbol = f"{YELLOW}SKIP{RESET}"
        test_log.append(("skip", name, detail))
    elif passed:
        results["passed"] += 1
        symbol = f"{GREEN}PASS{RESET}"
        test_log.append(("pass", name, detail))
    else:
        results["failed"] += 1
        symbol = f"{RED}FAIL{RESET}"
        test_log.append(("fail", name, detail))

    status = f"[{symbol}]"
    detail_str = f"  {DIM}→ {detail}{RESET}" if detail else ""
    print(f"  {status} {name}{detail_str}")


def section(title: str):
    print(f"\n{CYAN}{BOLD}{'─'*60}{RESET}")
    print(f"{CYAN}{BOLD}  {title}{RESET}")
    print(f"{CYAN}{'─'*60}{RESET}")


# ═══════════════════════════════════════════════
#  HTTP 1.1 TESTS
# ═══════════════════════════════════════════════

def test_get_basic():
    section("GET — Basic requests")

    # GET root
    r = raw_request(BASE_URL, "GET", "/")
    test("GET / returns 200 or 3xx",
         r["status_code"] in (200, 301, 302, 308),
         f"status={r['status_code']}")

    # Mandatory Host header
    r = raw_request(BASE_URL, "GET", "/")
    test("Response includes Date header",
         "date" in r.get("headers", {}),
         f"headers={list(r.get('headers',{}).keys())[:5]}")

    # Content-Type present
    r = raw_request(BASE_URL, "GET", "/")
    test("Response includes Content-Type",
         "content-type" in r.get("headers", {}),
         f"content-type={r.get('headers',{}).get('content-type','ausente')}")

    # Request with invalid path
    r = raw_request(BASE_URL, "GET", "/ruta-que-no-existe-xyz")
    test("GET non-existent route returns 404",
         r["status_code"] == 404,
         f"status={r['status_code']}")

def test_head_method():
    section("HEAD Method")

    r_get  = raw_request(BASE_URL, "GET",  "/")
    r_head = raw_request(BASE_URL, "HEAD", "/")

    test("HEAD returns same status as GET",
         r_head["status_code"] == r_get["status_code"],
         f"HEAD={r_head['status_code']} GET={r_get['status_code']}")

    test("HEAD has no body",
         len(r_head.get("body", "").strip()) == 0,
         f"body_len={len(r_head.get('body',''))}")

    test("HEAD has headers",
         len(r_head.get("headers", {})) > 0,
         f"headers={list(r_head.get('headers',{}).keys())[:4]}")


def test_delete_method():
    section("DELETE Method")

    r = raw_request(BASE_URL, "DELETE", "/archivo-inexistente")
    test("DELETE non-existent resource → 404 or 405",
         r["status_code"] in (404, 405),
         f"status={r['status_code']}")


def test_status_codes():
    section("Error pages")

    r = raw_request(BASE_URL, "GET", "/esta-pagina-no-existe")
    test("404 has body (error page)",
         r["status_code"] == 404 and len(r.get("body", "")) > 0,
         f"status={r['status_code']} body_len={len(r.get('body',''))}")

    # URI too long → 414
    long_path = "/" + "a" * 8192
    r_long = raw_request(BASE_URL, "GET", long_path)
    test("URI too long → 414 or 400",
         r_long["status_code"] in (400, 414),
         f"status={r_long['status_code']}")

def test_response_headers_quality():
    section("Response headers quality")

    r = raw_request(BASE_URL, "GET", "/")
    h = r.get("headers", {})

    test("Header Server present",
         "server" in h,
         f"server={h.get('server','absent')}")

    test("Header Date present and not empty",
         "date" in h and len(h.get("date", "")) > 0,
         f"date={h.get('date','absent')}")

    test("Content-Type does not have malformed charset",
         "content-type" not in h or ";" not in h.get("content-type","") or "charset" in h.get("content-type",""),
         f"content-type={h.get('content-type','')}")


# ═══════════════════════════════════════════════
#  1. POST WITH BODY
# ═══════════════════════════════════════════════

def test_post_with_body():
    section("POST — Body variants")

    # 1a. POST with valid JSON body
    body    = '{"key": "value"}'
    headers = {"Content-Type": "application/json"}
    r = raw_request(BASE_URL, "POST", "/", headers=headers, body=body)
    test(
        "POST / with JSON body → not a server error (< 500)",
        r["status_code"] is not None and r["status_code"] < 500,
        f"status={r['status_code']}",
    )

    # 1b. POST with empty body (Content-Length: 0)
    r = raw_request(BASE_URL, "POST", "/", headers={"Content-Length": "0"}, body="")
    test(
        "POST / with empty body (CL=0) → server responds",
        r["status_code"] is not None,
        f"status={r['status_code']}",
    )

    # 1c. POST with form-encoded body
    form_body = "username=admin&password=secret"
    r = raw_request(
        BASE_URL, "POST", "/",
        headers={"Content-Type": "application/x-www-form-urlencoded"},
        body=form_body,
    )
    test(
        "POST / with form-encoded body → server responds",
        r["status_code"] is not None,
        f"status={r['status_code']}",
    )

# ═══════════════════════════════════════════════
#  MAIN RUNNER
# ═══════════════════════════════════════════════

def check_server_available():
    """Checks that the server is active before running tests."""
    host, port, _, _ = parse_url(BASE_URL)
    try:
        sock = socket.create_connection((host, port), timeout=3)
        sock.close()
        return True
    except Exception:
        return False


def print_summary():
    p = results["passed"]
    f = results["failed"]
    s = results["skipped"]
    total = p + f + s

    print(f"\n{BOLD}{'═'*60}{RESET}")
    print(f"{BOLD}  FINAL SUMMARY{RESET}")
    print(f"{'═'*60}")
    print(f"  {GREEN}✔ Passed : {p}{RESET}")
    print(f"  {RED}✘ Failed : {f}{RESET}")
    if s:
        print(f"  {YELLOW}⊘ Skipped: {s}{RESET}")
    print(f"  Total   : {total}")
    pct = int(100 * p / (p + f)) if (p + f) > 0 else 0
    bar_len = 40
    filled = int(bar_len * p / (p + f)) if (p + f) > 0 else 0
    bar = f"{GREEN}{'█' * filled}{RED}{'░' * (bar_len - filled)}{RESET}"
    print(f"\n  [{bar}] {pct}%")
    print(f"{'═'*60}\n")

    if f > 0:
        print(f"{BOLD}{RED}  Failed tests:{RESET}")
        for kind, name, detail in test_log:
            if kind == "fail":
                print(f"    {RED}✘{RESET} {name}")
                if detail:
                    print(f"      {DIM}{detail}{RESET}")
        print()


def run_all():
    print(f"\n{BOLD}{'═'*60}{RESET}")
    print(f"{BOLD}  HTTP/1.1 TEST SUITE{RESET}")
    print(f"{BOLD}  Target: {CYAN}{BASE_URL}{RESET}")
    print(f"{BOLD}{'═'*60}{RESET}")

    if not check_server_available():
        print(f"\n{RED}{BOLD}  ✘ Cannot connect to {BASE_URL}{RESET}")
        print(f"  Make sure the server is running.\n")
        raise RuntimeError(f"Cannot connect to {BASE_URL}")

    print(f"\n{GREEN}  ✔ Server available at {BASE_URL}{RESET}")

    # Run all tests
    test_get_basic()
    test_head_method()
    test_delete_method()
    test_status_codes()
    test_response_headers_quality()
    test_post_with_body()

    print_summary()
    return 0 if results["failed"] == 0 else 1


if __name__ == "__main__":
    # Allows passing the URL as an argument: python test_http.py http://localhost:4242
    if len(sys.argv) > 1:
        BASE_URL = sys.argv[1].rstrip("/")
    try:
        exit_code = run_all()
        sys.exit(exit_code)
    except RuntimeError as e:
        print(f"{RED}{BOLD}Error: {e}{RESET}")
        sys.exit(1)