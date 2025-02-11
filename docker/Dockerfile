FROM burningdaylight/mingw-arch

RUN pacman-key --init
RUN pacman -Sy archlinux-keyring --noconfirm
RUN pacman -Syu --noconfirm ninja patchutils dos2unix base-devel go
RUN su devel -c "cd $HOME && git clone https://aur.archlinux.org/yay.git && cd yay && makepkg -si --noconfirm"
RUN su devel -c "yay -Syu --noconfirm"
RUN su devel -c "yay -S --noconfirm mingw-w64-libsndfile mingw-w64-qt5-activeqt qt5-base qt5-declarative"
