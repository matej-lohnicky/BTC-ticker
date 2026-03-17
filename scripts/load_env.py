import importlib.util
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


def _load_weather_coordinates_module(project_dir: Path) -> Any:
    module_path = project_dir / "scripts" / "weather_coordinates.py"
    spec = importlib.util.spec_from_file_location("weather_coordinates", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError("Unable to load scripts/weather_coordinates.py")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


project_dir = Path(env.subst("$PROJECT_DIR"))
dotenv_values = _load_dotenv(project_dir / ".env")
weather_coordinates = _load_weather_coordinates_module(project_dir)

ssid = dotenv_values.get("WIFI_SSID", "")
password = dotenv_values.get("WIFI_PASSWORD", "")
timezone = dotenv_values.get("APP_TIMEZONE", "")
weather_city = dotenv_values.get("WEATHER_CITY", "")
weather_country = dotenv_values.get("WEATHER_COUNTRY", "")

if not timezone:
    raise RuntimeError("APP_TIMEZONE must be set in .env")

weather_latitude, weather_longitude = weather_coordinates.resolve_weather_coordinates(
    weather_city,
    weather_country,
)
weather_coordinates.write_weather_location_header(
    project_dir,
    weather_latitude,
    weather_longitude,
)

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
