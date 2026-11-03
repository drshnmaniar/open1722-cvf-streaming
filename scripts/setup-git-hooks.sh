#!/bin/sh
# filepath: /home/drshn/project/open1722-cvf-streaming/scripts/setup-git-hooks.sh
set -eu

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

for hook in pre-commit pre-push; do
    if [ ! -x "$repo_root/.githooks/$hook" ]; then
        printf 'Missing or non-executable .githooks/%s\n' "$hook" >&2
        exit 1
    fi
done

configured_hooks_path=$(git config --get core.hooksPath || true)
if [ -n "$configured_hooks_path" ]; then
    if [ "$configured_hooks_path" = ".githooks" ]; then
        printf '%s\n' "Repository pre-commit and pre-push hooks are already enabled."
        exit 0
    fi

    printf 'Refusing to replace existing core.hooksPath: %s\n' \
        "$configured_hooks_path" >&2
    exit 1
fi

hooks_dir=$(git rev-parse --git-path hooks)
for hook in pre-commit pre-push; do
    if [ -e "$hooks_dir/$hook" ]; then
        printf 'Refusing to replace existing hook: %s/%s\n' "$hooks_dir" "$hook" >&2
        exit 1
    fi
done

git config --local core.hooksPath .githooks
printf '%s\n' "Enabled repository pre-commit and pre-push hooks."