#!/bin/sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp bin/kernel isodir/boot/PlOS
cat > isodir/boot/grub/grub.cfg << EOF
set default=0 # Set the default menu entry
 
menuentry "PlOS" {
   multiboot /boot/PlOS 
   boot
}
EOF
grub-mkrescue -o build/PlOS.iso isodir
