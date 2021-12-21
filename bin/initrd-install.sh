#!/bin/bash

set -e

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

cp -r lib /usr/
dracut -f
grub2-mkconfig -o /boot/efi/EFI/fedora/grub.cfg
grub2-mkconfig -o /boot/grub2/grub

