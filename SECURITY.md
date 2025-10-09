# security

## what it covers

- api key management — secure storage in .env file
- data transmission — https encryption via curl
- error handling — prevents sensitive info leakage
- input validation — basic checks on user prompts

## best practices

- never commit api keys to version control
- use strong, unique api keys
- keep dependencies updated
- run in isolated environments

## reporting vulnerabilities

report security issues to [security@example.com] or open a private issue.

## known considerations

- alpha version — audit code before production use
- no advanced encryption — rely on https
- local key storage — protect .env file access 