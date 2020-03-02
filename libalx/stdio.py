################################################################################
#	Copyright (C) 2020	Alejandro Colomar Andrés		       #
#	SPDX-License-Identifier:	LGPL-2.0-only			       #
################################################################################
#
# C-like stdio.h functions
#
################################################################################


import	sys


def printf(format, *args):

	fprintf(sys.stdout, format % args);
	return	None;


def fprintf(stream, format, *args):

	stream.write(format % args);
	return	None;


def sprintf(format, *args):

	str = format % args;
	return	str;


def snprintf(size, format, *args):

	str = format % args;
	return	str[0:size];


################################################################################
#	end of file							       #
################################################################################
