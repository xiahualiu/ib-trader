# CLAUDE.md

## Python
Prefix Python commands with: `source /home/ibuser/workspace/.venv/bin/activate &&`

## Rules
1. Don't assume user intent — ask with `AskUserQuestion`.
2. Challenge unreasonable requests.
3. Get user confirmation before editing any code.
4. Use `EnterPlanMode` for non-trivial changes.
5. Use `WebSearch` when the answer depends on current info (versions, APIs, compatibility).
