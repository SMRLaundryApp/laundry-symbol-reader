#!/usr/bin/python3
################################################################################
################################################################################
#	Copyright (C) 2020	All rights reserved			       #
#	Authors:							       #
#		Alejandro Colomar Andrés				       #
#		Joao							       #
#		Jordy							       #
#		Tim							       #
################################################################################
################################################################################


################################################################################
#	imports								       #
################################################################################
import time

import cv2		as cv
import numpy		as np

import libalx.stdio	as alx


################################################################################
#	global variables						       #
################################################################################
img_extension	= ".png"

templates_names	= [
	"machine_wash_normal",
	"bleach",
	"iron",
	"tumble_dry",
	"dry_clean"
];


################################################################################
#	functions							       #
################################################################################
def img_get():
	cam	= cv.VideoCapture(0);
	if not cam.isOpened():
		raise Exception("Couldn't open cam.\n");

	_, img	= cam.read();
	cam.release();

	return	img;

def wait_for_ESC():
	ESC	= 27;
	while (True):
		k	= cv.waitKey(5) & 0xFF;
		if k == ESC:
			break;

def get_template_imgs():
	templates = dict();
	for name in templates_names:
		templates[name]	= cv.imread(name + img_extension, 0);

	return	templates;


################################################################################
#	main								       #
################################################################################
def main():

	alx.printf("Hello, world!\n");
	alx.printf("We have %i days to finish this\n", 4);

	img		= img_get();
	templates	= get_template_imgs();

	cv.namedWindow("img");
	cv.imshow("img", img);

	wait_for_ESC();

	cv.destroyAllWindows();


################################################################################
#	run								       #
################################################################################
if __name__ == "__main__":
	main();


################################################################################
#	end of file							       #
################################################################################
