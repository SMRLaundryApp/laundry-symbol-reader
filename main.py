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
#	DBG								       #
################################################################################
# 0: Only solution;  >=1 debugging info

DBG	= 1;

def dbg_printf(dbg, fmt, *args):
	if (dbg > DBG):
		return	None;
	alx.printf(fmt % args);
	return	None;

def dbg_show(dbg, win, img):
	if (dbg > DBG):
		return	None;
	cv.imshow(win, img);
	wait_for_ESC();
	return	None;

def dbg_show_templates(dbg, win, templates):
	for i in templates:
		dbg_show(dbg, win, templates[i]);


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
	"dry_clean",
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
	dbg_printf(2, "Loaded template: %s\n", fname);
	return	t;

def get_templates():
	temp = dict();

	for t in templates_names:
		t_not		= t + "_not";
		temp[t]		= get_template(t);
		temp[t_not]	= get_template(t_not);

	return	temp;

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
	dbg_printf(2, "ctr= (%i, %i); sz= (%i, %i); angle= %lf\n", int(ctr[0]),
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

def fill_img(img):
	inv	= cv.bitwise_not(img);
	tmp	= img.copy();
	cv.floodFill(tmp, None, (0, 0), 0);
	filled	= cv.bitwise_or(tmp, inv);
	return	filled;

########
# helpers

def crop_clean_symbol(sym):
	border	= cv.copyMakeBorder(sym, top=60, bottom=60, left=60, right=60,
			borderType=cv.BORDER_CONSTANT, value=0);
	dbg_show(3, "img", border);
	cnts,_ = cv.findContours(border, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);
	i	= largest_cnt(cnts);
	loc	= cv.minAreaRect(cnts[i]);
	symloc	= [
		[loc[0][0], loc[0][1]],
		[loc[1][0] * 1.3, loc[1][1] * 1.3],
		loc[2]
	];
	if symloc[1][0] < symloc[1][1]:
		symloc[1][0] = symloc[1][1];
	else:
		symloc[1][1] = symloc[1][0];
	symloc[2] = 0;
	roi	= img_roi_2rrect(border, symloc);
	dbg_show(2, "img", roi);
	return	roi;

def crop_base_symbol(sym):
	cnts, _	= cv.findContours(sym, cv.RETR_EXTERNAL,
						cv.CHAIN_APPROX_SIMPLE);
	cnt_i	= largest_cnt(cnts);
	mask	= np.zeros(sym.shape, np.uint8);
	dbg_show(3, "img", mask);
	cv.drawContours(mask, cnts, cnt_i, 255, -1);
	dbg_show(3, "img", mask);
	sym_base	= cv.bitwise_and(sym, mask);
	dbg_show(2, "img", sym_base);
	return	sym_base;

def cmp_template(t, img):
	img_n	= cv.resize(img, t.shape[::-1]);
	diff	= cv.bitwise_xor(t, img_n);
	dbg_show(3, "img", diff);
	and_	= cv.bitwise_and(t, img_n);
	dbg_show(3, "img", and_);
	nand	= cv.bitwise_not(and_);
	dbg_show(3, "img", nand);
	nande	= cv.erode(nand, None, iterations=2,
						borderType=cv.BORDER_CONSTANT,
						borderValue=0);
	dbg_show(3, "img", nande);
	diffd	= cv.dilate(diff, None, iterations=2,
						borderType=cv.BORDER_CONSTANT,
						borderValue=0);
	dbg_show(3, "img", diffd);
	diffmsk	= cv.bitwise_and(diffd, nande);
	dbg_show(2, "img", diffmsk);
	pix_and	= cv.countNonZero(and_);
	pix_dif	= cv.countNonZero(diffmsk);
	dbg_printf(3, "and: %i, dif: %i\t", pix_and, pix_dif);
	match	= (pix_and - pix_dif) / pix_and;
	if (match < 0):
		match	= 0;
	dbg_printf(2, "match: %.3lf\n", match);
	return	match;

def sym_inside(sym):
	dbg_show(3, "img", sym);
	filld	= sym.copy();
	cv.floodFill(filld, None, (0, 0), 0);
	dbg_show(3, "img", filld);
	cnts, _	= cv.findContours(filld, cv.RETR_EXTERNAL,
						cv.CHAIN_APPROX_SIMPLE);
	cnt_i	= largest_cnt(cnts);
	mask	= np.zeros(sym.shape, np.uint8);
	dbg_show(3, "img", mask);
	cv.drawContours(mask, cnts, cnt_i, 255, -1);
	dbg_show(3, "img", mask);
	invsym	= cv.bitwise_not(sym);
	dbg_show(3, "img", invsym);
	inv	= cv.bitwise_and(invsym, mask);
	dbg_show(3, "img", inv);
	inside	= cv.bitwise_not(inv);
	dbg_show(2, "img", inside);
	return	inside;

def sym_outside(sym):
	dbg_show(3, "img", sym);
	filled	= fill_img(sym);
	dbg_show(3, "img", filled);
	cnts, _	= cv.findContours(filled, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);
	i	= largest_cnt(cnts);
	nmsk	= np.zeros(sym.shape, np.uint8);
	dbg_show(3, "img", nmsk);
	cv.drawContours(nmsk, cnts, i, 255, -1);
	dbg_show(3, "img", nmsk);
	msk	= cv.bitwise_not(nmsk);
	dbg_show(3, "img", msk);
	sym_out	= cv.bitwise_and(filled, msk);
	dbg_show(2, "img", sym_out);
	return	sym_out;

########
# process

def find_label(img):
	hsv	= cv.cvtColor(img, cv.COLOR_BGR2HSV);
	dbg_show(2, "img", hsv);
	s	= hsv[:,:,1];
	dbg_show(2, "img", s);
	blur	= cv.medianBlur(s, 71);
	dbg_show(2, "img", blur);
	_, thr	= cv.threshold(blur, 30, 255, cv.THRESH_BINARY_INV);
	dbg_show(2, "img", thr);
	tmp	= cv.erode(thr, None, iterations=80,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(2, "img", tmp);
	tmp	= cv.dilate(tmp, None, iterations=80,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(1, "img", tmp);
	cnts,_ = cv.findContours(tmp, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);
	cnt	= cnts[0];
	rotrect	= cv.minAreaRect(cnt);
	return	rotrect;

def align_label(img, rotrect):
	align	= img_rotate_2rect(img, rotrect);
	dbg_show(1, "img", align);
	return	align;

def crop_label(img, rotrect):
	roi	= img_roi_2rrect(img, rotrect);
	dbg_show(2, "img", roi);
	border	= cv.copyMakeBorder(roi, top=50, bottom=50, left=50, right=50,
			borderType=cv.BORDER_CONSTANT, value=(255, 255, 255));
	dbg_show(1, "img", border);
	return	border;

def find_symbols_loc(img):
	gray	= cv.cvtColor(img, cv.COLOR_BGR2GRAY);
	dbg_show(2, "img", gray);
	blur	= cv.medianBlur(gray, 3);
	dbg_show(2, "img", blur);
	_, thr	= cv.threshold(blur, 50, 255, cv.THRESH_BINARY);
	dbg_show(2, "img", thr);
	filled	= fill_img(thr);
	dbg_show(2, "img", filled);
	tmp	= cv.dilate(filled, None, iterations=2,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(2, "img", tmp);
	tmp	= cv.erode(tmp, None, iterations=2,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(2, "img", tmp);
	tmp	= cv.erode(tmp, None, iterations=10,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(2, "img", tmp);
	syms	= cv.dilate(tmp, None, iterations=10,
					borderType=cv.BORDER_CONSTANT,
					borderValue=0);
	dbg_show(1, "img", syms);
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
	dbg_show(2, "img", img);
	syms	= list();
	for symloc in symlocs:
		roi	= img_roi_2rrect(img, symloc);
		dbg_show(2, "img", roi);
		gray	= cv.cvtColor(roi, cv.COLOR_BGR2GRAY);
		dbg_show(2, "img", gray);
		_, thr	= cv.threshold(gray, 64, 255, cv.THRESH_BINARY);
		dbg_show(1, "img", thr);
		syms.append(thr);
	return	syms;

def clean_symbols(syms):
	syms_clean	= list();
	for sym in syms:
		inv	= cv.bitwise_not(sym);
		dbg_show(2, "img", inv);
		tmp	= cv.dilate(inv, None, iterations=3,
						borderType=cv.BORDER_CONSTANT,
						borderValue=0);
		dbg_show(2, "img", tmp);
		cnts, _	= cv.findContours(tmp, cv.RETR_EXTERNAL,
							cv.CHAIN_APPROX_SIMPLE);
		cnt_i	= largest_cnt(cnts);
		mask	= np.zeros(tmp.shape, np.uint8);
		dbg_show(2, "img", mask);
		cv.drawContours(mask, cnts, cnt_i, 255, -1);
		dbg_show(2, "img", mask);
		nsym_clean	= cv.bitwise_and(inv, mask);
		dbg_show(2, "img", nsym_clean);
		sym_clean	= cv.bitwise_not(nsym_clean);
		dbg_show(1, "img", sym_clean);
		syms_clean.append(sym_clean);
	return	syms_clean;

def match_templates(img, temps):
	match	= 0;
	code	= 0;
	thr	= 0.4;
	for i in temps:
		t	= temps[i];
		img_filled	= fill_img(img);
		img_norm	= crop_clean_symbol(img_filled);
		img_base_n	= crop_base_symbol(img_norm);
		t_filled	= fill_img(t);
		t_norm		= crop_clean_symbol(t_filled);
		m		= cmp_template(t_norm, img_base_n);
		if m > match:
			code	= i;
			match	= m;
	if match < thr:
		code	= "error";
	dbg_printf(2, "%s (match: %.3lf)\n", code, match);
	return	code;

def find_details(img, code):
	for c in templates_names:
		dbg_printf(2, "%s =? %s\n", c, code);
		if c == code:
			sym_i	= sym_inside(img);
			dbg_show(1, "img", sym_i);
			sym_o	= sym_outside(img);
			dbg_show(1, "img", sym_o);


################################################################################
#	main								       #
################################################################################
def main():

	dbg_printf(1, "Hello, world!\n");
	dbg_printf(1, "We have %i days to finish this\n", 2);
	if (DBG):
		cv.namedWindow("img");

	img		= img_get();
	templates	= get_templates();

	dbg_show_templates(2, "img", templates);

	dbg_show(1, "img", img);

	rotrect	= find_label(img);
	align	= align_label(img, rotrect);
	label	= crop_label(align, rotrect);
	cnts	= find_symbols_loc(label);
	symlocs	= find_symbols(cnts);
	syms	= crop_symbols(label, symlocs);
	syms	= clean_symbols(syms);
	codes	= list();
	for sym in syms:
		dbg_show(0, "img", sym);
		code	= match_templates(sym, templates);
		codes.append(code);
		dbg_printf(0, "%s\n", code);
		find_details(sym, code);

	cv.destroyAllWindows();

	dbg_printf(1, "F*** you %i and %i times python.  This is a C printf :)\n", 3000, 1);
	dbg_printf(1, "I'm just playing python; you know I love you.\n");

	return	1; # python always does fail, so return 1 ;)


################################################################################
#	run								       #
################################################################################
if __name__ == "__main__":
	main();


################################################################################
#	end of file							       #
################################################################################

