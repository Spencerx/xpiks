@echo off

cd ..\
mkdir xpiks-deps
cd xpiks-deps

git init
git remote add origin https://bitbucket.org/ribtoks/xpiks-deps.git

git fetch --depth=30 origin master
git merge 76c80a64ff15d6f67cabf37e4120d1a0c19a5e31
