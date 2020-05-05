###############################################################################
#        Copyright (C) 2020        Sebastian Francisco Colomar Bauza          #
#        Copyright (C) 2020        Alejandro Colomar Andrés                   #
#        SPDX-License-Identifier:  GPL-2.0-only                               #
###############################################################################

FROM	debian@sha256:e6a6f2625ec46aa6ce5c537208565cde16138e7963c341ff2a3ecbf9a6736060 \
			AS build

RUN	apt-get update							&& \
	apt-get upgrade -V --yes					&& \
	apt-get install -V \
			gcc \
			gcc-10 \
			g++ \
			g++-10 \
			make \
			git \
			pkg-config \
			libbsd-dev \
			libgsl-dev \
			libopencv-dev \
			deborphan \
			--yes						&& \
	apt-get autoremove --purge --yes				&& \
	apt-get purge $(deborphan) --yes				&& \
	apt-get autoclean						&& \
	apt-get clean
WORKDIR	/tmp
RUN	git clone							\
	    --single-branch						\
	    --branch v1.0-b23						\
	    https://github.com/alejandro-colomar/libalx.git		&& \
	make	base cv				-C libalx	-j 8	&& \
	make	install-base install-cv		-C libalx	-j 8
RUN	git clone							\
	    --single-branch						\
	    --branch v1.1						\
	    https://github.com/SMRLaundryApp/laundry-symbol-reader.git  && \
	make			-C laundry-symbol-reader	-j 2

FROM	debian@sha256:e6a6f2625ec46aa6ce5c537208565cde16138e7963c341ff2a3ecbf9a6736060
RUN	apt-get update							&& \
	apt-get upgrade --yes						&& \
	apt-get install -V \
			make \
			libc6 \
			libstdc++6 \
			libbsd0 \
			libgsl23 \
			libgslcblas0 \
			libopencv-core4.2 \
			libopencv-videoio4.2 \
			libopencv-dev \
			--yes						&& \
	apt-get autoremove --purge --yes				&& \
	apt-get autoclean						&& \
	apt-get clean
WORKDIR	/tmp
COPY	--from=build /tmp/libalx ./libalx
RUN	make	install-base install-cv		-C libalx	-j 8	&& \
	rm -rf	libalx
COPY	--from=build /tmp/laundry-symbol-reader ./laundry-symbol-reader
RUN	make	install		-C laundry-symbol-reader	-j 8	&& \
	rm -rf	laundry-symbol-reader
CMD	["bash"]

# docker container run --rm --tty --interactive --volume $PWD:$PWD laundrysymbolreader/reader laundry-symbol-reader $PWD/2.jpeg  

