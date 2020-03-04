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

########
# input

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

########
# cv

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

def img_roi_2rrect(img, rrect):
	ctr	= rrect[0];
	sz	= rrect[1];
	x	= int(ctr[0] - sz[0] / 2);
	y	= int(ctr[1] - sz[1] / 2);
	w	= int(sz[0]);
	h	= int(sz[1]);
	roi	= img_roi_set(img, x, y, w, h);
	return	roi;

def largest_cnt(cnts):
	A	= cv.contourArea(cnts[0]);
	cnt	= 0;
	for i in range(len(cnts)):
		a	= cv.contourArea(cnts[i]);
		if a > A:
			A	= a;
			cnt	= i;
	return	cnt;

def orb_match_template(img, t):
	orb	= cv.ORB_create();
	cv.imshow("img", img);
	wait_for_ESC();
	img	= cv.resize(img, t.shape[::-1]);
	cv.imshow("img", img);
	wait_for_ESC();
	kp0, d0	= orb.detectAndCompute(img, None);
	kp1, d1	= orb.detectAndCompute(img, None);
	matcher	= cv.DescriptorMatcher_create("BruteForce-Hamming");
	matches	= matcher.match(d1, d0);
	matches	= sorted(matches, key = lambda x:x.distance);
	imgmatx	= img.copy();
	imgmatx	= cv.drawMatches(img, kp1, t, kp0, matches, imgmatx);
	cv.imshow("img", imgmatx);
	wait_for_ESC();

def orb_match_templates(img, temps):
	for i in temps:
		t	= temps[i];
		orb_match_template(img, t);

def match_templates(sym, temps):
	best	= 0;
	res	= [[0]];
	print (res);
	thr	= 0.32;
	for i in temps:
		t	= temps[i];
		r	= cv.matchTemplate(sym, t, cv.TM_CCOEFF_NORMED);
		print(r);
#		if r > res:
#			res	= r;
#			best	= i;
	if res[0][0] >= thr:
		alx.printf("sym string: %s\n", best);
		return	best;
	alx.printf("sym not detected\n");
	return	"Not detected";

########
# process

def find_label(img):
	hsv	= cv.cvtColor(img, cv.COLOR_BGR2HSV);
#	cv.imshow("img", hsv);
#	wait_for_ESC();
	s	= hsv[:,:,1];
#	cv.imshow("img", s);
#	wait_for_ESC();
	blur	= cv.medianBlur(s, 71);
#	cv.imshow("img", blur);
#	wait_for_ESC();
	_, thr	= cv.threshold(blur, 30, 255, cv.THRESH_BINARY_INV);
#	cv.imshow("img", thr);
#	wait_for_ESC();
	tmp	= cv.erode(thr, None, iterations=80,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
#	cv.imshow("img", tmp);
#	wait_for_ESC();
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
	roi	= img_roi_2rrect(img, rotrect);
#	cv.imshow("img", roi);
#	wait_for_ESC();
	border	= cv.copyMakeBorder(roi, top=50, bottom=50, left=50, right=50,
			borderType=cv.BORDER_CONSTANT, value=(255, 255, 255));
	cv.imshow("img", border);
	wait_for_ESC();
	return	border;

def find_symbols_loc(img):
	gray	= cv.cvtColor(img, cv.COLOR_BGR2GRAY);
#	cv.imshow("img", gray);
#	wait_for_ESC();
	blur	= cv.medianBlur(gray, 3);
#	cv.imshow("img", blur);
#	wait_for_ESC();
	_, thr	= cv.threshold(blur, 50, 255, cv.THRESH_BINARY);
#	cv.imshow("img", thr);
#	wait_for_ESC();
	inv	= cv.bitwise_not(thr);
#	cv.imshow("img", inv);
#	wait_for_ESC();
	tmp	= thr.copy();
	cv.floodFill(tmp, None, (0, 0), 0);
#	cv.imshow("img", tmp);
#	wait_for_ESC();
	filled	= cv.bitwise_or(tmp, inv);
#	cv.imshow("img", filled);
#	wait_for_ESC();
	tmp	= cv.dilate(filled, None, iterations=2,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
#	cv.imshow("img", tmp);
#	wait_for_ESC();
	tmp	= cv.erode(tmp, None, iterations=2,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
#	cv.imshow("img", tmp);
#	wait_for_ESC();
	tmp	= cv.erode(tmp, None, iterations=10,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
#	cv.imshow("img", tmp);
#	wait_for_ESC();
	syms	= cv.dilate(tmp, None, iterations=10,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	cv.imshow("img", syms);
	wait_for_ESC();
	return	syms;

def find_symbols(img):
	cnts,_ = cv.findContours(img, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);
	syms	= list();
	for cnt in cnts:
		s_	= cv.minAreaRect(cnt);
		sym	= [
			[s_[0][0], s_[0][1]],
			[s_[1][0] * 1.8, s_[1][1] * 1.8],
			s_[2]
		];
		if sym[1][0] < sym[1][1]:
			sym[1][0] = sym[1][1];
		else:
			sym[1][1] = sym[1][0];
		sym[2]	= 0;
		syms.append(sym);
	return	syms;

def crop_symbols(img, symlocs):
#	cv.imshow("img", img);
#	wait_for_ESC();
	syms	= list();
	for symloc in symlocs:
		roi	= img_roi_2rrect(img, symloc);
#		cv.imshow("img", roi);
#		wait_for_ESC();
		gray	= cv.cvtColor(roi, cv.COLOR_BGR2GRAY);
#		cv.imshow("img", gray);
#		wait_for_ESC();
		_, thr	= cv.threshold(gray, 64, 255, cv.THRESH_BINARY);
		cv.imshow("img", thr);
		wait_for_ESC();
		syms.append(thr);
	return	syms;

def clean_symbols(syms):
	syms_clean	= list();
	for sym in syms:
		inv	= cv.bitwise_not(sym);
		cv.imshow("img", inv);
		tmp	= cv.dilate(inv, None, iterations=3,
						borderType=cv.BORDER_CONSTANT,
						borderValue=0);
		cv.imshow("img", tmp);
		wait_for_ESC();
		cnts, _	= cv.findContours(tmp, cv.RETR_EXTERNAL,
							cv.CHAIN_APPROX_SIMPLE);
		cnt_i	= largest_cnt(cnts);
		mask	= np.zeros(tmp.shape, np.uint8);
		cv.imshow("img", mask);
		wait_for_ESC();
		alx.printf("cnt is %i\n", cnt_i);
		cv.drawContours(mask, cnts, cnt_i, 255, -1);
		cv.imshow("img", mask);
		wait_for_ESC();
		nsym_clean	= cv.bitwise_and(inv, mask);
		cv.imshow("img", nsym_clean);
		wait_for_ESC();
		sym_clean	= cv.bitwise_not(nsym_clean);
		syms_clean.append(sym_clean);
	return	syms_clean;



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
	label	= crop_label(align, rotrect);
	cnts	= find_symbols_loc(label);
	symlocs	= find_symbols(cnts);
	syms	= crop_symbols(label, symlocs);
	syms	= clean_symbols(syms);
	for sym in syms:
		cv.imshow("img", sym);
		wait_for_ESC();
		orb_match_templates(sym, templates);
	

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

