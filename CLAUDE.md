# Repository instructions for Claude Code

## Python

All Python commands must use the project virtual environment. Source the venv before running any Python command: source /workspace/.venv/bin/activate && <command>

## Workflow

1. **Do not assume user intent.** When reasoning about the user's request, do not fill in gaps or make assumptions — ask the user with AskUserQuestion instead.
2. For non-trivial changes, present a plan and get confirmation before editing.
3. For small changes that don't warrant a full plan, still ask before editing.
