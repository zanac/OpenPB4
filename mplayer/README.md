compile 1.3.0 with this parameter:
configure --enable-fbdev --disable-x11 --enable-cross-compile --cc=arm-linux-gnueabihf-cc --as=arm-linux-gnueabihf-asarm-linux-gnueabihf-as --target=arm-linux-gnueabihf  --disable-gui  --disable-mencoder --disable-ftp  --disable-v4l2 --disable-vcd --enable-static --disable-ossaudio


you can run mplayer for embedding with:
./mplayer-static -vo fbdev:/dev/fb0 ./spaceace.avi -geometry 100:100 -zoom -x 100 -y 50 -loop 0
