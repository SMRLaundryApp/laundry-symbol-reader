/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


#define ALX_NO_PREFIX
#include <libalx/base/errno/perror.h>
#include <libalx/base/stdio/seekc.h>
#include <libalx/base/stdlib/maximum.h>
#include <libalx/base/stdlib/minimum.h>
#include <libalx/base/stdlib/strto/strtoi_s.h>
#include <libalx/extra/cv/alx/gray.h>
#include <libalx/extra/cv/alx/lines.h>
#include <libalx/extra/cv/alx/median.h>
#include <libalx/extra/cv/core/array/bitwise.h>
#include <libalx/extra/cv/core/array/component.h>
#include <libalx/extra/cv/core/array/normalize.h>
#include <libalx/extra/cv/core/contours/contours.h>
#include <libalx/extra/cv/core/img/img.h>
#include <libalx/extra/cv/core/rect/rect.h>
#include <libalx/extra/cv/core/roi/roi.h>
#include <libalx/extra/cv/highgui/file.h>
#include <libalx/extra/cv/highgui/window.h>
#include <libalx/extra/cv/imgproc/features/edges.h>
#include <libalx/extra/cv/imgproc/filter/border.h>
#include <libalx/extra/cv/imgproc/filter/dilate_erode.h>
#include <libalx/extra/cv/imgproc/filter/edges.h>
#include <libalx/extra/cv/imgproc/filter/smooth.h>
#include <libalx/extra/cv/imgproc/geometric/rotate.h>
#include <libalx/extra/cv/imgproc/histogram/hist.h>
#include <libalx/extra/cv/imgproc/miscellaneous/color.h>
#include <libalx/extra/cv/imgproc/miscellaneous/fill.h>
#include <libalx/extra/cv/imgproc/miscellaneous/threshold.h>
#include <libalx/extra/cv/imgproc/shape/contours.h>
#include <libalx/extra/cv/imgproc/shape/rect.h>
#include <libalx/extra/cv/types.h>


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/
#define DBG	03

#ifdef DBG
#define dbg_show(dbg, img, msg)		do				\
{									\
									\
	if (dbg <= DBG) {						\
		perrorx(msg);						\
		alx_cv_imshow(img, "dbg", -1);				\
	}								\
} while (0);

#define dbg_update_win()		do				\
{									\
									\
	alx_cv_destroy_all_windows();					\
	alx_cv_named_window("dbg", ALX_CV_WINDOW_NORMAL);		\
} while (0);
#else
#define dbg_show(img, msg)	do {} while (0);
#define dbg_update_win()	do {} while (0);
#endif

#define MAX_SYMBOLS	(6)


/******************************************************************************
 ******* enum *****************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* struct / union *******************************************************
 ******************************************************************************/


/******************************************************************************
 ******* static functions (prototypes) ****************************************
 ******************************************************************************/
static
int	init	(img_s **restrict img, img_s *syms[static restrict MAX_SYMBOLS]);
static
void	deinit	(img_s *restrict img, img_s *syms[static restrict MAX_SYMBOLS]);
static
int	proc	(const char *fname);

static
int	find_label			(img_s *img);
static
int	find_symbols_vertically		(img_s *img);
static
int	find_symbols_horizontally	(img_s *img);
static
int	align_symbols			(img_s *img);
static
int	isolate_symbols			(img_s *restrict img,
					 img_s *syms[static restrict MAX_SYMBOLS],
					 ptrdiff_t *restrict n);
static
int	clean_symbol	(img_s *sym);


/******************************************************************************
 ******* main *****************************************************************
 ******************************************************************************/
int	main	(int argc, char *argv[])
{
	const char	*fname;

	if (argc != 2)
		return	1;

	fname	= argv[1];
	if (proc(fname))
		return	2;

	return	0;
}


/******************************************************************************
 ******* static functions (definitions) ***************************************
 ******************************************************************************/
