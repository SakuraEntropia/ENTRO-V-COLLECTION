#!/usr/bin/env bash
# Build, inspect, and optionally install the macOS plugin bundles.
# Usage:
#   ./scripts/package-macos.sh [--build DIR] [--install] [--zip]
# The script never fabricates a format that was not actually built.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/../work/build-triad"
DO_INSTALL=0
DO_ZIP=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build)   BUILD_DIR="$(cd "$2" && pwd)"; shift 2 ;;
        --install) DO_INSTALL=1; shift ;;
        --zip)     DO_ZIP=1; shift ;;
        -h|--help)
            sed -n '2,8p' "${BASH_SOURCE[0]}"; exit 0 ;;
        *) echo "unknown argument: $1" >&2; exit 2 ;;
    esac
done

if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "build directory not found: ${BUILD_DIR}" >&2
    exit 1
fi

# target name -> displayed product name (bash 3.2 on macOS lacks associative arrays)
INSTRUMENTS=("Matriarch" "PolyBrute" "MoogOne")
product_name() {
    case "$1" in
        Matriarch) echo "MATRIARCH V" ;;
        PolyBrute) echo "POLYBRUTE V" ;;
        MoogOne)   echo "ONE V" ;;
        *)         echo "$1" ;;
    esac
}
STAGE="${ROOT}/dist"
mkdir -p "${STAGE}"

found_any=0
for name in "${INSTRUMENTS[@]}"; do
    base="${BUILD_DIR}/${name}_artefacts/Release"
    product="$(product_name "$name")"
    for kind in VST3 AU Standalone; do
        case "${kind}" in
            VST3) src="${base}/VST3/${product}.vst3" ;;
            AU)   src="${base}/AU/${product}.component" ;;
            Standalone) src="${base}/Standalone/${product}.app" ;;
        esac
        if [[ -e "${src}" ]]; then
            rm -rf "${STAGE}/$(basename "${src}")"
            ditto "${src}" "${STAGE}/$(basename "${src}")"
            found_any=1
            echo "staged $(basename "${src}")"
        else
            echo "missing ${src}" >&2
        fi
    done
done

if [[ ${found_any} -eq 0 ]]; then
    echo "no artefacts found under ${BUILD_DIR}" >&2
    exit 1
fi

echo
echo "Bundle architectures and signatures:"
for bundle in "${STAGE}"/*.vst3 "${STAGE}"/*.component "${STAGE}"/*.app; do
    [[ -e "${bundle}" ]] || continue
    echo "--- $(basename "${bundle}")"
    if [[ -d "${bundle}/Contents/MacOS" ]]; then
        lipo -info "${bundle}/Contents/MacOS/"* 2>/dev/null || true
    fi
    codesign -dv "${bundle}" 2>&1 | sed -n '1,4p' || true
done

if [[ ${DO_INSTALL} -eq 1 ]]; then
    mkdir -p "${HOME}/Library/Audio/Plug-Ins/VST3" "${HOME}/Library/Audio/Plug-Ins/Components"
    for name in "${INSTRUMENTS[@]}"; do
        product="$(product_name "$name")"
        if [[ -d "${STAGE}/${product}.vst3" ]]; then
            rm -rf "${HOME}/Library/Audio/Plug-Ins/VST3/${product}.vst3"
            ditto "${STAGE}/${product}.vst3" "${HOME}/Library/Audio/Plug-Ins/VST3/${product}.vst3"
        fi
        if [[ -d "${STAGE}/${product}.component" ]]; then
            rm -rf "${HOME}/Library/Audio/Plug-Ins/Components/${product}.component"
            ditto "${STAGE}/${product}.component" "${HOME}/Library/Audio/Plug-Ins/Components/${product}.component"
        fi
        if [[ -d "${STAGE}/${product}.app" ]]; then
            rm -rf "/Applications/${product}.app"
            ditto "${STAGE}/${product}.app" "/Applications/${product}.app" || {
                echo "could not copy ${product}.app to /Applications (permission denied?)" >&2
            }
        fi
    done
    echo "installed VST3/AU for the current user; standalone apps copied when permitted."
    echo "restart the DAW or rescan plugins before use."
fi

if [[ ${DO_ZIP} -eq 1 ]]; then
    archive="${STAGE}/AnalogTriad-macOS-$(date +%Y%m%d).zip"
    rm -f "${archive}"
    (cd "${STAGE}" && zip -qry "${archive}" ./*.vst3 ./*.component ./*.app 2>/dev/null) || true
    echo "wrote ${archive}"
fi

echo "done."
