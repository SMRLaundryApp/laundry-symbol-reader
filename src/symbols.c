/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "symbols.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ALX_NO_PREFIX
#include <libalx/base/compiler.h>
#include <libalx/base/errno.h>
#include <libalx/base/stdlib.h>
#include <libalx/extra/cv/cv.h>

#include "dbg.h"
#include "templates/templates.h"


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/
#define MAX_SYMBOLS	(5)


/******************************************************************************
 ******* enum / struct / union ************************************************
 ******************************************************************************/


/******************************************************************************
 ******* variables ************************************************************
 ******************************************************************************/
img_s		*symbols[MAX_SYMBOLS];
ptrdiff_t	nsyms;


/******************************************************************************
 ******* static prototypes ****************************************************
 ******************************************************************************/


/******************************************************************************
 ******* global functions *****************************************************
 ******************************************************************************/
int	init_symbols	(void)
{
	ptrdiff_t	i;

	for (i = 0; i < ARRAY_SSIZE(symbols); i++) {
		if (alx_cv_init_img(&symbols[i]))
			goto err0;
	}
	nsyms	= 0;

	return	0;

err0:	for (i--; i >= 0; i--)
		alx_cv_deinit_img(symbols[i]);
	return	-1;
}

void	deinit_symbols	(void)
{

	nsyms	= 0;
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(symbols); i++)
		alx_cv_deinit_img(symbols[i]);
}

int	extract_symbols	(img_s *restrict img)
{
	img_s		*tmp;
	conts_s		*conts;
	const cont_s	*cont;
	rect_s		*rect;
	ptrdiff_t	x, y, w, h;
	ptrdiff_t	y_all, w_all, h_all;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&tmp))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;
	if (alx_cv_init_rect(&rect))
		goto err1;

	/* Find symbols */
	status--;
	alx_cv_extract_imgdata(img, NULL, &w, &h, NULL, NULL, NULL);
	alx_cv_clone(tmp, img);					dbg_show(2, tmp);
	alx_cv_normalize(tmp);					dbg_show(3, tmp);
	alx_cv_smooth(tmp, ALX_CV_SMOOTH_MEDIAN, 3);		dbg_show(3, tmp);
	alx_cv_adaptive_thr(tmp, ALX_CV_ADAPTIVE_THRESH_GAUSSIAN,
			ALX_CV_THRESH_BINARY_INV, h / 2, 5);	dbg_show(3, tmp);
	alx_cv_holes_fill(tmp);
	alx_cv_erode_dilate(tmp, h / 15);			dbg_show(3, tmp);
	alx_cv_dilate_erode(tmp, h / 15);			dbg_show(3, tmp);
	alx_cv_contours(tmp, conts);
	alx_cv_sort_conts_lr(conts);
	if (alx_cv_extract_conts(conts, NULL, &nsyms))
		goto err;
	if (nsyms != 5) {
		perrorx("[error]	%i symbols detected\n", (int)nsyms);
		goto err;
	}

	/* Crop to symbols */
	status--;
	y_all	= PTRDIFF_MAX;
	w_all	= 0;
	h_all	= 0;
	for (ptrdiff_t i = 0; i < nsyms; i++) {
		if (alx_cv_extract_conts_cont(&cont, conts, i))
			goto err;
		alx_cv_bounding_rect(rect, cont);
		alx_cv_extract_rect(rect, &x, &y, &w, &h);
		if (w > w_all)
			w_all	= w;
		if (h > h_all)
			h_all	= h;
		if (y < y_all) {
			if (y_all != PTRDIFF_MAX)
				h_all	+= y_all - y;
			y_all	= y;
		}
	}
	y_all	+= h_all / 2;
	h_all	*= 1.2;
	y_all	-= h_all / 2;
	w_all	*= 1.4;
	for (ptrdiff_t i = 0; i < nsyms; i++) {
		alx_cv_clone(symbols[i], img);			dbg_show(3, symbols[i]);
		if (alx_cv_extract_conts_cont(&cont, conts, i))
			goto err;
		alx_cv_bounding_rect(rect, cont);
		alx_cv_extract_rect(rect, &x, NULL, &w, NULL);
		x	+= w / 2 - w_all / 2;
		alx_cv_set_rect(rect, x, y_all, w_all, h_all);
		alx_cv_roi_set(symbols[i], rect);		dbg_show(1, symbols[i]);
	}

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err1:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(tmp);
	return	status;
}

