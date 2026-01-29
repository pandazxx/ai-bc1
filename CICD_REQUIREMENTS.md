# CI/CD Requirements

- CI/CD runs in GitHub Actions.
- Run unit tests whenever a tag matching `release/*` is pushed.
- Build a WebGL package whenever a tag matching `release/*` is pushed.
- Upload the WebGL build to a configurable S3 bucket.
