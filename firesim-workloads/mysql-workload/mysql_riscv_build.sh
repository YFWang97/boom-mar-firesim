#!/bin/bash 

set -e

unset CONDA_PREFIX
unset CONDA_DEFAULT_ENV
unset CONDA_PROMPT_MODIFIER
unset CONDA_SHLVL
export PATH="/usr/bin:/bin:/usr/sbin:/sbin"



##### Install RISCV sysroot using debootstrap #####
sudo apt install -y debootstrap
sudo apt install -y qemu-user-static

sysroot=/opt/riscv64-sysroot-mysql-workload

if [ ! -d "$sysroot" ]; then
    sudo mkdir -p "$sysroot"
    sudo qemu-debootstrap --variant=minbase --arch=riscv64 focal "$sysroot" http://ports.ubuntu.com/
    sudo chroot "$sysroot" qemu-riscv64-static \
        /bin/bash -c "apt update && apt install -y build-essential libssl-dev libncurses5-dev libtirpc-dev libbsd-dev pkg-config"

    sudo rm "$sysroot/usr/lib/riscv64-linux-gnu/libresolv.so"
    
    sudo ln -s libresolv.so.2 "$sysroot/usr/lib/riscv64-linux-gnu/libresolv.so"

    echo "$sysroot setup completed"

else
    echo "$sysroot already exists"
fi

##### Download MySQL 8.0.24 #####
if [ ! -d ./mysql-8.0.24 ]; then
    echo "Downloading mysql-8.0.24"
    wget https://dev.mysql.com/get/Downloads/MySQL-8.0/mysql-boost-8.0.24.tar.gz
    tar zxf mysql-boost-8.0.24.tar.gz
    rm -rf mysql-boost-8.0.24.tar.gz
fi

##### Build MySQL ######
cd mysql-8.0.24/
rm -rf riscv_build
rm -rf host_build
mkdir riscv_build
mkdir host_build

# Copy the cmake file
cp ../riscv.cmake cmake/

# Boost patch
cp ../boost.cmake cmake/

# Disable X Plugin
sed -i 's/\(OPTION(WITH_MYSQLX.*\)ON)/\1OFF)/' ./plugin/x/CMakeLists.txt

# Patch the temptable always_lock_free
sed -i '/static_assert/{N; /char type/s/static_assert(false,/static_assert(true,/ }' ./storage/temptable/include/temptable/lock_free_type.h
sed -i '/static_assert/{N; /short type/s/static_assert(false,/static_assert(true,/ }' ./storage/temptable/include/temptable/lock_free_type.h
sed -i '/static_assert/{N; /bool type/s/static_assert(false,/static_assert(true,/ }' ./storage/temptable/include/temptable/lock_free_type.h

# Patch priority setting
sed -i 's/setpriority/\/\/setpriority/' storage/innobase/buf/buf0buf.cc

cd host_build

cmake .. -G "Unix Makefiles" -DWITH_SERVER=OFF -DWITH_UNIT_TESTS=OFF -DWITH_DOCS=OFF -DWITH_BOOST=../boost/boost_1_73_0/boost/
cmake --build . --target comp_err -j8
export PATH="${PWD}/runtime_output_directory/:$PATH"

cd ../riscv_build
cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv.cmake -DWITH_BOOST=../boost/boost_1_73_0/boost/ -DSTACK_DIRECTION=-1
make -j8

echo "MySQL RISCV compile completed"

##### Make overlay #####
cd ../../
mkdir -p overlay/
cp mysql-8.0.24/riscv_build/bin/mysql overlay/
cp mysql-8.0.24/riscv_build/bin/mysqld overlay/
cp mysql-8.0.24/riscv_build/bin/mysqladmin overlay/
cp mysql-8.0.24/riscv_build/bin/mysqlslap overlay/

cp mysqlslap_riscv_run.sh overlay/

rm -rf mysql-8.0.24
sudo rm -rf $sysroot
