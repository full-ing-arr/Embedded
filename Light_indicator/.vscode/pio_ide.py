"""Expose PlatformIO's existing CMake build model to VS Code CMake Tools."""
from pathlib import Path

Import("env")

query_dir = Path(env.subst("$BUILD_DIR")) / ".cmake" / "api" / "v1" / "query"
query_dir.mkdir(parents=True, exist_ok=True)
for query in ("codemodel-v2", "cache-v2", "cmakeFiles-v1", "toolchains-v1"):
    query_file = query_dir / query
    if not query_file.exists():
        query_file.touch()
