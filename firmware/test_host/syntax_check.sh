#!/bin/bash
# Syntax-checks the ESP32 hardware modules against a minimal Arduino mock (no toolchain needed).
# This catches typos and type errors; it does not prove the code builds with the real core.
cd "$(dirname "$0")"
set -e
for f in ../shuffler/platform.cpp ../shuffler/hardware.cpp ../shuffler/motion.cpp ../shuffler/storage.cpp ../shuffler/cli.cpp; do
  echo "== $f"; g++ -std=c++17 -fsyntax-only -Wall -Wno-unused-function -I mock -I ../shuffler "$f"
done
echo "== ../shuffler/shuffler.ino"; cp ../shuffler/shuffler.ino /tmp/shuffler_ino.cpp
g++ -std=c++17 -fsyntax-only -Wall -Wno-unused-function -I mock -I ../shuffler /tmp/shuffler_ino.cpp
echo "syntax OK"
