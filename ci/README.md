# CI

`github-build.yml` is the GitHub Actions workflow for Linux + macOS builds.
It is parked here because the push token lacked the `workflow` scope. To
activate: `git mv ci/github-build.yml .github/workflows/build.yml` and push
with a token that has the `workflow` scope (or via the GitHub UI).
