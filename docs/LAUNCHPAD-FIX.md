# Launchpad icon refresh — 2026-10-25

Reported symptom: blank/default icons in Launchpad.

Observed before repair:

- All four `/Applications` bundles had valid signatures, correct `CFBundleIconFile`, and decodable non-empty ICNS artwork.
- macOS file-icon service returned the intended artwork, so missing icon files were not the diagnosed problem.
- LaunchServices also registered eight development/distribution copies alongside the four installed apps.
- Installed bundle directory dates were retained from the initial build, older than the replaced executables.

Applied a scoped refresh: unregistered the eight workspace copies, touched and force-registered the four installed apps, and restarted Dock. No Launchpad database, icon order, backup or application file was deleted. The installer now refreshes these registrations after each successful replacement.

Verification: Python syntax checks; real helper execution; installed bundle timestamps; re-read LaunchServices registrations; rendered file-icon output; application signatures. The native UI inspection service timed out, so direct visual confirmation inside Launchpad is left to the user rather than claimed as an automated pass.
