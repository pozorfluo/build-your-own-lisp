#!/bin/bash

# mostly a playground file

# say hello to : Git
# see https://www.digitalocean.com/community/tutorials/how-to-install-git-on-debian-10
# see https://www.taniarascia.com/getting-started-with-git/
# see https://www.taniarascia.com/how-to-create-and-use-bash-scripts/
# see https://github.github.com/training-kit/downloads/github-git-cheat-sheet/

git config --global user.name "spagbol"
git config --global user.email "38818825+pozorfluo@users.noreply.github.com"

# git config --global credential.helper wincred

nano ~/.gitconfig

# getting glibc
cd ~/_source/C
# mkdir glibc && cd glibc
git clone git://sourceware.org/git/glibc.git
cd glibc
git checkout master
# git checkout release/2.30/master

find -name stdio.h
# see https://www.gnu.org/software/libc/manual/html_node/index.html

# retrieve some template from windows user google drive
# translate path from windows <=> wsl
# see https://devblogs.microsoft.com/commandline/share-environment-vars-between-wsl-and-windows/
# see https://adamtheautomator.com/windows-subsystem-for-linux/#sharing-environment-variables
# with WSLENV
# cmd.exe --% /C "set WUSER=%USERPROFILE% & set WSLENV=WUSER/p"
# see https://superuser.com/questions/1271205/how-to-get-the-host-user-home-directory-in-wsl-bash
# with wslpath

echo "${WINTEMPLATES}"/.clang-format
cp .clang-format "${WINTEMPLATES}"/.clang-format

export WINHOME=$(wslpath $(cmd.exe /C "echo %USERPROFILE%"))
echo $WINHOME

# this bit assumes that %GEN% is set correctly on the windows side
export WINGEN=$(wslpath $(cmd.exe /C "echo %GEN%"))
echo $WINGEN

# remove newline from path retrieved from the windows side
# see https://stackoverflow.com/questions/19345872/how-to-remove-a-newline-from-a-string-in-bash
# ${parameter//find/replace}
# CLEANED=${WINGEN//[$'\t\r\n']}
# echo "|$CLEANED|"
# WINTEMPLATES+="/_templates"
WINTEMPLATES="${WINGEN//[$'\t\r\n']}/_templates/C/default"
echo $WINTEMPLATES

# create new project folder
NEWPROJECT="build-your-own-lisp"
cd ~/_source/C
mkdir $NEWPROJECT && cd $NEWPROJECT

# copy content from template folder to new project folder
# see https://ss64.com/bash/cp.html
# see https://stackoverflow.com/questions/4175264/bash-retrieve-absolute-path-given-relative
# we need to copy hidden files and folders, that is those starting with a .
# see https://superuser.com/questions/61611/how-to-copy-with-cp-to-include-hidden-files-and-hidden-directories-and-their-con
echo "copy ${WINTEMPLATES} -> $(pwd)"
# ~/_source/C/${NEWPROJECT}
# cp -R ${WINTEMPLATES}/* .
cp -R "${WINTEMPLATES}/." " ~/_source/C/${NEWPROJECT}"
ls -a

# create remote repo on github
# see https://help.github.com/en/github/importing-your-projects-to-github/adding-an-existing-project-to-github-using-the-command-line
USERNAME="pozorfluo"

# init repo
# add, push
# see https://git-scm.com/book/en/v2/Git-Basics-Working-with-Remotes
git init
git add .
git commit -m "first commit"
git remote add origin https://github.com/${USERNAME}/${NEWPROJECT}.git
git push -u origin master


# vs code settings folder
cd /mnt/c/Users/porte/AppData/Roaming/Code/User

git remote add origin https://github.com/${USERNAME}/vsc-settings.git
git remote show origin
git push -u origin master

# check working directory, staging area
git status

# stage modified and untracked
# git add . && git add -u.
git add -A

# commit, push
git commit -m 'removed Makefile from .gitignore'

git push -u origin master
