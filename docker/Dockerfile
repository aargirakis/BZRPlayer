FROM burningdaylight/mingw-arch

RUN sed -i '/^\[community\]/,+1d' /etc/pacman.conf
RUN pacman-key --init
RUN pacman -Sy archlinux-keyring --noconfirm
RUN pacman -Syu --noconfirm base-devel go
RUN su devel -c "cd $HOME && git clone https://aur.archlinux.org/yay.git && cd yay && makepkg -si --noconfirm"
RUN su devel -c "yay -Syu --noconfirm dos2unix mingw-w64-libb2 mingw-w64-qt6-activeqt mingw-w64-qt6-base \
    mingw-w64-qt6-svg mingw-w64-vulkan-headers ninja patchutils qt6-declarative"