static
int	init	(img_s **restrict img, img_s *syms[static restrict MAX_SYMBOLS])
{
	ptrdiff_t	i;

	if (alx_cv_alloc_img(img))
		return	-1;
	if (alx_cv_init_img(*img, 1, 1))
		goto err0;
	for (i = 0; i < MAX_SYMBOLS; i++) {
		if (alx_cv_alloc_img(&syms[i]))
			goto err1;
		if (alx_cv_init_img(syms[i], 1, 1))
			goto err2;
	}
	alx_cv_named_window("dbg", ALX_CV_WINDOW_NORMAL);
	return	0;

	for (; i >= 0; i--) {
		alx_cv_deinit_img(syms[i]);
err2:		alx_cv_free_img(syms[i]);
err1:		continue;
	}
	alx_cv_deinit_img(*img);
err0:	alx_cv_free_img(*img);
	return	-1;
}

static
void	deinit	(img_s *restrict img, img_s *syms[static restrict MAX_SYMBOLS])
{

	alx_cv_destroy_all_windows();
	for (ptrdiff_t i = MAX_SYMBOLS - 1; i >= 0; i--) {
		alx_cv_deinit_img(syms[i]);
		alx_cv_free_img(syms[i]);
	}
	alx_cv_deinit_img(img);
	alx_cv_free_img(img);
}

static
int	proc	(const char *fname)
{
	img_s		*img, *syms[MAX_SYMBOLS];
	ptrdiff_t	nsyms;
	clock_t		time_0;
	clock_t		time_1;
	double		time_tot;
	int		status;

	status	= -1;
	if (init(&img, syms))
		return	-1;
	time_0 = clock();

	status--;
	if (alx_cv_imread(img, fname))
		goto err;
	status--;
	if (find_label(img))
		goto err;
	status--;
	if (find_symbols_vertically(img))
		goto err;
	status--;
	if (find_symbols_horizontally(img))
		goto err;
	status--;
	if (align_symbols(img))
		goto err;
	status--;
	if (isolate_symbols(img, syms, &nsyms))
		goto err;
	status--;
	for (ptrdiff_t i = 0; i < nsyms; i++) {
		if (clean_symbol(syms[i]))
			goto err;
	}

	time_1 = clock();
	time_tot = ((double) time_1 - time_0) / CLOCKS_PER_SEC;
	printf("Total time:	%5.3lf s;\n", time_tot);

	alx_cv_imwrite(img, "wash.png");

	status	= 0;
err:	deinit(img, syms);
	return	status;
}

static
int	find_label			(img_s *img)
{
	img_s		*tmp;
	conts_s		*conts;
	const cont_s	*lbl;
	rect_rot_s	*rect_rot;
	rect_s		*rect;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_alloc_img(&tmp))
		return	status;
	if (alx_cv_init_img(tmp, 1, 1))
		goto err0;
	if (alx_cv_alloc_conts(&conts))
		goto err1;
	alx_cv_init_conts(conts);
	if (alx_cv_alloc_rect_rot(&rect_rot))
		goto err2;
	if (alx_cv_alloc_rect(&rect))
		goto err3;

	/* Find label */
	status--;
	alx_cv_clone(tmp, img);					dbg_show(2, tmp, NULL);
	alx_cv_white_mask(tmp, 50, 60, 40);			dbg_show(3, tmp, NULL);
	alx_cv_dilate_erode(tmp, 10);				dbg_show(3, tmp, NULL);
	alx_cv_erode_dilate(tmp, 30);				dbg_show(3, tmp, NULL);
	alx_cv_contours(tmp, conts);				dbg_show(2, tmp, NULL);
	if (alx_cv_conts_largest(&lbl, NULL, conts))
		goto err;
	alx_cv_min_area_rect(rect_rot, lbl);

	/* Align & crop to label */
	status--;
	alx_cv_rotate_2rect(img, rect_rot, rect);		dbg_show(3, img, NULL);
	alx_cv_roi_set(img, rect);		dbg_update_win(); dbg_show(1, img, NULL);

	/* deinit */
	status	= 0;
err:	alx_cv_free_rect(rect);
err3:	alx_cv_free_rect_rot(rect_rot);
err2:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_free_img(tmp);
	return	status;
}

