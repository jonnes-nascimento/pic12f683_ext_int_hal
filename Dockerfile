# Usage:
# 
#		docker build -t jonnes/mplabx:latest .
#		Linux   -> docker run --rm -it -v "$(PWD)":/home/developer/projects/PIC/pic12f683_ext_int_hal jonnes/mplabx
#       Windows -> docker run --rm -it -v "%cd%":/home/developer/projects/PIC/pic12f683_ext_int_hal jonnes/mplab


FROM ubuntu:jammy

LABEL maintainer = Jonnes Nascimento <jonnes.nascimento@gmail.com>

# Install i386 packages for Microchip Tools dependencies
RUN dpkg --add-architecture i386 \
    && apt-get update -yq \
    && apt-get upgrade -yq \
    && apt-get install -yq --no-install-recommends build-essential bzip2 cpio curl python2-minimal:i386 ca-certificates unzip wget \
    libc6:i386 libx11-6:i386 libxext6:i386 libstdc++6:i386 libexpat1:i386 \
    libxext6 libxrender1 libxtst6 libgtk2.0-0 libxslt1.1 libncurses5-dev bzip2 \
	unzip \
	xz-utils

# Install XC8 compiler
RUN curl -fSL -A "Mozilla/4.0" -o /tmp/xc8.run "http://www.microchip.com/mplabxc8linux" \
    && chmod a+x /tmp/xc8.run \
    && /tmp/xc8.run --mode unattended --unattendedmodeui none \
        --netservername localhost --LicenseType FreeMode --prefix /opt/microchip/xc8 \
    && rm /tmp/xc8.run

ENV PATH $PATH:/opt/microchip/xc8/bin

# Install MPLABX IDE, current version
RUN curl -fSL -A "Mozilla/4.0" -o /tmp/mplabx-installer.tar "http://www.microchip.com/mplabx-ide-linux-installer" \
    && tar xf /tmp/mplabx-installer.tar && rm /tmp/mplabx-installer.tar \
    && USER=root ./*-installer.sh --nox11 \
        -- --unattendedmodeui none --mode unattended --installdir /opt/microchip/mplabx \
    && rm ./*-installer.sh

ENV PATH $PATH:/opt/microchip/mplabx/mplab_ide/bin

VOLUME /tmp/.X11-unix

# Container developer user ident
RUN useradd developer \
    && mkdir -p /home/developer/MPLABXProjects \
    && touch /home/developer/MPLABXProjects/.directory \
    && chown developer:developer /home/developer/MPLABXProjects

VOLUME /home/user/MPLABXProjects

RUN echo 'deb http://archive.ubuntu.com/ubuntu/ focal-backports main' > /etc/apt/sources.list.d/focal-backports.list

# Default to UTF-8 file.encoding
ENV LANG C.UTF-8

# add a simple script that can auto-detect the appropriate JAVA_HOME value
# based on whether the JDK or only the JRE is installed
RUN { \
		echo '#!/bin/sh'; \
		echo 'set -e'; \
		echo; \
		echo 'dirname "$(dirname "$(readlink -f "$(which javac || which java)")")"'; \
	} > /usr/local/bin/docker-java-home \
	&& chmod +x /usr/local/bin/docker-java-home

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/jre

RUN set -x \
	&& apt-get update \
	&& apt-get install -y \
		openjdk-8-jre-headless \
	&& rm -rf /var/lib/apt/lists/* \
	&& [ "$JAVA_HOME" = "$(docker-java-home)" ]

# Container Tool Version Reports to Build Log
RUN [ -x /opt/microchip/xc8/bin/xc8 ] && xc8 --ver

#CMD ["/opt/microchip/mplabx/mplab_ide/bin/mplab_ide"]