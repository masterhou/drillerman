#!/usr/bin/env bash
glc-capture --start --pbo --disable-audio -o out.glc ./dman
glc-play out.glc  -o - -y 1 | mencoder -demuxer y4m - -nosound -ovc x264 -x264encopts qp=6:pass=1 -of avi -o video.avi
