from pathlib import Path
from typing import Any, cast

if "Import" not in globals():
    raise RuntimeError("This script must be run by PlatformIO/SCons")

Import = cast(Any, globals()["Import"])
Import("env")
env = cast(Any, globals()["env"])


def _load_dotenv(dotenv_path: Path) -> dict:
    values = {}
    if not dotenv_path.exists():
        return values

    for raw_line in dotenv_path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key:
            values[key] = value

    return values


def _to_cpp_define_string(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return '\\"{}\\"'.format(escaped)


project_dir = Path(env.subst("$PROJECT_DIR"))
dotenv_values = _load_dotenv(project_dir / ".env")

ssid = dotenv_values.get("WIFI_SSID", "")
password = dotenv_values.get("WIFI_PASSWORD", "")
timezone = dotenv_values.get("APP_TIMEZONE", "")

if not timezone:
    raise RuntimeError("APP_TIMEZONE must be set in .env")

cpp_defines = []

if ssid and password:
    cpp_defines.extend(
        [
            ("WIFI_SSID", _to_cpp_define_string(ssid)),
            ("WIFI_PASSWORD", _to_cpp_define_string(password)),
        ]
    )

cpp_defines.append(("APP_TIMEZONE", _to_cpp_define_string(timezone)))

if cpp_defines:
    env.Append(CPPDEFINES=cpp_defines)
