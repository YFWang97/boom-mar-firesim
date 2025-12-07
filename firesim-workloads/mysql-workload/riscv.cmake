# riscv.cmake
#
# Cross-compile toolchain file for building MySQL 8.0.44
# (mysql-boost-8.0.44, which already contains Boost)
# for riscv64-linux-gnu using a sysroot.
#
# Use with:
#   cmake .. -G "Unix Makefiles" \
#     -DCMAKE_TOOLCHAIN_FILE=../cmake/riscv.cmake
#
# Do NOT pass WITH_BOOST / DOWNLOAD_BOOST when using the mysql-boost tarball.

# ---------------- Target system ----------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# ---------------- Target triple ----------------
set(RISCV_TRIPLE "riscv64-linux-gnu")

# ---------------- Sysroot ----------------
# QEMU / target root filesystem
set(CMAKE_SYSROOT "/opt/riscv64-sysroot-mysql-workload")

# ---------------- Compilers ----------------
set(CMAKE_C_COMPILER   "${RISCV_TRIPLE}-gcc-9")
set(CMAKE_CXX_COMPILER "${RISCV_TRIPLE}-g++-9")
set(CMAKE_AR           "${RISCV_TRIPLE}-ar")
set(CMAKE_RANLIB       "${RISCV_TRIPLE}-ranlib")
set(CMAKE_NM           "${RISCV_TRIPLE}-nm")
set(CMAKE_STRIP        "${RISCV_TRIPLE}-strip")
set(CMAKE_LINKER       "${RISCV_TRIPLE}-ld")

# ---------------- Search paths ----------------
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

# Do NOT search host paths for libs/includes:
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR "/usr/bin/qemu-riscv64-static;-L;${CMAKE_SYSROOT}" CACHE STRING "Emulator for running riscv64 target binaries on host" FORCE)

set(BUILD_TEMPTABLE_STORAGE_ENGINE OFF CACHE BOOL "" FORCE)
set(WITH_TEMPTABLE_STORAGE_ENGINE OFF CACHE BOOL "" FORCE)

set(BUILD_GROUP_REPLICATION OFF CACHE BOOL "" FORCE)
set(WITH_GROUP_REPLICATION OFF CACHE BOOL "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -latomic")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -latomic")

set(CMAKE_C_STANDARD_LIBRARIES   "${CMAKE_C_STANDARD_LIBRARIES} -latomic")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -latomic")

set(CMAKE_C_LINK_FLAGS   "${CMAKE_C_LINK_FLAGS} -latomic")
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES};atomic")

set(BUILD_XPLUGIN OFF CACHE BOOL "" FORCE)
set(WITH_X OFF CACHE BOOL "" FORCE)
set(WITH_PROTOBUF OFF CACHE BOOL "" FORCE)

