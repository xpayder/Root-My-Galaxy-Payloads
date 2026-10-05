#!/bin/bash
set -e

apt-get update
apt-get install -y bc bison build-essential curl flex git libssl-dev libelf-dev clang lld llvm wget unzip python3 gcc-aarch64-linux-gnu

# Fetch KernelSU
cd /workspace/kernelsu
if [ ! -d "KernelSU" ]; then
    git clone https://github.com/tiann/KernelSU.git
    cd KernelSU
    git checkout v3.2.5
    git apply ../patches/KernelSU-v3.2.5-samsung-kdp-rkp-defex.patch
    cd ..
fi

cd /workspace/samsungopensource/SM-G990B_16_Opensource/Kernel

export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export LLVM=1
export LLVM_IAS=1
export CLANG_TRIPLE=aarch64-linux-gnu-

make O=out vendor/r9q_eur_openx_defconfig

# Force the exact vermagic string instead of default
echo "5.4.289-qgki-32192773-abG990B2XXSKIZH2" > out/include/config/kernel.release
mkdir -p out/include/generated
echo '#define UTS_RELEASE "5.4.289-qgki-32192773-abG990B2XXSKIZH2"' > out/include/generated/utsrelease.h

make O=out modules_prepare

# Generate SELinux headers for KernelSU if needed
mkdir -p out/security/selinux
if [ -f "out/scripts/selinux/genheaders/genheaders" ]; then
    out/scripts/selinux/genheaders/genheaders out/security/selinux/flask.h out/security/selinux/av_permissions.h || true
fi

# Build KernelSU
make -C out M="/workspace/kernelsu/KernelSU/kernel" src="/workspace/kernelsu/KernelSU/kernel" \
  CONFIG_KSU=m CONFIG_KSU_SAMSUNG_KDP=y CONFIG_KSU_SAMSUNG_RKP=y CONFIG_KSU_SAMSUNG_DEFEX=y \
  KBUILD_MODPOST_WARN=1 modules

cp /workspace/kernelsu/KernelSU/kernel/kernelsu.ko /workspace/kernelsu/android13-5.4_kernelsu-G990B2XXSKIZH2-kdp.ko
modinfo /workspace/kernelsu/android13-5.4_kernelsu-G990B2XXSKIZH2-kdp.ko | grep vermagic
