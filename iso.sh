#!/bin/sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp bin/kernel isodir/boot/kernel
cat > isodir/boot/grub/grub.cfg << EOF
set default=0
set timeout=0
 
menuentry "PlOS" {
   multiboot /boot/kernel 
   boot
}
EOF
grub-mkrescue -o build/PlOS.iso isodir
