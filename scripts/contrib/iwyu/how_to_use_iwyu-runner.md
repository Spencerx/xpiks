# Install
First of all you need [install iwyu](https://github.com/include-what-you-use/include-what-you-use#how-to-install).

# Usage in xpiks code
To run `include what you use` in `xpiks` code, please use `iwyu-runner.sh` script.

## `iwyu-runner.sh` usage:

* run `qmake` command as usual,
* in directory with `Makefile` generated by `qmake` run:
```
path/to/iwyu-runner.sh directory/to/check
```
For example, if you run `qmake` in `<repo-root>/build/`, and you want to check `src/xpiks-qt/Artworks/`:
```
../scripts/contrib/iwyu/iwyu-runner.sh ../src/xpiks-qt/Artworks/
```


# Output analize
* `iwyu` output is well described, so additonal advice isn't nessesary
* `iwyu-runner.sh` prints output only for files that need includes cleanup
* sometimes [pragmas](https://github.com/include-what-you-use/include-what-you-use/blob/master/docs/IWYUPragmas.md) is needed if `iwyu` have problem with code analyze