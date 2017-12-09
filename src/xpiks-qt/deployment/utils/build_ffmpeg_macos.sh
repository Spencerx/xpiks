#!/bin/bash

# To be run in the directory of 'ffmpeg' sources

COMMON_OPTIONS="--enable-version3 --disable-static --enable-shared --arch=x86_64 --logfile=logfile.txt"

COMPLETELY_DISABLED_OPTIONS="--disable-swscale-alpha --disable-programs --disable-ffprobe --disable-ffserver --disable-ffplay --disable-ffmpeg --disable-doc --disable-network --disable-encoders --disable-muxers --disable-bsfs"

# list of all possible VIDEO decoders was retrieved by
# ffmpeg -codecs | grep -e "^[ ]..V.*" | awk '{split($0,a); print(a[2])}' | tr '\n' ','

SUPPORTED_VIDEO_DECODERS="012v,4xm,8bps,a64_multi,a64_multi5,aasc,aic,alias_pix,amv,anm,ansi,apng,asv1,asv2,aura,aura2,avrn,avrp,avs,avui,ayuv,bethsoftvid,bfi,binkvideo,bintext,bmp,bmv_video,brender_pix,c93,cavs,cdgraphics,cdxl,cfhd,cinepak,cljr,cllc,cmv,cpia,cscd,cyuv,daala,dds,dfa,dirac,dnxhd,dpx,dsicinvideo,dvvideo,dxa,dxtory,dxv,escape124,escape130,exr,ffv1,ffvhuff,fic,flashsv,flashsv2,flic,flv1,fraps,frwu,g2m,gif,h261,h263,h263i,h263p,h264,hap,hevc,hnm4video,hq_hqa,hqx,huffyuv,idcin,idf,iff_ilbm,indeo2,indeo3,indeo4,indeo5,interplayvideo,jpeg2000,jpegls,jv,kgv1,kmvc,lagarith,ljpeg,loco,m101,mad,magicyuv,mdec,mimic,mjpeg,mjpegb,mmvideo,motionpixels,mpeg1video,mpeg2video,mpeg4,mpegvideo_xvmc,msa1,msmpeg4v1,msmpeg4v2,msmpeg4v3,msrle,mss1,mss2,msvideo1,mszh,mts2,mvc1,mvc2,mxpeg,nuv,paf_video,pam,pbm,pcx,pgm,pgmyuv,pictor,png,ppm,prores,ptx,qdraw,qpeg,qtrle,r10k,r210,rawvideo,rl2,roq,rpza,rscc,rv10,rv20,rv30,rv40,sanm,screenpresso,sgi,sgirle,sheervideo,smackvideo,smc,smvjpeg,snow,sp5x,sunrast,svq1,svq3,targa,targa_y216,tdsc,tgq,tgv,theora,thp,tiertexseqvideo,tiff,tmv,tqi,truemotion1,truemotion2,truemotion2rt,tscc,tscc2,txd,ulti,utvideo,v210,v210x,v308,v408,v410,vb,vble,vc1,vc1image,vcr1,vixl,vmdvideo,vmnc,vp3,vp5,vp6,vp6a,vp6f,vp7,vp8,vp9,webp,wmv1,wmv2,wmv3,wmv3image,wnv1,wrapped_avframe,ws_vqa,xan_wc3,xan_wc4,xbin,xbm,xface,xwd,y41p,ylc,yop,yuv4,zerocodec,zlib,zmbv"
DECODERS_OPTIONS="--disable-decoders --enable-decoder=$SUPPORTED_VIDEO_DECODERS"

PROTOCOLS_OPIONS="--disable-protocols --enable-protocol=file --enable-protocol=data"
DEVICES_OPTIONS="--disable-outdevs"

SUPPORTED_FILTERS="buffer,buffersink,yadif,scale,format,rotate,transpose"
FILTERS_OPTIONS="--disable-filters --enable-filter=$SUPPORTED_FILTERS"

# uncomment following for the RELEASE build
#COMMON_OPTIONS="$COMMON_OPTIONS --disable-debug"

echo "Cleaning previous build artifacts.."
make clean

echo "Running configure with selected parameters.."
./configure $COMMON_OPTIONS $DECODERS_OPTIONS $PROTOCOLS_OPIONS $DEVICES_OPTIONS $FILTERS_OPTIONS

echo "Building everything.."
make

echo "Copying libs"

LIBS_DIR="../../libs/release/"
cp libavcodec/libavcodec.57.dylib "$LIBS_DIR"
cp libavfilter/libavfilter.6.dylib "$LIBS_DIR"
cp libavformat/libavformat.57.dylib "$LIBS_DIR"
cp libavutil/libavutil.55.dylib "$LIBS_DIR"
cp libswscale/libswscale.4.dylib "$LIBS_DIR"

echo "Done."

