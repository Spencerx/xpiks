# Install
First of all you need [install iwyu](https://github.com/include-what-you-use/include-what-you-use#how-to-install).

# Usage in xpiks code
To run `include what you use`, you need change your compiler to `iwyu`:

* run qmake command ass usual
* run `make` with following arguments:
```
make -k CXX="/usr/bin/iwyu -Xiwyu --transitive_includes_only -Xiwyu --mapping_file=../../tools/iwyu/mapping-file.imp -Xiwyu --check_also=*.h"
```
Of course path to `iwyu` and [mapping file](https://github.com/include-what-you-use/include-what-you-use/blob/master/docs/IWYUMappings.md) must be correct.

# Output analize
* Iwyu output is well described, so additonal advice isn't nessesary.
* `iwyu` is running for whole project, so you need ignore output for `moc_` files.
