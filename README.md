# twincam

[![GitHub Build Status](https://github.com/inotify-tools/inotify-tools/workflows/build/badge.svg)](https://github.com/ericcurtin/twincam/actions)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=ericcurtin_twincam&metric=bugs)](https://sonarcloud.io/summary/new_code?id=ericcurtin_twincam)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=ericcurtin_twincam&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=ericcurtin_twincam)

A lightweight camera application, designed to start quickly in a bare environment

# To build, install and run twincam

On Fedora (steps should be similar on other platforms):

```
sudo dnf install -y git gcc g++ libevent libevent-devel openssl openssl-devel \
  gnutls gnutls-devel meson boost boost-devel python3-pip libdrm libdrm-devel \
  systemd-udev doxygen cmake graphviz libatomic texlive-latex cppcheck \
  libyaml-devel clang zip valgrind libasan findutils SDL2_image-devel
```

On CentOS Stream 9:

```
sudo dnf install -y 'dnf-command(config-manager)'
sudo dnf config-manager --set-enabled crb
sudo dnf install -y epel-release
sudo dnf install -y SDL2_image-devel
sudo dnf install -y git gcc g++ libevent libevent-devel openssl openssl-devel \
  gnutls gnutls-devel meson boost boost-devel python3-pip libdrm libdrm-devel \
  systemd-udev doxygen cmake graphviz libatomic texlive-latex cppcheck \
  libyaml-devel clang zip valgrind libasan findutils
```

On Clear Linux:

```
sudo swupd bundle-add vim git dev-utils devpkg-gnutls devpkg-boost \
  devpkg-libevent devpkg-libdrm ccache
```

On Ubuntu:

```
sudo apt install -y clang gnutls-dev libboost-dev meson cmake pkg-config \
  libevent-dev libdrm-dev gcc make autoconf automake cppcheck libsdl2-dev \
  meson vim libgtest-dev libyaml-dev curl zip valgrind libasan5 \
  libsdl2-image-dev python3-pip
```

On Alpine (enable community repo in /etc/apk/repositories):

```
sudo apk add git vim gcc meson g++ pkgconf gnutls-dev cmake boost-dev py-pip \
  ccache libevent-dev libdrm-dev
```

On all platforms:

```
sudo pip install jinja2 ply pyyaml
git clone https://git.libcamera.org/libcamera/libcamera.git
cd libcamera
meson build --prefix=/usr && ninja -v -C build && sudo ninja -v -C build install
cd -
git clone https://github.com/ericcurtin/twincam
cd twincam
meson build --prefix=/usr && ninja -v -C build && sudo ninja -v -C build install
```

Then switch to a tty without a Desktop Environment (Gnome, XFCE, etc.) running via
Ctrl+Alt+F? (whichever tty does not have DE running F1, F2, F3, etc.) and run:

```
twincam
```

