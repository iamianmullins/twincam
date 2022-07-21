#!/bin/bash

# Hackish only for Eric's personal usage

set -e

if command -v rpm > /dev/null && rpm -qa | grep -q plymouth; then
  echo "Please uninstall plymouth"
    if command -v dnf > /dev/null; then
      echo "Please run 'dnf remove plymouth'"
    fi

  exit 0
fi

for i in $(find lib -type f | xargs dirname); do
  mkdir -p /usr/$i
done

cp lib/dracut/modules.d/81twincam/module-setup-minimal.sh /usr/lib/dracut/modules.d/81twincam/module-setup.sh
cp lib/systemd/system/*.service /usr/lib/systemd/system/
ln -fs ../twincam.service /usr/lib/systemd/system/sysinit.target.wants/
ln -fs ../twincam-quit.service /usr/lib/systemd/system/multi-user.target.wants/

# Some distros don't have initrds
if command -v dracut > /dev/null; then
  dracut -f
fi

