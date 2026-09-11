#!/bin/bash
# Applies RP2350 compatibility patches to submodules. Run once after clone.
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
PATCHES_DIR="$SCRIPT_DIR/patches"
echo "Applying RP2350 compatibility patches..."
cd "$REPO_ROOT/fatfs_lib"
if git apply --check "$PATCHES_DIR/fatfs_rp2350.patch" 2>/dev/null; then
    git apply "$PATCHES_DIR/fatfs_rp2350.patch"
    echo "OK: fatfs_rp2350.patch applied"
else
    echo "SKIP: fatfs_rp2350.patch already applied"
fi
echo "Done."
