#!/usr/bin/env python3
"""Refresh only this collection's app registrations; preserve Launchpad layout."""
import argparse
import os
import re
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parents[1]
LSREGISTER = '/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister'
PRODUCTS = {'MATRIARCH V': 'Matriarch', 'POLYBRUTE V': 'PolyBrute',
            'ONE V': 'MoogOne', 'CONTINUUM V': 'Continuum'}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--restart-dock', action='store_true')
    args = parser.parse_args()
    if sys.platform != 'darwin':
        parser.error('Requires macOS')
    installed = [Path('/Applications') / (name + '.app') for name in PRODUCTS]
    for app in installed:
        if not (app / 'Contents/Info.plist').is_file():
            raise RuntimeError(f'Missing installed application: {app}')
    result = subprocess.run([LSREGISTER, '-dump'], check=True, capture_output=True, text=True)
    identifiers = {'com.sakuraentropia.' + value for value in PRODUCTS.values()}
    paths = set()
    for block in result.stdout.split('--------------------------------------------------------------------------------'):
        fields = {}
        for line in block.splitlines():
            key, separator, value = line.partition(':')
            if separator and key.strip() in ('path', 'identifier'):
                fields[key.strip()] = value.strip()
        if fields.get('identifier') not in identifiers:
            continue
        path = Path(re.sub(r' \(0x[0-9a-fA-F]+\)$', '', fields.get('path', ''))).resolve()
        if path.is_relative_to(WORKSPACE) and path.name in {p.name for p in installed}:
            paths.add(path)
    for path in sorted(paths):
        subprocess.run([LSREGISTER, '-u', str(path)], check=True, capture_output=True)
        print('Unregistered development copy:', path)
    for app in installed:
        # ditto preserves old bundle directory dates. Invalidate that cache key
        # after replacing the executable/resources, then force registration.
        os.utime(app, None)
        subprocess.run([LSREGISTER, '-f', str(app)], check=True, capture_output=True)
        subprocess.run(['codesign', '--verify', '--deep', '--strict', str(app)], check=True, capture_output=True)
        print('Refreshed:', app)
    if args.restart_dock:
        result = subprocess.run(['killall', 'Dock'], capture_output=True, text=True)
        if result.returncode and 'No matching processes' not in result.stderr:
            raise RuntimeError(result.stderr)
        print('Dock refresh requested; Launchpad layout was not reset.')

if __name__ == '__main__':
    main()
