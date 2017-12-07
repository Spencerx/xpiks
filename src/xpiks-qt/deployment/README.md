# Xpiks deployment

## Prerequisites

1. Choose your platform's deploy script
2. Make sure Qt's `bin/` directory is in your `$PATH`
3. Verify build directory path (Release version and correct Qt's version)
4. Verify Xpiks version string
5. Update dependencies

## Updating dependencies

Dependencies used by Xpiks:

* _libcurl_ (from [main website](https://curl.haxx.se/libcurl/), Windows build from [GitHub repository](https://github.com/blackrosezy/build-libcurl-windows))
* _hunspell_ (from GitHub submodule)
* _zlib_ (from [main website](https://zlib.net/))
* _exiftool_ (latest production release from [main website](https://www.sno.phy.queensu.ca/~phil/exiftool/history.html))
* _tiny-AES_ (from GitHub submodule)
* _sqlite_ (from [main website](https://www.sqlite.org/))
* _ffmpeg_ (from [main website](https://www.ffmpeg.org/download.html))
* _openssl_ (LTS from [main website](https://www.openssl.org/source/))
* _ministaller_ [Windows] (from GitHub repo)

1. `utils/` directory contains basic scripts to build complex dependencies
2. `BitBucket:xpiks-deps` repo contains other scripts and build libraries

## Post-deploy

1. Calculate sha1 sum for created packages (`shasum -a 1 filepath` for macOS)
