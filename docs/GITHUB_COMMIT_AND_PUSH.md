# GitHub: Cleanup and Push Checklist

Use this when wrapping up a phase and pushing both repos to GitHub.

---

## 1. Packaging cleanup (both repos)

- **.gitignore** — Ensure `build/`, `output/` (if you don’t want to commit curve data), `venv/`, `.DS_Store`, IDE folders are ignored. Both repos already have suitable .gitignore.
- **No secrets** — No API keys, credentials, or machine-specific paths in committed files.
- **Optional:** Remove or don’t add `build/`, `output/`, `*.zip` (distribution zip) so the repo stays small. Compressor: `output/` is optional (uncomment in .gitignore if you want to exclude it). Inspector: `output/` is already ignored.

---

## 2. Ombic Compressor

From the **ombic-compressor** directory:

```bash
cd /path/to/ombic-compressor
git status
git add .
# Review: git diff --staged
git commit -m "Phase wrap: GUI spec updates, distribution zip, docs archive and design backlog"
git push origin main
```

If the remote doesn’t exist yet:

```bash
git remote add origin https://github.com/YOUR_USERNAME/ombic-compressor.git
git push -u origin main
```

---

## 3. Ombic VST Inspector

From the **ombic-vst-inspector** directory:

```bash
cd /path/to/ombic-vst-inspector
git status
git add .
# Review: git diff --staged
git commit -m "Phase wrap: docs archive, sync with two-repo layout"
git push origin main
```

If the remote doesn’t exist yet:

```bash
git remote add origin https://github.com/YOUR_USERNAME/ombic-vst-inspector.git
git push -u origin main
```

---

## 4. (Optional) VST folder

If the old **VST** folder is still a git repo and you want to record the deprecation:

```bash
cd /path/to/VST
git add README.md docs/ARCHIVE_ORIGINAL_VST_REPO.md
git commit -m "Deprecate: point to ombic-compressor and ombic-vst-inspector"
git push origin main   # if you have a remote for it
```

Otherwise you can leave the VST folder as a local-only stub and remove it from your Cursor workspace.

---

## 5. Workspace

In Cursor, remove **VST** from the workspace and keep only **ombic-compressor** and **ombic-vst-inspector** as roots so you’re only working in two folders.
