<?php
header("Content-Type: text/plain");
echo "Hello from PHP CGI!\n";
echo "method=" . $_SERVER['REQUEST_METHOD'] . "\n";
echo "query=" . ($_SERVER['QUERY_STRING'] ?? '') . "\n";
echo "cwd=" . getcwd() . "\n";
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $body = file_get_contents("php://stdin");
    echo "--- body ---\n$body\n";
}