static
int	find_symbols_vertically		(img_s *img)
{
	img_s		*clean, *tmp, *bkgd;
	conts_s		*conts;
	const cont_s	*syms;
	rect_s		*rect;
	ptrdiff_t	x, y, w, h;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_alloc_img(&clean))
		return	status;
	if (alx_cv_init_img(clean, 1, 1))
		goto err0;
	if (alx_cv_alloc_img(&bkgd))
		goto err1;
	if (alx_cv_init_img(bkgd, 1, 1))
		goto err2;
	if (alx_cv_alloc_img(&tmp))
		goto err3;
	if (alx_cv_init_img(tmp, 1, 1))
		goto err4;
	if (alx_cv_alloc_conts(&conts))
		goto err5;
	alx_cv_init_conts(conts);
	if (alx_cv_alloc_rect(&rect))
		goto err6;

	/* Clean BKGD */
	status--;
	alx_cv_clone(tmp, img);					dbg_show(2, tmp, NULL);
	alx_cv_component(img, ALX_CV_CMP_BGR_R);		dbg_show(3, img, NULL);
	alx_cv_smooth(img, ALX_CV_SMOOTH_MEDIAN, 3);		dbg_show(3, img, NULL);
	alx_cv_clone(bkgd, img);				dbg_show(2, bkgd, NULL);
	alx_cv_clone(clean, img);				dbg_show(3, clean, NULL);
	alx_cv_white_mask(tmp, -1, 32, 64);			dbg_show(3, tmp, NULL);
	alx_cv_dilate_erode(tmp, 5);				dbg_show(3, tmp, NULL);
	alx_cv_bkgd_mask(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_dilate(tmp, 10);					dbg_show(3, tmp, NULL);
	alx_cv_median(bkgd);					dbg_show(3, bkgd, NULL);
	alx_cv_and_2ref(bkgd, tmp);				dbg_show(3, bkgd, NULL);
	alx_cv_invert(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_and_2ref(clean, tmp);				dbg_show(3, clean, NULL);
	alx_cv_or_2ref(clean, bkgd);				dbg_show(2, clean, NULL);

	/* Find syms */
	status--;
	alx_cv_clone(tmp, clean);				dbg_show(3, tmp, NULL);
	alx_cv_normalize(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_smooth(tmp, ALX_CV_SMOOTH_MEDIAN, 5);		dbg_show(3, tmp, NULL);
	alx_cv_canny(tmp, 127, 200, 3, true);			dbg_show(3, tmp, NULL);
	alx_cv_dilate_h(tmp, 1);				dbg_show(3, tmp, NULL);
	alx_cv_dilate(tmp, 1);					dbg_show(3, tmp, NULL);
	alx_cv_holes_fill(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_erode_dilate(tmp, 15);				dbg_show(3, tmp, NULL);
	alx_cv_extract_imgdata(tmp, NULL, &w, NULL, NULL, NULL, NULL);
	alx_cv_dilate_h(tmp, w / 6);				dbg_show(3, tmp, NULL);
	alx_cv_contours(tmp, conts);				dbg_show(2, tmp, NULL);
	if (alx_cv_conts_largest(&syms, NULL, conts))
		goto err; 
	alx_cv_bounding_rect(rect, syms);
	alx_cv_extract_rect(rect, NULL, &y, NULL, &h);

	/* Crop to symbols */
	status--;
	y	+= h / 2;
	h	*= 2;
	y	-= h / 2;
	x	= 0;
	if (alx_cv_init_rect(rect, x, y, w, h))
		goto err;
	alx_cv_roi_set(img, rect);		dbg_update_win(); dbg_show(1, img, NULL);

	/* deinit */
	status	= 0;
err:	alx_cv_free_rect(rect);
err6:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err5:	alx_cv_deinit_img(tmp);
err4:	alx_cv_free_img(tmp);
err3:	alx_cv_deinit_img(bkgd);
err2:	alx_cv_free_img(bkgd);
err1:	alx_cv_deinit_img(clean);
err0:	alx_cv_free_img(clean);
	return	status;
}

static
int	find_symbols_horizontally	(img_s *img)
{
	img_s		*tmp;
	conts_s		*conts;
	const cont_s	*syms;
	rect_s		*rect;
	ptrdiff_t	x, y, w, h;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_alloc_img(&tmp))
		return	status;
	if (alx_cv_init_img(tmp, 1, 1))
		goto err0;
	if (alx_cv_alloc_conts(&conts))
		goto err1;
	alx_cv_init_conts(conts);
	if (alx_cv_alloc_rect(&rect))
		goto err2;

	/* Find symbols */
	status--;
	alx_cv_clone(tmp, img);					dbg_show(2, tmp, NULL);
	alx_cv_extract_imgdata(tmp, NULL, &w, &h, NULL, NULL, NULL);
	alx_cv_init_rect(rect, 20, 0, w - 40, h);
	alx_cv_roi_set(tmp, rect);		dbg_update_win(); dbg_show(3, tmp, NULL);
	alx_cv_normalize(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_threshold(tmp, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(3, tmp, NULL);
	alx_cv_dilate_h(tmp, w / 40);				dbg_show(3, tmp, NULL);
	alx_cv_dilate_v(tmp, h / 40);				dbg_show(3, tmp, NULL);
	alx_cv_contours(tmp, conts);				dbg_show(2, tmp, NULL);
	if (alx_cv_conts_largest(&syms, NULL, conts))
		goto err;
	alx_cv_bounding_rect(rect, syms);
	alx_cv_extract_rect(rect, &x, NULL, &w, NULL);

	/* Crop to symbols */
	status--;
	y	= 0;
	w	+= 40;
	if (alx_cv_init_rect(rect, x, y, w, h))
		goto err;
	alx_cv_roi_set(img, rect);		dbg_update_win(); dbg_show(1, img, NULL);

	/* deinit */
	status	= 0;
err:	alx_cv_free_rect(rect);
err2:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_free_img(tmp);
	return	status;
}

static
int	align_symbols			(img_s *img)
{
	img_s		*tmp;
	conts_s		*conts;
	const cont_s	*syms;
	rect_rot_s	*rect_rot;
	rect_s		*rect;
	ptrdiff_t	w, h;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_alloc_img(&tmp))
		return	status;
	if (alx_cv_init_img(tmp, 1, 1))
		goto err0;
	if (alx_cv_alloc_conts(&conts))
		goto err1;
	alx_cv_init_conts(conts);
	if (alx_cv_alloc_rect_rot(&rect_rot))
		goto err2;
	if (alx_cv_alloc_rect(&rect))
		goto err3;

	/* Find symbols */
	status--;
	alx_cv_clone(tmp, img);					dbg_show(2, tmp, NULL);
	alx_cv_extract_imgdata(tmp, NULL, &w, &h, NULL, NULL, NULL);
	alx_cv_normalize(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_threshold(tmp, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(3, tmp, NULL);
	alx_cv_dilate_h(tmp, w / 40);				dbg_show(3, tmp, NULL);
	alx_cv_dilate_v(tmp, h / 40);				dbg_show(3, tmp, NULL);
	alx_cv_contours(tmp, conts);				dbg_show(2, tmp, NULL);
	if (alx_cv_conts_largest(&syms, NULL, conts))
		goto err;
	alx_cv_min_area_rect(rect_rot, syms);

	/* Aling & crop to symbols */
	alx_cv_rotate_2rect(img, rect_rot, rect);		dbg_show(3, img, NULL);
	alx_cv_roi_set(img, rect);		dbg_update_win(); dbg_show(1, img, NULL);

	/* deinit */
	status	= 0;
err:	alx_cv_free_rect(rect);
err3:	alx_cv_free_rect_rot(rect_rot);
err2:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_free_img(tmp);
	return	status;
}

static
int	isolate_symbols			(img_s *restrict img,
					 img_s *syms[static restrict MAX_SYMBOLS],
					 ptrdiff_t *restrict n)
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
	if (alx_cv_alloc_img(&tmp))
		return	status;
	if (alx_cv_init_img(tmp, 1, 1))
		goto err0;
	if (alx_cv_alloc_conts(&conts))
		goto err1;
	alx_cv_init_conts(conts);
	if (alx_cv_alloc_rect(&rect))
		goto err2;

	/* Find symbols */
	status--;
	alx_cv_clone(tmp, img);					dbg_show(2, tmp, NULL);
	alx_cv_normalize(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_smooth(tmp, ALX_CV_SMOOTH_MEDIAN, 3);		dbg_show(3, tmp, NULL);
	alx_cv_canny(tmp, 127, 200, 3, false);			dbg_show(3, tmp, NULL);
//	alx_cv_threshold(tmp, ALX_CV_THRESH_BINARY_INV, 80);	dbg_show(3, tmp, NULL);
	alx_cv_dilate(tmp, 1);					dbg_show(3, tmp, NULL);
//	alx_cv_dilate_erode(tmp, 8);				dbg_show(3, tmp, NULL);
	alx_cv_holes_fill(tmp);					dbg_show(3, tmp, NULL);
	alx_cv_erode_dilate(tmp, 12);				dbg_show(3, tmp, NULL);
	alx_cv_contours(tmp, conts);				dbg_show(2, tmp, NULL);
	if (alx_cv_extract_conts(conts, NULL, n))
		goto err;
	printf("--%i--\n", (int)*n);

	/* Crop to symbols */
	status--;
	y_all	= PTRDIFF_MAX;
	w_all	= 0;
	h_all	= 0;
	for (ptrdiff_t i = 0; i < *n; i++) {
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
	for (ptrdiff_t i = 0; i < *n; i++) {
		alx_cv_clone(syms[i], img);			dbg_show(3, syms[i], NULL);
		if (alx_cv_extract_conts_cont(&cont, conts, i))
			goto err;
		alx_cv_bounding_rect(rect, cont);
		alx_cv_extract_rect(rect, &x, NULL, &w, NULL);
		x	+= w / 2 - w_all / 2;
		if (alx_cv_init_rect(rect, x, y_all, w_all, h_all))
			goto err;
		alx_cv_roi_set(syms[i], rect);	dbg_update_win(); dbg_show(1, syms[i], NULL);
	}

	/* deinit */
	status	= 0;
err:	alx_cv_free_rect(rect);
err2:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_free_img(tmp);
	return	status;
}

static
int	clean_symbol	(img_s *img)
{
	img_s		*mask, *bkgd;
	conts_s		*conts;
	ptrdiff_t	w, h;
	ptrdiff_t	i;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_alloc_img(&mask))
		return	status;
	if (alx_cv_init_img(mask, 1, 1))
		goto err0;
	if (alx_cv_alloc_img(&bkgd))
		goto err1;
	if (alx_cv_init_img(bkgd, 1, 1))
		goto err2;
	if (alx_cv_alloc_conts(&conts))
		goto err3;
	alx_cv_init_conts(conts);

	/* Find symbol */
	status--;
	alx_cv_clone(mask, img);				dbg_show(2, mask, NULL);
	alx_cv_threshold(mask, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(3, mask, NULL);
	alx_cv_dilate(mask, 2);					dbg_show(3, mask, NULL);
	alx_cv_holes_fill(mask);				dbg_show(3, mask, NULL);
	alx_cv_contours(mask, conts);				dbg_show(2, mask, NULL);
	alx_cv_extract_imgdata(mask, NULL, &w, &h, NULL, NULL, NULL);
	if (alx_cv_conts_closest(NULL, &i, conts, w / 2, h / 2, NULL))
		goto err;
	alx_cv_contour_mask(mask, conts, i);			dbg_show(2, mask, NULL);
	alx_cv_dilate(mask, 2);					dbg_show(3, mask, NULL);

	/* Find BKGD */
	alx_cv_clone(bkgd, img);				dbg_show(2, bkgd, NULL);
	alx_cv_median(bkgd);					dbg_show(2, bkgd, NULL);
	alx_cv_invert(mask);					dbg_show(2, mask, NULL);
	alx_cv_and_2ref(bkgd, mask);				dbg_show(2, bkgd, NULL);

	/* Clean symbol */
	alx_cv_invert(mask);					dbg_show(2, mask, NULL);
	alx_cv_and_2ref(img, mask);				dbg_show(2, img, NULL);
	alx_cv_or_2ref(img, bkgd);				dbg_show(2, img, NULL);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err3:	alx_cv_deinit_img(bkgd);
err2:	alx_cv_free_img(bkgd);
err1:	alx_cv_deinit_img(mask);
err0:	alx_cv_free_img(mask);
	return	status;
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
