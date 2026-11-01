#!/bin/sh
set -eu

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

configured_hooks_path=$(git config --get core.hooksPath || true)
if [ -n "$configured_hooks_path" ]; then
    if [ "$configured_hooks_path" = ".githooks" ]; then
        printf '%s\n' "Repository pre-push hook is already enabled."
        exit 0
    fi

    printf 'Refusing to replace existing core.hooksPath: %s\n' \
        "$configured_hooks_path" >&2
    exit 1
fi

hooks_dir=$(git rev-parse --git-path hooks)
if [ -e "$hooks_dir/pre-push" ]; then
    printf 'Refusing to replace existing hook: %s/pre-push\n' "$hooks_dir" >&2
    exit 1
fi

git config --local core.hooksPath .githooks
printf '%s\n' "Enabled repository pre-push hook. Pushes will build and test with Meson."
