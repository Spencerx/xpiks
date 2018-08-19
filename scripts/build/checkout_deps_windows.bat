@echo off
rem fake submodule (exists only for appveyor tests)

cd ..\

git clone --depth=10 --branch=vs2015 https://bitbucket.org/ribtoks/xpiks-deps.git

cd xpiks-deps

git checkout e78792f
