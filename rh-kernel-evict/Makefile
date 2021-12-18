# obj-m += hello.o rowhammer.c
obj-m += rh.o
ARCH=arm
CROSS_COMPILE=/home/yuan/projects/drammer/exp/aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-

# export ARCH=arm
# export CROSS_COMPILE=/home/yuan/projects/drammer/exp/aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-

all:
	make -C /home/yuan/projects/drammer/exp/msm/ M=/home/yuan/projects/rh-impl/rh-kernel-evict modules

clean:
	make -C /home/yuan/projects/drammer/exp/msm/ M=/home/yuan/projects/rh-impl/rh-kernel-evict clean

install:
	adb push rh.ko /data/local/

run:
	adb shell insmod /data/local/rh.ko
	adb shell dmesg -c
	adb shell rmmod rh