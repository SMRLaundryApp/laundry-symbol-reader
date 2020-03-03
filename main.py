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
from enum	import Enum

import cv2		as cv
import numpy		as np

import libalx.stdio	as alx


################################################################################
#	enum								       #
################################################################################
class Img_Source(Enum):
	CAM	= 1;
	FILE	= 2;


################################################################################
#	global variables						       #
################################################################################
img_source	= Img_Source.FILE;
img_src_fname	= "foo.jpeg";

templates_ext	= ".png";
templates_dir	= "templates/";
templates_names	= [
	"bleach",
	"iron",
	"machine_wash",
	"tumble_dry"
];


################################################################################
#	functions							       #
################################################################################
def cam_get():
	cam	= cv.VideoCapture(0);
	if not cam.isOpened():
		raise Exception("Couldn't open cam.\n");

	_, img	= cam.read();
	cam.release();

	return	img;

def img_get():
	if img_source == Img_Source.CAM:
		img	= cam_get();
	elif img_source == Img_Source.FILE:
		img	= cv.imread(img_src_fname);

	return	img;

def wait_for_ESC():
	ESC	= 27;
	while (True):
		k	= cv.waitKey(5) & 0xFF;
		if k == ESC:
			break;

def get_template(name):
	fname	= templates_dir + name + templates_ext;
	tmp	= cv.imread(fname, cv.IMREAD_GRAYSCALE);
	_, t	= cv.threshold(tmp, 127, 255, cv.THRESH_BINARY);
	alx.printf("Loaded template: %s\n", fname);
	return	t;

def get_templates():
	temp = dict();

	for t in templates_names:
		t_not		= t + "_not";
		temp[t]		= get_template(t);
		temp[t_not]	= get_template(t_not);

	return	temp;


################################################################################
#	main								       #
################################################################################
def main():

	alx.printf("Hello, world!\n");
	alx.printf("We have %i days to finish this\n", 4);
	cv.namedWindow("img");

	img		= img_get();
	templates	= get_templates();

	for i in templates:
		cv.imshow("img", templates[i]);
		wait_for_ESC();
	time.sleep(1);

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
