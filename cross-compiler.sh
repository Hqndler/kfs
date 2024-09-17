# sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmfr-dev texinfo libisl-dev

wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz
wegt https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.gz

tar xvf gcc-14.2.0.tar.gz
tar xvf binutils-2.43.1.tar.gz

export PREFIX="$PWD/opt/cross"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"

mkdir -p build-binutils
cd build-binutils
../binutils-2.43.1/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ..

which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir -p build-gcc
cd build-gcc
../gcc-14.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc