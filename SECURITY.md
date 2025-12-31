# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 0.1.x | ✅ |

---

## Reporting a Vulnerability

If you discover a security vulnerability, please report it responsibly:

1. **DO NOT** open a public issue
2. Email details to the maintainer via [GitHub profile](https://github.com/itx-Luke)
3. Include:
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

**Response Time:** We aim to respond within 48 hours.

---

## Security Considerations

### API Keys

- **Never commit API keys** to version control
- Store `GEMINI_API_KEY` as environment variable
- The app reads keys from environment, not config files

### ADB Security

- Neo-Z uses ADB to communicate with emulators
- Only connects to locally-running emulator instances
- Does not expose ADB over network by default

### Input Handling

- Input hooks run with user-level permissions
- No kernel-level drivers are installed
- Raw input is processed locally, never transmitted

### Data Storage

- Configuration stored in user's `%APPDATA%` directory
- No sensitive data is logged
- Log files contain only diagnostic information

---

## Dependency Security

We use GitHub Dependabot to monitor dependencies. To review:

```powershell
# Check for known vulnerabilities
gh api /repos/itx-Luke/test/dependabot/alerts
```

---

## Best Practices for Contributors

1. **Validate all input** — Never trust external data
2. **Use parameterized commands** — Avoid shell injection in ADB commands
3. **Minimize permissions** — Request only what's needed
4. **Keep dependencies updated** — Review and update regularly
5. **Review security advisories** — Monitor Qt and CMake security bulletins
