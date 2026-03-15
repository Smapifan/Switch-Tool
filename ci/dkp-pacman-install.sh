#!/usr/bin/env bash
# ci/dkp-pacman-install.sh
# Robust dkp-pacman database sync + package install with retry / exponential backoff.
#
# Usage: bash ci/dkp-pacman-install.sh [pkg1 pkg2 ...]
#   Defaults to: switch-sdl2 switch-mesa
#
# Exit codes:
#   0  – all packages installed (or already present)
#   1  – sync or install failed after all retry attempts
set -euo pipefail

# ── Default packages ──────────────────────────────────────────────────────────
if [ "$#" -eq 0 ]; then
    set -- switch-sdl2 switch-mesa
fi

MAX_ATTEMPTS=6
INITIAL_SLEEP=5   # seconds; doubles each attempt → 5 10 20 40 80 160

# ── Helpers ───────────────────────────────────────────────────────────────────
log() { echo "[dkp-pacman-install] $*"; }

run_with_retry() {
    local desc="$1"; shift
    local attempt=1
    local sleep_time=$INITIAL_SLEEP
    local rc

    while [ "$attempt" -le "$MAX_ATTEMPTS" ]; do
        log "Attempt $attempt/$MAX_ATTEMPTS: $desc"
        rc=0
        # Use || so set -e does not abort; captures the real exit code.
        "$@" || rc=$?
        if [ "$rc" -eq 0 ]; then
            log "Success: $desc"
            return 0
        fi
        log "FAILED (exit $rc): $desc — sleeping ${sleep_time}s before retry"
        sleep "$sleep_time"
        attempt=$((attempt + 1))
        sleep_time=$((sleep_time * 2))
    done

    log "All $MAX_ATTEMPTS attempts failed: $desc"
    return 1
}

print_diagnostics() {
    log "=== DIAGNOSTICS ==="
    log "--- dkp-pacman version ---"
    dkp-pacman -V || true
    log "--- HTTP status: pkg.devkitpro.org ---"
    curl -sI --max-time 15 https://pkg.devkitpro.org/ || true
    log "--- mirrorlist(s) ---"
    for f in /etc/pacman.d/mirrorlist*; do
        [ -f "$f" ] && { log "  File: $f"; cat "$f"; } || true
    done
    log "=== END DIAGNOSTICS ==="
}

# ── Step 1: Database sync ──────────────────────────────────────────────────────
log "Syncing dkp-pacman package database..."
if ! run_with_retry "dkp-pacman -Sy --noconfirm" dkp-pacman -Sy --noconfirm; then
    print_diagnostics
    log "ERROR: Failed to sync package database after $MAX_ATTEMPTS attempts."
    exit 1
fi

# ── Step 2: Package install ────────────────────────────────────────────────────
log "Installing packages: $*"
if ! run_with_retry "dkp-pacman -S --needed --noconfirm $*" \
        dkp-pacman -S --needed --noconfirm "$@"; then
    print_diagnostics
    log "ERROR: Failed to install packages after $MAX_ATTEMPTS attempts."
    exit 1
fi

log "All packages installed successfully."
