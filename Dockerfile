FROM	debian:testing	AS build
RUN	apt-get update							&& \
	apt-get upgrade -V --yes					&& \
	apt-get install -V \
			gcc \
			g++ \
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
RUN	git clone https://github.com/alejandro-colomar/libalx.git	&& \
	make	base cv				-C libalx	-j 8	&& \
	make	install-base install-cv		-C libalx	-j 8
RUN	git clone https://github.com/SMRLaundryApp/laundry-symbol-reader.git  && \
	make			-C laundry-symbol-reader	-j 2

FROM	debian:testing
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
CMD	["laundry-symbol-reader"]

# docker container run --user $(id -u):$(id -g) --tty --interactive --rm --volume $PWD:$PWD --workdir $PWD --env IMG_FNAME=2.jpeg laundrysymbolreader/reader  

