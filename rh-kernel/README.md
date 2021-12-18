# RowHammer on Smartphone: kernel implementation

On LG Nexus 5.

## Prepare

### Build kernel

```[bash]
git clone https://android.googlesource.com/kernel/msm -b android-msm-hammerhead-3.4-marshmallow-mr3
export ARCH=arm
export CROSS_COMPILE=.../aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
make hammerhead_defconfig

```

1. Download kernel source.
1. Make config file.
1. Enable kernel module loading/unloading in .config file.
    - CONFIG_MODULES=y and CONFIG_MODULE_UNLOAD=y
    - (optional) CONFIG_CPU_ICACHE_DISABLE=y and CONFIG_CPU_DCACHE_DISABLE=y

1. Fix environment issues.
    - YYLTYPE yylloc; in /scripts/dtc/dtc-lexer.l
    - YYLTYPE yylloc; in /scripts/dtc/dtc-parser.tab.c_shipped
    - Replace them with extern YYLTYPE yylloc;
    - In kernel/timeconst.h, replace if (!defined(@val)) with if (!@val)
    - See [ref](https://patchwork.kernel.org/project/linux-kbuild/patch/1353269117-39917-1-git-send-email-pefoley2@verizon.net/)

1. Kernel: arch/arm/boot/zImage-dtb is ready

### Build Android

```[bash]
repo init -u https://android.googlesource.com/platform/manifest -b android-6.0.1_r77
repo sync -c --no-tags --no-clone-bundle -j2
source build/envsetup.sh
lunch aosp_hammerhead-userdebug
export TARGET_PREBUILT_KERNEL=.../msm/arch/arm/boot/zImage-dtb
```

1. Download AOSP image

1. Solve environments
    - prebuilts/sdk/tools/jack:128
    - prebuilts/sdk/tools/jack-admin:123
    - `HTTP_CODE=$(curl --fail --silent --output $JACK_EXIT --write-out %{http_code} --connect-timeout 5 --noproxy 127.0.0.1:$SERVER_PORT_ADMIN http://127.0.0.1:$SERVER_PORT_ADMIN/$CMD)`

1. Gain root permission.
    - In build/core/main.mk, change ro.secure=0
    - See [ref](https://stackoverflow.com/questions/43679158/how-can-i-compile-an-android-kernel-with-root-permission)

1. Install proprietary binaries (See [ref](https://stackoverflow.com/questions/33083512/aosp-6-0-build-for-hammerhead-proprietary-binaries/39927763#39927763))

1. Follow installation steps.

## Run

```[bash]
export ARCH=arm
export CROSS_COMPILE=.../aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
make all; adb root;
make install; make run;
```

1. Find ION config in .../msm/arch/arm/boot/dts/msm8974-ion.dtsi
