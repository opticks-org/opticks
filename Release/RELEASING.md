# Creating a new support release or release candidate

1. Ensure all issues have been merged into the appropriate support branch
1. Ensure all issues in the issue tracker are assigned to the appropriate release milestone, are marked as done, and were merged
1. Optional: Perform a test build (release mode) for each platform and fix any merge/build errors
1. Update the build information and push the changes. `build.py --update-version=rc --new-version=X.Y.ZrcW --release-date=YYYY-MM-DD` (or `--release-date=today`)
1. Run the dependency downloader for the appropriate platforms.
1. Build release mode `build.py -d /path/to/deps --arch=64 --mode=release --build-opticks=core`
1. Build developer docs `build.py -d /path/to/deps --build-doxygen=html` (or `--build-doxygen=all` on Windows)
1. Build the installer(s) for the platform.
1. Smoke test the installer and build for basic functionality. (install, open, check message log, load data, execute a couple of algorithms)
1. Commit the version changes (ensure only version number changes are committed) and tag with the release number (vX.Y.ZrcW)
1. Upload and update the website. (installers to github. installers, SDK, and optionally docs to opticks.org)
1. Make a release announcement.
1. Close the milestone in the issue tracker.

# Creating a final production release

1. Ensure a clean working copy of the release candidate.
1. Update the build information and push the changes. `build.py --update-version=production --new-version=X.Y.Z --release-date=YYYY-MM-DD` (or `--release-date=today`)
1. Build debug mode `build.py -d /path/to/deps --arch=64 --mode=debug --build-opticks=core`
1. Build release mode `build.py -d /path/to/deps --arch=64 --mode=release --build-opticks=core`
1. Build developer docs `build.py -d /path/to/deps --build-doxygen=html` (or `--build-doxygen=all` on Windows)
1. Build the installer(s) for the platform.
1. Smoke test the installer and build for basic functionality. (install, open, check message log, load data, execute a couple of algorithms)
1. Commit the version changes (ensure only version number changes are committed) and tag with the release number (vX.Y.Z)
1. Upload and update the website. (installers to github. installers, SDK, and docs to opticks.org)
1. Change "latest" sdk and doc pointers on opticks.org
1. Make a release announcement.
1. Close the milestone in the issue tracker.
