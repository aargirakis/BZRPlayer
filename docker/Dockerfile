FROM burningdaylight/mingw-arch

RUN \
if ! printf '%s\n' "3.28.0-1" "$(pacman -Q cmake | awk '{print $2}')" | sort -C -V; then \
  pacman-key --init && pacman -Sy archlinux-keyring --noconfirm && pacman -Syu --noconfirm ninja; \
else \
  pacman -Sy --noconfirm ninja; \
fi