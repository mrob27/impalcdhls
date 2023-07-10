# Introduction

These instructions are for Fedora 35 and assume you are performing
work for academic research (required by the LegUp dependency)

These instructions are adapted from earlier work in the CAAD lab,
which ws published at github.com/HafsahShahzad/HLS

The "zeroth" step is:

0. If you are on Fedora 35, the instructions below should work. If you
have some other Linux, you will probably encounter problems, which you
will need to work out.


## Build process

1. Install the following dependencies
```
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake tcl-devel suitesparse-devel mysql-devel \
  lpsolve-devel libtool gmp-devel g++ glibc-devel.i686 libnsl
```


2. Create a symlink for liblpsolve55
```
sudo ln -s /usr/lib64/liblpsolve55.so /usr/lib64/liblpsolve55_pic.so
```


3. Modify line 259 in `/usr/include/mysql/mariadb_stmt.h`
```
sudo gedit /usr/include/mysql/mariadb_stmt.h
```
by typecasting `MYSQL_TYPE_GEOMETRY`
```
extern MYSQL_PS_CONVERSION mysql_ps_fetch_functions[(int)MYSQL_TYPE_GEOMETRY + 1];
```


4. Use `git` to get a copy of this project:
```
git clone https://github.com/asanaullah/legup
```

The `git` command creates a directory `legup` within whatever directory
you were in. Make a note of where this is, for step 9.


5. You also need to get the `autophase` submodule (a dependency in a
subdirectory) which `git` does not download unless you specifically request:
```
cd legup
git submodule update --init --recursive
```


6. From within the `legup` directory just created, run `build-gcc-and-llvm.sh`
```
chmod +x build-gcc-and-llvm.sh
./build-gcc-and-llvm.sh
```


7. Get the LegUP archive `legup-4.0.tar.gz`:

Start at http://legup.eecg.utoronto.ca and read the instructions. There
are restrictions on the use of LegUP which you must agree to; after you do
this it will let you download `legup-4.0.tar.gz`.

Once you have `legup-4.0.tar.gz` put it in the same directory as the
script `build-legup.sh`.

To make sure this is the right version of the tarfile, you can perform
a few simple tests:

```
  # The following command should show "30125"
  cat legup-4.0.tar.gz | gunzip | tar tvf - | wc -l
  # The following command should show "0"
  cat legup-4.0.tar.gz | gunzip | tar tvf - | grep -v ' legup-4.0/' | wc -l
```


8. Run `build-legup.sh`
```
chmod +x build-legup.sh
./build-legup.sh
```


9. Export required PATHs in `.bashrc`

Notice here the variable `WORKDIR` should be whatever directory was
created in the earlier `git clone` step.
```
gedit ~/.bashrc    # or vim, emacs, subl, etc.

Add the following to .bashrc:

WORKDIR=${HOME}/legup   # This should be the directory created by git clone
export LEGUP_PATH= "${WORKDIR}/legup-4.0"
export AUTOPHASE_PATH= "${WORKDIR}/autophase"
export DEVICE_FAMILY="Cyclone V"
export FRONT_END="${WORKDIR}/clang/bin/clang"
```


10. Check the Clang and Autophase paths

Log out and log in again, then do the following:
```
$FRONT_END --version
# should print:
# clang version 3.5.2 (tags/RELEASE_352/final)
# Target: x86_64-unknown-linux-gnu
# Thread model: posix
ls -l $AUTOPHASE_PATH/gym-hls/setup.py
# Should show a file (currently it is 115 bytes)
```
The first should print "clang version 3.5.2". The other command should
work (if you get "No such file or directory" it didn't work).

If these do not work, you may need to find older versions of the libraries
**libtcl8.5.so** and **libtinfo.so.5** and put them in ~/.local/lib
and then add this to your .bashrc:
```
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${HOME}/.local/lib"
```


11. Check the versions of Python and pip
```
python3.9 --version
# Python 3.9.14
pip3.9 --version
# pip 22.2.2 from ~/.local/lib/python3.9/site-packages/pip (python 3.9)
```


Note: Ray/RLib has limited support for some Python versions. For example,
when these instructions were written, the current version of Ray
(1.7.0) did not work in the version of Python (3.10) that was standard
on Fedora 35. Therefore we proceed here with specific versions that are
known to work. First is to get Python and pip 3.9:

```
sudo dnf install python3.9
python3.9 -V
# should print "Python 3.9.12"
cd
curl -sSL https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3.9 get-pip.py
pip3.9 --version
# should print "pip 22.0.4 from ~/.local/lib/python3.9/site-packages/pip (python 3.9)"
```


12. Now use pip3.9 to install the machine learning libraries and other
required packages:
```
pip3.9 install scipy==1.8.*
pip3.9 install numpy==1.19.*
pip3.9 install tensorflow==2.6.*
pip3.9 install ray[rlib]==1.7.*
pip3.9 install sklearn
pip3.9 install iPython==8.3.*
pip3.9 install keras==2.6.*
pip3.9 install gym==0.22.*
pip3.9 install pandas==1.4.2
pip3.9 install tabulate==0.8.9
pip3.9 install dm-tree==0.1.7
pip3.9 install lz4==4.0.0
```


13. Build the autophase `gym-hls` by running its setup.py
```
cd autophase/gym-hls/
pip3.9 install -e .
```


14. Test Legup by doing a 'make' in its matrixmultiply example - this
will generate the verilog file. Run 'make v' to get clock cycle
estimation using modelsim
```
cd $LEGUP_PATH/examples/matrixmultiply
make
# The following assumes the "Steps planned for the future" below
# have been done. It only works if Modelsim is installed, but is
# not needed to run Autophase experiments.
make v  
```


15. Test Autophase. A log is made in the same folder that documents
the results of each trial.
```
cd $AUTOPHASE_PATH/algos/rl
python RL-PPO2.py
# This will run a long time, stop it whenever you get bored
```


16. All other files, including the Python program used for the Munafo
paper, are published at github.com/mrob27/impalcdhls

If you got this file from there, then there should also be a README.md
which contains further instructions.

