#!/usr/bin/python3
################################################################################
################################################################################
#	Copyright (C) 2020						       #
#									       #
#	Authors:							       #
#		Alejandro Colomar Andrés				       #
#		Joao							       #
#		Jordy							       #
#		Tim							       #
#									       #
#	SPDX-License-Identifier:	GPL-2.0-only			       #
################################################################################
################################################################################


################################################################################
#	imports								       #
################################################################################
import time
from enum	import Enum

import cv2	as cv
import numpy	as np

import libalx.printf	as alx


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
img_src_fname	= "samples/2.jpeg";

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

def show_templates(win, templates):
	for i in templates:
		cv.imshow(win, templates[i]);
		wait_for_ESC();

def img_rotate(img, x, y, angle):
	map_mat	= cv.getRotationMatrix2D((y, x), angle, 1);
	h, w	= img.shape[:2];
	img	= cv.warpAffine(img, map_mat, (w, h),
				flags=cv.INTER_LINEAR,
				borderMode=cv.BORDER_CONSTANT, borderValue=0);
	return	img;

def img_rotate_2rect(img, rotrect):
	ctr	= rotrect[0];
	sz	= rotrect[1];
	angle	= rotrect[2];
	if angle < -45.0:
		angle += 90.0;
	alx.printf("ctr= (%i, %i); sz= (%i, %i); angle= %lf\n", int(ctr[0]),
			int(ctr[1]), int(sz[0]), int(sz[1]), angle);
	ctr	= tuple(map(int, ctr));
	sz	= tuple(map(int, sz));
	img	= img_rotate(img, ctr[0], ctr[1], angle);
	return	img;

def img_roi_set(img, x, y, w, h):
	roi	= img[y:y+h, x:x+w].copy();
	return	roi;

def find_label(img):
#	gray	= cv.cvtColor(img, cv.COLOR_BGR2GRAY);
	hsv	= cv.cvtColor(img, cv.COLOR_BGR2HSV);
	cv.imshow("img", hsv);
	wait_for_ESC();
	s	= hsv[:,:,1];
	cv.imshow("img", s);
	wait_for_ESC();
	blur	= cv.medianBlur(s, 71);
	cv.imshow("img", blur);
	wait_for_ESC();
	_, thr	= cv.threshold(blur,30,255, cv.THRESH_BINARY_INV);
	cv.imshow("img", thr);
	wait_for_ESC();
	tmp	= cv.erode(thr, None, iterations=80,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	cv.imshow("img", tmp);
	wait_for_ESC();
	tmp	= cv.dilate(tmp, None, iterations=80,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	cv.imshow("img", tmp);
	wait_for_ESC();
	cnts,_ = cv.findContours(tmp, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);
	cnt	= cnts[0];
	rotrect	= cv.minAreaRect(cnt);
	return	rotrect;

def align_label(img, rotrect):
	align	= img_rotate_2rect(img, rotrect);
	cv.imshow("img", align);
	wait_for_ESC();
	return	align;

def crop_label(img, rotrect):
	ctr	= rotrect[0];
	sz	= rotrect[1];
	x	= int(ctr[0] - sz[0] / 2);
	y	= int(ctr[1] - sz[1] / 2);
	w	= int(sz[0]);
	h	= int(sz[1]);
	roi	= img_roi_set(img, x, y, w, h);
	cv.imshow("img", roi);
	wait_for_ESC();
	return	roi;

def match_template(img, t):
	r	= cv.matchTemplate(img, t, cv.TM_CCOEFF_NORMED);



################################################################################
#	main								       #
################################################################################
def main():

	alx.printf("Hello, world!\n");
	alx.printf("We have %i days to finish this\n", 4);
	cv.namedWindow("img");

	img		= img_get();
	templates	= get_templates();

#	show_templates("img", templates);

	cv.imshow("img", img);
	wait_for_ESC();

	rotrect	= find_label(img);
	align	= align_label(img, rotrect);
	roi	= crop_label(align, rotrect);

	cv.destroyAllWindows();

	alx.printf("F*** you %i and %i times python.  This is a C printf :)\n", 3000, 1);
	alx.printf("I'm just playing python; you know I love you.\n");

	return	1; # python always does fail, so return 1 ;)


################################################################################
#	run								       #
################################################################################
if __name__ == "__main__":
	main();


################################################################################
#	end of file							       #
################################################################################

