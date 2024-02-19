#!/usr/bin/env bash

BZIMAGE_PATH=
BIOS_PATH=

KERNEL="-kernel $BZIMAGE_PATH"
APPEND="console=ttyS0 nokaslr rcu_nocbs=0 nmi_watchdog=0 ignore_loglevel modules=sd-mod,usb-storage,ext4 rootfstype=ext4 earlyprintk=serial"
APPEND+=" biosdevname=0 kvm-intel.emulate_invalid_guest_state=1 kvm-intel.enable_apicv=1 kvm-intel.enable_shadow_vmcs=1 kvm-intel.ept=1 kvm-intel.eptad=1 kvm-intel.fasteoi=1 kvm-intel.flexpriority=1 kvm-intel.nested=1 kvm-intel.pml=1 kvm-intel.unrestricted_guest=1 kvm-intel.vmm_exclusive=1 kvm-intel.vpid=1 net.ifnames=0"
QEMU_MEMORY="${QEMU_MEMORY:=16384}"
QEMU_SYSTEM_x86_64="${QEMU_SYSTEM_x86_64:=qemu-system-x86_64}"
QEMU_CPU="${QEMU_CPU:=qemu64,+smep,+smap,+rdtscp}"
QEMU_9P_SHARED_FOLDER="$PWD/out"


RAMDISK="${INITRAMFS}"
HDA=

if [ -n "${ENABLE_SGX}" ]; then
  QEMU_CPU="host,+sgx-provisionkey"
  SGX_EPC="-object memory-backend-epc,id=mem1,size=64M,prealloc=on \
           -M sgx-epc.0.memdev=mem1,sgx-epc.0.node=0"
  QEMU_SYSTEM_x86_64="sudo $QEMU_SYSTEM_x86_64"
fi

set -x


$QEMU_SYSTEM_x86_64 \
  -L $BIOS_PATH \
  ${KERNEL} \
  ${APPEND:+ -append "${APPEND}"} \
  ${RAMDISK:+ -initrd "${RAMDISK}"} \
  ${HDA:+ -drive "${HDA}"} \
  ${ATTACH_GDB:+ -gdb tcp::${GDB_PORT}} \
  ${ATTACH_GDB:+ -S} \
  ${NET1:+ -net ${NET1}} \
  ${NET2:+ -net ${NET2}} \
  ${ENABLE_KVM:+ -enable-kvm} \
  ${VM_SHARED_FOLDER:+ -hdc fat:rw:"${VM_SHARED_FOLDER}"} \
  ${QEMU_9P_SHARED_FOLDER:+ -fsdev local,id=test_dev,path=${QEMU_9P_SHARED_FOLDER},security_model=none} \
  ${QEMU_9P_SHARED_FOLDER:+ -device virtio-9p-pci,fsdev=test_dev,mount_tag=test_mount} \
  -display none \
  -smp cores=4,cores=2,threads=2,sockets=1 \
  -cpu ${QEMU_CPU} \
  ${SGX_EPC} \
  -m ${QEMU_MEMORY} \
  -echr 17 \
  -serial mon:stdio \
  -no-reboot
