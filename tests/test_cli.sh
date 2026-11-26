#!/usr/bin/env bash

set -eu

BIN="${1:-./fishby}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="$(cat "$ROOT_DIR/VERSION")"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf -- "$TMP_DIR"' EXIT

export XDG_DATA_HOME="$TMP_DIR"

PASSED=0

expect_success() {
    if "$BIN" "$@" >"$TMP_DIR/output" 2>&1; then
        PASSED=$((PASSED + 1))
    else
        echo "FAIL: expected success"
        cat "$TMP_DIR/output"
        exit 1
    fi
}

expect_failure() {
    status=0

    "$BIN" "$@" >"$TMP_DIR/output" 2>&1 || status=$?

    if [ "$status" -ne 1 ]; then
        echo "FAIL: expected exit code 1, got $status"
        cat "$TMP_DIR/output"
        exit 1
    fi

    PASSED=$((PASSED + 1))
}

expect_success --help
grep -q 'Usage: fishby' "$TMP_DIR/output"

expect_success -h

expect_success --version
grep -Fxq "fishby $VERSION" "$TMP_DIR/output"

expect_success -v
grep -Fxq "fishby $VERSION" "$TMP_DIR/output"

expect_failure --unknown
expect_failure --fps
expect_failure --fps 0
expect_failure --fps 9
expect_failure --fps 121
expect_failure --fps -1
expect_failure --fps abc
expect_failure --fps 30abc
expect_failure --fps ""
expect_failure --fps 999999999999999999999999

expect_failure --name
expect_failure --name ""
expect_failure --name "name with spaces"
expect_failure --name "invalid/name"
expect_failure --name $'\033[31mBob'
expect_failure --name "abcdefghijklmnopqrstuvwxyz"

if [ -e "$TMP_DIR/fishby" ]; then
    echo "FAIL: unexpected persistent state"
    exit 1
fi

echo "PASS: CLI regression tests ($PASSED)"
