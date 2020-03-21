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
import argparse

import cv2	as cv
import numpy	as np
from nose import case

import libalx.printf	as alx


################################################################################
#	DBG								       #
################################################################################
# 0: Only solution;  >=1 debugging info

DBG	= 3;

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

# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-i", "--image", help="path to the image file")
args = vars(ap.parse_args())

# file location from command
img_src_fname	= args["image"]

templates_ext	= ".png";
t_base_dir	= "templates/base/"
t_base_names	= [
	"bleach",
	"pro",
	"iron",
	"wash",
	"dry"
];
t_inner_dir	= "templates/inner/";
t_inner_names	= [
	"1_dot",
	"2_dot",
	"3_dot",
	"4_dot",
	"5_dot",
	"6_dot",
	"30",
	"40",
	"50",
	"60",
	"95",
	"A",
	"F",
	"P",
	"W",
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
	fname	= t_base_dir + name + templates_ext;
	tmp	= cv.imread(fname, cv.IMREAD_GRAYSCALE);
	t	= threshold(tmp, 127, cv.THRESH_BINARY);
	dbg_printf(2, "Loaded template: %s\n", fname);
	return	t;

def get_templates():
	temp = dict();

	for t in t_base_names:
		t_not		= t + "_not";
		temp[t]		= get_template(t);
		temp[t_not]	= get_template(t_not);

	return	temp;

def get_t_inner(name):
	fname	= t_inner_dir + name + templates_ext;
	tmp	= cv.imread(fname, cv.IMREAD_GRAYSCALE);
	t	= threshold(tmp, 127, cv.THRESH_BINARY);
	dbg_printf(2, "Loaded template: %s\n", fname);
	return	t;

def get_templates_inner():
	temp_in = dict();

	for t in t_inner_names:
		temp_in[t]	= get_t_inner(t);

	return	temp_in;

########
# cv

def threshold(img, val, thr_type):
	_, thr	= cv.threshold(img, val, 255, thr_type);
	return	thr;

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

def contours(img):
	cnts, _	= cv.findContours(img, cv.RETR_EXTERNAL,cv.CHAIN_APPROX_SIMPLE);
	return	cnts;

def add_border(img, sz, value):
	border	= cv.copyMakeBorder(img, top=sz, bottom=sz, left=sz, right=sz,
			borderType=cv.BORDER_CONSTANT, value=value);
	return	border;

def dilate(img, i):
	dil	= cv.dilate(img, None, iterations=i,
				borderType=cv.BORDER_CONSTANT, borderValue=0);
	return	dil;

def erode(img, i):
	ero	= cv.erode(img, None, iterations=i,
				borderType=cv.BORDER_CONSTANT, borderValue=0);
	return	ero;

def dilate_erode(img, i):
	dil	= dilate(img, i);
	out	= erode(dil, i);
	return	out;

def erode_dilate(img, i):
	ero	= erode(img, i);
	out	= dilate(ero, i);
	return	out;

########
# helpers

def crop_clean_symbol(sym):
	border	= add_border(sym, 60, 0);			dbg_show(3, "img", border);
	cnts	= contours(border);
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
	roi	= img_roi_2rrect(border, symloc);		dbg_show(2, "img", roi);
	return	roi;

def crop_base_symbol(sym):
	cnts	= contours(sym);
	cnt_i	= largest_cnt(cnts);
	mask	= np.zeros(sym.shape, np.uint8);		dbg_show(3, "img", mask);
	cv.drawContours(mask, cnts, cnt_i, 255, -1);		dbg_show(3, "img", mask);
	sym_base	= cv.bitwise_and(sym, mask);		dbg_show(2, "img", sym_base);
	return	sym_base;

def cmp_template(t, img, tolerance):
	if img.size < t.size:
		img_n	= cv.resize(img, t.shape[::-1]);
		t_n	= t;
	else:
		img_n	= img;
		t_n	= cv.resize(t, img.shape[::-1]);
	diff	= cv.bitwise_xor(t_n, img_n);			dbg_show(3, "img", diff);
	and_	= cv.bitwise_and(t_n, img_n);			dbg_show(3, "img", and_);
	nand	= cv.bitwise_not(and_);				dbg_show(3, "img", nand);
	nande	= erode(nand, tolerance);			dbg_show(3, "img", nande);
	diffd	= dilate(diff, tolerance);			dbg_show(3, "img", diffd);
	diffmsk	= cv.bitwise_and(diffd, nande);			dbg_show(2, "img", diffmsk);
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
	cv.floodFill(filld, None, (0, 0), 0);			dbg_show(3, "img", filld);
	cnts	= contours(filld);
	cnt_i	= largest_cnt(cnts);
	mask	= np.zeros(sym.shape, np.uint8);		dbg_show(3, "img", mask);
	cv.drawContours(mask, cnts, cnt_i, 255, -1);		dbg_show(3, "img", mask);
	invsym	= cv.bitwise_not(sym);				dbg_show(3, "img", invsym);
	inv	= cv.bitwise_and(invsym, mask);			dbg_show(3, "img", inv);
	inside	= cv.bitwise_not(inv);				dbg_show(2, "img", inside);
	return	inside;

def sym_outside(sym):
	dbg_show(3, "img", sym);
	filled	= fill_img(sym);				dbg_show(3, "img", filled);
	cnts	= contours(filled);
	i	= largest_cnt(cnts);
	nmsk	= np.zeros(sym.shape, np.uint8);		dbg_show(3, "img", nmsk);
	cv.drawContours(nmsk, cnts, i, 255, -1);		dbg_show(3, "img", nmsk);
	msk	= cv.bitwise_not(nmsk);				dbg_show(3, "img", msk);
	sym_out	= cv.bitwise_and(filled, msk);			dbg_show(2, "img", sym_out);
	return	sym_out;

def sym_out_code(sym):
	cnts	= contours(sym);
	qty	= len(cnts);
	if qty == 2:
		return	"very_delicate";
	if qty == 1:
		return	"delicate";
	return	None;

def norm_inner(sym):
	border	= add_border(sym, 20, 0);			dbg_show(3, "img", border);
	blob	= dilate_erode(border, 5);			dbg_show(3, "img", blob);
	cnts	= contours(blob);
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
	roi	= img_roi_2rrect(border, symloc);		dbg_show(2, "img", roi);

	return	roi;

def match_t_inner(sym, temps):
	match	= 0;
	code	= 0;
	thr	= 0.1;
	for i in temps:
		t	= temps[i];				dbg_show(3, "img", t);
		s_fill	= fill_img(sym);			dbg_show(3, "img", s_fill);
		s_norm	= norm_inner(s_fill);			dbg_show(3, "img", s_norm);
		t_fill	= fill_img(t);				dbg_show(3, "img", t_fill);
		t_norm	= norm_inner(t_fill);			dbg_show(3, "img", t_norm);
		m	= cmp_template(t_norm, s_norm, 0);
		if m > match:
			code	= i;
			match	= m;
	if match < thr:
		code	= "error";
	dbg_printf(2, "%s (match: %.3lf)\n", code, match);
	return	code;

def decode_inner(code_main, code_in):
	if (code_main == "bleach"):
		return	None;
	if (code_main == "pro"):
		if (code_in == "A"):
			return	"any_solvent";
		if (code_in == "F"):
			return	"petroleum_only";
		if (code_in == "W"):
			return	"wet_clean";
		if (code_in == "P"):
			return	"any_solvent_except_TCE";
	if (code_main == "iron") or (code_main == "dry"):
		if (code_in == "1_dot"):
			return	"low_temp";
		if (code_in == "2_dot"):
			return	"medium_temp";
		if (code_in == "3_dot"):
			return	"high_temp";
	if (code_main == "wash"):
		if (code_in == "30") or (code_in == "1_dot"):
			return	"30";
		if (code_in == "40") or (code_in == "2_dot"):
			return	"40";
		if (code_in == "50") or (code_in == "3_dot"):
			return	"50";
		if (code_in == "60") or (code_in == "4_dot"):
			return	"60";
		if (code_in == "70") or (code_in == "5_dot"):
			return	"70";
		if (code_in == "95") or (code_in == "6_dot"):
			return	"95";
	return	None;


########
# process

def find_label(img):
	hsv	= cv.cvtColor(img, cv.COLOR_BGR2HSV);		dbg_show(2, "img", hsv);
	s	= hsv[:,:,1];					dbg_show(2, "img", s);
	blur	= cv.medianBlur(s, 71);				dbg_show(2, "img", blur);
	thr	= threshold(blur, 30, cv.THRESH_BINARY_INV);	dbg_show(2, "img", thr);
	tmp	= erode_dilate(thr, 80);			dbg_show(1, "img", tmp);
	cnts	= contours(tmp);
	cnt	= cnts[0];
	rotrect	= cv.minAreaRect(cnt);
	return	rotrect;

def align_label(img, rotrect):
	align	= img_rotate_2rect(img, rotrect);		dbg_show(1, "img", align);
	return	align;

def crop_label(img, rotrect):
	roi	= img_roi_2rrect(img, rotrect);			dbg_show(2, "img", roi);
	border	= add_border(roi, 50, (255, 255, 255));		dbg_show(1, "img", border);
	return	border;

def find_symbols_loc(img):
	gray	= cv.cvtColor(img, cv.COLOR_BGR2GRAY);		dbg_show(2, "img", gray);
	blur	= cv.medianBlur(gray, 3);			dbg_show(2, "img", blur);
	thr	= threshold(blur, 50, cv.THRESH_BINARY);	dbg_show(2, "img", thr);
	filled	= fill_img(thr);				dbg_show(2, "img", filled);
	tmp	= dilate_erode(filled, 2);			dbg_show(2, "img", tmp);
	syms	= erode_dilate(tmp, 10);			dbg_show(1, "img", syms);
	return	syms;

def find_symbols(img):
	cnts	= contours(img);
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
		thr	= threshold(gray, 64, cv.THRESH_BINARY);
		dbg_show(1, "img", thr);
		syms.append(thr);
	return	syms;

def clean_symbols(syms):
	syms_clean	= list();
	for sym in syms:
		inv	= cv.bitwise_not(sym);			dbg_show(2, "img", inv);
		tmp	= dilate(inv, 3);			dbg_show(2, "img", tmp);
		cnts	= contours(tmp);
		cnt_i	= largest_cnt(cnts);
		mask	= np.zeros(tmp.shape, np.uint8);	dbg_show(2, "img", mask);
		cv.drawContours(mask, cnts, cnt_i, 255, -1);	dbg_show(2, "img", mask);
		nsym_clean	= cv.bitwise_and(inv, mask);	dbg_show(2, "img", nsym_clean);
		sym_clean	= cv.bitwise_not(nsym_clean);	dbg_show(1, "img", sym_clean);
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
		m		= cmp_template(t_norm, img_base_n, 2);
		if m > match:
			code	= i;
			match	= m;
	if match < thr:
		code	= "error";
	dbg_printf(2, "%s (match: %.3lf)\n", code, match);
	return	code;

def find_details(img, code, temps):
	full_code	= code;
	for c in t_base_names:
		dbg_printf(2, "%s =? %s\n", c, code);
		if c == code:
			dbg_printf(1, "%s;", code);
			sym_i	= sym_inside(img);		dbg_show(1, "img", sym_i);
			dbg_printf(2, "\n");
			code_i_	= match_t_inner(sym_i, temps);
			code_i	= decode_inner(code, code_i_);
			full_code += "; " + code_i;		dbg_printf(1, " %s;", code_i);
			sym_o	= sym_outside(img);		dbg_show(1, "img", sym_o);
			code_o	= sym_out_code(sym_o);
			if code_o:
				full_code += "; " + code_o;	dbg_printf(1, " %s\n", code_o);
	return	full_code;

def get_washing_instruction_id(instruction):
	switcher = {
		"hand_wash": 0,
		"wash": 1,
		"wash; delicate": 2,
		"wash; very_delicate": 3,
		"wash_not": 4,
		"wash; 30": 5,
		"wash; 40": 6,
		"wash; 50": 7,
		"wash; 60": 8,
		"wash; 70": 9,
		"wash; 95": 10,
		"wiring_not": 11,
		"wiring": 12,
		"bleach": 13,
		"bleach_not": 14,
		"bleach; non_Cl": 15,
		"bleach; Cl": 16,
		"tumble_dry": 17,
		"tumble_dry; low_temp": 18,
		"tumble_dry; medium_temp": 19,
		"tumble_dry; high_temp": 20,
		"tumble_dry; no heat": 21,
		"tumble_dry_not": 22,
		"dry; line": 23,
		"dry; shade": 24,
		"dry; line_shade": 25,
		"dry; drip": 26,
		"dry; drip_shade": 27,
		"dry; flat": 28,
		"dry; flat_shade": 29,
		"dry; natural": 30,
		"dry_not": 31,
		"iron": 32,
		"iron; low_temp": 33,
		"iron; medium_temp": 34,
		"iron; high_temp": 35,
		"iron_not": 36,
		"iron_steam": 37,
		"iron_steam_not": 38,
		"wet_clean": 39,
		"wet_clean; delicate": 40,
		"wet_clean; very_delicate": 41,
		"dry_clean": 42,
		"dry_clean; any_solvent": 43,
		"dry_clean; petroleum_only": 44,
		"dry_clean; petroleum_only; delicate": 45,
		"dry_clean; petroleum_only; very_delicate": 46,
		"dry_clean; any_solvent_except_TCE": 47,
		"dry_clean; any_solvent_except_TCE; delicate": 48,
		"dry_clean; any_solvent_except_TCE; very_delicate": 49,
		"dry_clean_not": 50,
		"wet_clean_not": 51,
		"dry_clean; short_cycle": 52,
		"dry_clean; reduced_moisture": 53,
		"dry_clean; no_steam": 54,
		"dry_clean; low_heat": 55,
		"wash; 30; delicate": 100

	}
	return switcher.get(instruction, -1)

def print_format_output(codes):
	alx.printf("{\n");
	alx.printf("\"success\":true,\n");
	alx.printf("\"message\":\n");
	alx.printf("\"\n");
	for code in codes:
		alx.printf("%s\n", code);
	alx.printf("\"\n");
	alx.printf("}\n");
	return	None;


################################################################################
#	main								       #
################################################################################
def main():

	dbg_printf(1, "Hello, world!\n");
	dbg_printf(1, "We have %i days to finish this\n", 0);
	if (DBG):
		cv.namedWindow("img");

	templates	= get_templates();			dbg_show_templates(2, "img", templates);
	templates_in	= get_templates_inner();		dbg_show_templates(2, "img", templates_in);
	img		= img_get();				dbg_show(1, "img", img);

	rotrect	= find_label(img);
	align	= align_label(img, rotrect);
	label	= crop_label(align, rotrect);
	cnts	= find_symbols_loc(label);
	symlocs	= find_symbols(cnts);
	syms	= crop_symbols(label, symlocs);
	syms	= clean_symbols(syms);
	codes	= list();
	for sym in syms:
		dbg_show(1, "img", sym);
		code	= match_templates(sym, templates);
		dbg_printf(1, "%s\n", code);
		full_code	= find_details(sym, code, templates_in);
		dbg_printf(1, "%s\n", full_code);
		codes.append(full_code);
	array = []
	for code in codes:
		array.append(get_washing_instruction_id(code));
#		alx.printf("%s\n", code);

	print_format_output(codes);

#	print(array);


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

