# shallow clone WSL2 repo
git clone --depth 1 https://github.com/microsoft/WSL2-Linux-Kernel.git

# snoop around
cd tools/perf
cat Makefile.perf

# install dependencies
sudo apt-get install flex
sudo apt-get install bison

# snoop around extra options mentioned in logper
apt-cache search libelf-dev
apt-cache search systemtap-sdt-dev
apt-cache search libssl-dev
apt-cache search libslang2-dev
apt-cache search liblzma-dev
apt-cache search libnuma-dev
apt-cache search libbabeltrace-ctf-dev

# install extra
sudo apt-get install libelf-dev
sudo apt-get install systemtap-sdt-dev
sudo apt-get install libssl-dev
sudo apt-get install libslang2-dev
sudo apt-get install liblzma-dev
sudo apt-get install libnuma-dev
sudo apt-get install libbabeltrace-ctf-dev

# create destination dir and neat symlink
sudo mkdir /opt/perf-4.19
sudo ln -sT perf-4.19 /opt/perf/

# make
sudo make prefix=/opt/perf-4.19 install

# check result
/opt/perf/bin/perf --version

# add symbolic link
sudo ln -sT /opt/perf/bin/perf /usr/local/bin/perf
which -a perf

# it's still looking at /usr/bin/perf first !
# remove symbolic link
sudo rm /usr/local/bin/perf
which -a perf

# put binary on PATH
echo $PATH
# echo "${PATH:+${PATH}:}/opt/perf/bin/"
echo "/opt/perf/bin/${PATH:+:${PATH}}"

# don't do it like that, all variable would be expanded before appending to file
# echo "# perf path" >> ~/.profile
# echo "export PATH=\"/opt/perf/bin/${PATH:+:${PATH}}\"" >> ~/.profile

# add it directly
nano ~/.profile

# scratch that !  take advantage of $HOME/bin and put a symbolic link there
mkdir $HOME/bin
source .profile
ln -sT /opt/perf/bin/perf $HOME/bin/perf
which -a perf

# check result
perf --version