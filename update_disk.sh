 #!/bin/sh -e
 mkdir -p ./mnt/
 sudo mount -o loop,offset=1048576 ./disk.img ./mnt
 sudo mkdir -p ./mnt/boot/
 sudo mkdir -p ./mnt/boot/grub/
 sudo cp ./bin/kernel ./mnt/kernel
 sudo cp ./isodir/boot/grub/grub.cfg ./mnt/boot/grub2/grub.cfg
 sudo umount ./mnt
 sync