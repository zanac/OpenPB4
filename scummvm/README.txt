To compile ScummVM for armhf target use thit patch:


--- configure   2016-10-10 23:50:16.000000000 +0200
+++ configure-patch     2017-11-20 11:17:48.060000000 +0100
@@ -2195,6 +2195,8 @@
                                ;;
                        arm-apple-darwin11)
                                ;;
+                       arm-linux-gnueabihf)
+                               ;;

                        *)
                                define_in_config_if_yes yes 'USE_ARM_SCALER_ASM'




After that use this configure:
./configure --disable-seq-midi --disable-sndio --disable-timidity --host=arm-linux-gnueabihf --prefix=/usr --enable-vkeybd --disable-libcurl --disable-sdlnet --disable-nasm
