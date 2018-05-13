@echo off
rem fake submodule (exists only for appveyor tests)

cd ..\

git clone --depth=10 --branch=vs2015 https://bitbucket.org/ribtoks/xpiks-deps.git

cd xpiks-deps

git checkout 63b951d437c7b652c7bcefcd117d7d23724de4f3
