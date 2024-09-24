FROM burningdaylight/mingw-arch

RUN pacman-key --init
RUN pacman -Sy archlinux-keyring --noconfirm
RUN pacman -Syu --noconfirm ninja patchutils dos2unix