int	clean_symbol	(img_s *img)
{
	img_s		*mask, *bkgd;
	conts_s		*conts;
	ptrdiff_t	w, h;
	ptrdiff_t	i;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&mask))
		return	status;
	if (alx_cv_init_img(&bkgd))
		goto err0;
	if (alx_cv_init_conts(&conts))
		goto err1;

	/* Find symbol */
	status--;
	alx_cv_clone(mask, img);				dbg_show(2, mask);
	alx_cv_threshold(mask, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(3, mask);
	alx_cv_dilate(mask, 2);					dbg_show(3, mask);
	alx_cv_holes_fill(mask);				dbg_show(3, mask);
	alx_cv_contours(mask, conts);
	alx_cv_extract_imgdata(mask, NULL, &w, &h, NULL, NULL, NULL);
	if (alx_cv_conts_closest(NULL, &i, conts, w / 2, h / 2, NULL))
		goto err;
	alx_cv_contour_mask(mask, conts, i);			dbg_show(3, mask);
	alx_cv_dilate(mask, 2);					dbg_show(3, mask);

	/* Find BKGD */
	alx_cv_clone(bkgd, img);				dbg_show(3, bkgd);
	alx_cv_median(bkgd);					dbg_show(3, bkgd);
	alx_cv_invert(mask);					dbg_show(3, mask);
	alx_cv_and_2ref(bkgd, mask);				dbg_show(3, bkgd);

	/* Clean symbol */
	alx_cv_invert(mask);					dbg_show(3, mask);
	alx_cv_and_2ref(img, mask);				dbg_show(3, img);
	alx_cv_or_2ref(img, bkgd);				dbg_show(3, img);

	/* Threshold */
	alx_cv_normalize(img);					dbg_show(3, img);
	alx_cv_smooth(img, ALX_CV_SMOOTH_MEDIAN, 3);		dbg_show(3, img);
	w	= ALX_MIN(w, h);
	alx_cv_adaptive_thr(img, ALX_CV_ADAPTIVE_THRESH_GAUSSIAN,
			ALX_CV_THRESH_BINARY_INV, w / 2, 25);	dbg_show(1, img);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err1:	alx_cv_deinit_img(bkgd);
err0:	alx_cv_deinit_img(mask);
	return	status;
}

int	symbol_base	(const img_s *restrict sym, img_s *restrict base)
{
	img_s		*mask;
	conts_s		*conts;
	const cont_s	*cont;
	rect_s		*rect;
	ptrdiff_t	i;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&mask))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;
	if (alx_cv_init_rect(&rect))
		goto err1;

	/* Find base */
	status--;
	alx_cv_clone(base, sym);				dbg_show(2, base);
	alx_cv_holes_remove(base);				dbg_show(3, base);
	alx_cv_clone(mask, base);				dbg_show(3, mask);
	alx_cv_contours(mask, conts);
	if (alx_cv_conts_largest_a(&cont, &i, conts))
		goto err;
	alx_cv_contour_mask(mask, conts, i);			dbg_show(3, mask);
	alx_cv_and_2ref(base, mask);				dbg_show(3, base);
	alx_cv_bounding_rect(rect, cont);
	alx_cv_roi_set(base, rect);				dbg_show(1, base);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err1:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(mask);
	return	status;
}

int	symbol_inner	(const img_s *restrict sym, img_s *restrict in)
{
	img_s		*mask;
	conts_s		*conts;
	const cont_s	*cont;
	rect_s		*rect;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&mask))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;
	if (alx_cv_init_rect(&rect))
		goto err1;

	/* Find inner */
	status--;
	alx_cv_clone(in, sym);					dbg_show(2, in);
	alx_cv_holes_extract(in);				dbg_show(3, in);
	alx_cv_clone(mask, in);					dbg_show(3, mask);
	alx_cv_dilate_erode(mask, 10);				dbg_show(3, mask);
	alx_cv_contours(mask, conts);
	if (alx_cv_conts_largest_a(&cont, NULL, conts))
		goto err;
	alx_cv_bounding_rect(rect, cont);
	alx_cv_roi_set(in, rect);				dbg_show(1, in);


	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err1:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(mask);
	return	status;
}

int	symbol_outer	(const img_s *restrict sym, img_s *restrict out)
{
	img_s		*mask;
	conts_s		*conts;
	ptrdiff_t	i;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&mask))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;

	/* Find outer */
	status--;
	alx_cv_clone(out, sym);					dbg_show(2, out);
	alx_cv_holes_fill(out);					dbg_show(3, mask);
	alx_cv_clone(mask, out);				dbg_show(3, mask);
	alx_cv_contours(mask, conts);
	if (alx_cv_conts_largest_a(NULL, &i, conts))
		goto err;
	alx_cv_contour_mask(mask, conts, i);			dbg_show(3, mask);
	alx_cv_invert(mask);					dbg_show(3, mask);
	alx_cv_and_2ref(out, mask);				dbg_show(3, out);
	alx_cv_erode_dilate(out, 1);				dbg_show(1, out);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(mask);
	return	status;
}


/******************************************************************************
 ******* static function definitions ******************************************
 ******************************************************************************/


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/

