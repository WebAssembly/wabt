#!/bin/bash

set -o errexit
set -o pipefail

if [ -n "$1" ]; then
  BRANCH="$1"
elif [ "$CI" != "true" ]; then
  echo "Please specify a base branch in the command line"
  exit 1
elif [ -n "$GITHUB_EVENT_BEFORE" ] && [ "push" = "$GITHUB_EVENT_NAME" ]; then
  BRANCH="$GITHUB_EVENT_BEFORE"
elif [ -n "$GITHUB_BASE_REF" ]; then
  BRANCH="origin/$GITHUB_BASE_REF"
elif git symbolic-ref -q HEAD; then # check if we're in a branch
  BRANCH="@{upstream}"
else
  # in a detached HEAD.
  # default to origin/main, this is a "last resort" to make this script do the
  # right thing, and is only really here so it works when pushing a new branch,
  # with the caveat that it assumes the base branch to be called "main".
  # (this has been the case with wabt for a while. may fail if the repo lacks a
  # "main" branch for some reason.)
  BRANCH="origin/main"
fi

MERGE_BASE=$(git merge-base $BRANCH HEAD)
FORMAT_MSG=$(git clang-format $MERGE_BASE -q --diff -- src/)
if [ -n "$FORMAT_MSG" -a "$FORMAT_MSG" != "no modified files to format" ]
then
  echo "Please run git clang-format before committing, or apply this diff:"
  echo
  # Run git clang-format again, this time without capruting stdout.  This way
  # clang-format format the message nicely and add color.
  git clang-format $MERGE_BASE -q --diff -- src/
  exit 1
fi
