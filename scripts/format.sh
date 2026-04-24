set -euo pipefail
find src include tests -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format-17 -i {} +
