#!/bin/bash
# super_build.sh - Compile the official ZTE kernel tree (in-tree)

cd "$(dirname "$(readlink -f "$0")")"

# Tools paths (can be overridden via environment variables)
CLANG_DIR="${CLANG_DIR:-$(pwd)/clang-r536225}"
PREBUILTS_DIR="${PREBUILTS_DIR:-$(pwd)/prebuilts/kernel-build-tools/linux-x86/bin}"

if [ ! -d "$CLANG_DIR" ]; then
    echo "❌ Error: Clang compiler not found at $CLANG_DIR"
    echo "Please download the Android Clang compiler (revision r536225) and extract it to the root of this repository,"
    echo "or set the CLANG_DIR environment variable to its location (e.g. export CLANG_DIR=/path/to/clang)."
    exit 1
fi

if [ -d "$PREBUILTS_DIR" ]; then
    export PATH="$CLANG_DIR/bin:$PREBUILTS_DIR:$PATH"
else
    export PATH="$CLANG_DIR/bin:$PATH"
fi

# Architecture config
export ARCH=arm64
export SUBARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export CROSS_COMPILE_ARM32=arm-linux-gnueabi-
export CLANG_TRIPLE=aarch64-linux-gnu-
export CC=clang

# ZTE/Vendor variables
export ZTE_BOARD_NAME=qwjujube

# Kernel Source Path
KERNEL_DIR="$(pwd)/kernel_platform/common"

echo "🚀 Starting Unified Build for RedMagic 11 Pro (NX809J)"
echo "🔧 Using Clang: $CLANG_DIR"

# 1. Defconfig (Base configuration)
echo "📝 Applying nx809j_defconfig..."
make -C $KERNEL_DIR nx809j_defconfig

# Apply config overrides directly to the common/.config file
echo "⚙️ Appending custom configuration overrides..."
{
    echo 'CONFIG_LOCALVERSION_AUTO=n'
    echo 'CONFIG_CFI_CLANG=y'
    echo 'CONFIG_CFI_PERMISSIVE=y'
    echo 'CONFIG_KSU=y'
    echo '# CONFIG_ARM64_BTI_KERNEL is not set'
    echo 'CONFIG_UNWIND_TABLES=y'
    echo 'CONFIG_UNWIND_PATCH_PAC_INTO_SCS=y'
    echo '# CONFIG_DEBUG_INFO is not set'
        echo '# CONFIG_DEBUG_INFO_DWARF5 is not set'
    echo '# CONFIG_DEBUG_INFO_BTF is not set'
    echo '# CONFIG_DEBUG_INFO_BTF_MODULES is not set'
    echo 'CONFIG_SCHED_CLASS_EXT=y'
    echo 'CONFIG_EXT_GROUP_SCHED=y'
    echo 'CONFIG_LSM="landlock,lockdown,yama,loadpin,safesetid,selinux,smack,tomoyo,apparmor,ipe,bpf"'
    echo 'CONFIG_MODVERSIONS=y'
    echo 'CONFIG_BASIC_MODVERSIONS=y'
    echo 'CONFIG_EXTENDED_MODVERSIONS=y'
} >> $KERNEL_DIR/.config

# Process config overrides
echo "🔄 Updating defconfig with overrides..."
make -C $KERNEL_DIR LLVM=1 LLVM_IAS=1 olddefconfig

echo "🧹 Force-disabling BTF/DWARF debug metadata..."

$KERNEL_DIR/scripts/config --file $KERNEL_DIR/.config \
  -d DEBUG_INFO \
  -d DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT \
  -d DEBUG_INFO_DWARF4 \
  -d DEBUG_INFO_DWARF5 \
  -d DEBUG_INFO_BTF \
  -d DEBUG_INFO_BTF_MODULES \
  -d PAHOLE_HAS_SPLIT_BTF \
  -d DEBUG_INFO_REDUCED

make -C $KERNEL_DIR ARCH=arm64 LLVM=1 LLVM_IAS=1 olddefconfig


if [ ! -f "$KERNEL_DIR/.config" ]; then
    echo "❌ Error: failed to generate .config"
    exit 1
fi

# 2. Build kernel, modules, and DTBs
echo "🛠️ Compiling Kernel, Modules, and DTBs (in-tree)..."
make -C $KERNEL_DIR -j$(nproc) LLVM=1 LLVM_IAS=1 KBUILD_MODPOST_WARN=1 Image vmlinux modules dtbs

echo "✅ Compilation finished!"
echo "📦 Kernel Image built at: $KERNEL_DIR/arch/arm64/boot/Image"
