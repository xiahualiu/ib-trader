# Repository instructions for Claude Code

## Python

All Python commands must use the project virtual environment. Source the venv before running any Python command: source /home/ibuser/workspace/.venv/bin/activate && <command>

## Workflow

1. **Do not assume user intent.** When reasoning about the user's request, do not fill in gaps or make assumptions — ask the user with AskUserQuestion instead.
2. For non-trivial changes, present a plan and get confirmation before editing.
3. For small changes that don't warrant a full plan, still ask before editing.
4. Use `EnterPlanMode` to design an implementation approach and get user approval before writing code for non-trivial changes.
5. Use `WebSearch` to look up current information on the internet when the answer depends on up-to-date knowledge (package versions, compatibility, API changes, etc.).
