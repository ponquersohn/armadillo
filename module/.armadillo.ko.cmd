cmd_/root/ls2020/homebrew_tools/armadillo/module/armadillo.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000 -T /usr/src/linux-headers-5.3.0-kali2-common/scripts/module-common.lds --build-id  -o /root/ls2020/homebrew_tools/armadillo/module/armadillo.ko /root/ls2020/homebrew_tools/armadillo/module/armadillo.o /root/ls2020/homebrew_tools/armadillo/module/armadillo.mod.o ;  true