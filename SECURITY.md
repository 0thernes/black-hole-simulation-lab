# Security Policy

## Reporting Vulnerabilities

This is a research-first scientific computing project. Most security risks
here are about secret hygiene and supply-chain trust rather than user data.
If you find something concerning, please open a private security advisory on
the GitHub repository:

`https://github.com/0thernes/Stress-Test-Agents-Maxxxing/security/advisories`

Do not file public issues for vulnerabilities until they are remediated.

## Secret Hygiene

- The `.gitignore` is the floor for blocking accidentally committed credentials.
  Patterns include `.env`, common credential basenames, key file extensions
  (`.pem`, `.pfx`, `.key`, `.jks`), and SSH key file names.
- If a secret is committed by accident:
  1. Rotate or revoke the secret upstream immediately. Treat the value as
     compromised the moment it touches Git history.
  2. Open an issue noting the file path and secret type only. Never paste
     the value into the issue, commit message, or chat.
  3. We will rewrite history with `git filter-repo` or `git filter-branch`
     after the upstream rotation is confirmed.
- Pre-commit secret scanning is planned for a future iteration. Until then,
  treat secret-scanning as a manual discipline.

## Supply Chain

- Third-party integrations live under `external/` and are tracked as
  submodules or pulled with CMake `FetchContent` at specific commits or
  tagged releases. Floating against `main` of an upstream is not acceptable
  for production releases.
- The tsotchke ecosystem repositories
  (`tsotchke/eshkol`, `tsotchke/quantum_geometric_tensor`,
  `tsotchke/libirrep`, etc.) are the planned dependency surface. Each
  integration must be added with an ADR.

## CI Trust

- GitHub Actions workflows run on `windows-latest`. Workflows must not pull
  third-party actions without a pinned commit SHA. `actions/checkout@v4`
  and `actions/setup-python@v5` are pinned by major version because they
  are first-party.

## Scope Out

- This is not a hardened production service. It is a research codebase. We
  do not currently consider denial-of-service of the simulator a security
  issue. Numerical-correctness bugs are tracked under quality assurance,
  not security, unless they affect the integrity of an audit gate.
