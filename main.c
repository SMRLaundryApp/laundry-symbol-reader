/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


#define ALX_NO_PREFIX
#include <libalx/base/errno/error.h>
#include <libalx/base/stdio/seekc.h>
#include <libalx/base/stdlib/strto/strtoi_s.h>
#include <libalx/extra/cv/core/array.h>
#include <libalx/extra/cv/core/contours.h>
#include <libalx/extra/cv/core/img.h>
#include <libalx/extra/cv/core/rect.h>
#include <libalx/extra/cv/core/roi.h>
#include <libalx/extra/cv/highgui/file.h>
#include <libalx/extra/cv/highgui/window.h>
#include <libalx/extra/cv/imgproc/filter.h>
#include <libalx/extra/cv/imgproc/geometric.h>
#include <libalx/extra/cv/imgproc/miscellaneous.h>
#include <libalx/extra/cv/imgproc/shape.h>
#include <libalx/extra/cv/types.h>


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/
#define DBG

#ifdef DBG
#define dbg_show(img, msg)	do					\
{									\
	perrorx(msg);							\
	alx_cv_imshow(img, "dbg", -1);					\
} while (0);
#else
#define dbg_show(img, msg)	do {} while (0);
#endif


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
int	init	(img_s **restrict img);
static
void	deinit	(img_s *restrict img);
static
int	proc	(const char *fname);

static
int	find_label	(img_s *restrict img);


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
int	init	(img_s **restrict img)
{

	if (alx_cv_alloc_img(img))
		return	-1;
	if (alx_cv_init_img(*img, 1, 1))
		goto err;
	alx_cv_named_window("dbg", ALX_CV_WINDOW_KEEPRATIO);
	return	0;

err:	alx_cv_free_img(*img);
	return	-1;
}

static
void	deinit	(img_s *restrict img)
{

	alx_cv_destroy_all_windows();
	alx_cv_deinit_img(img);
	alx_cv_free_img(img);
}

static
int	proc	(const char *fname)
{
	img_s	*img;
	clock_t	time_0;
	clock_t	time_1;
	double	time_tot;
	int	status;

	status	= -1;
	if (init(&img))
		return	-1;
	time_0 = clock();

	if (alx_cv_imread(img, fname))
		goto err;
	if (find_label(img))
		goto err;

	time_1 = clock();
	time_tot = ((double) time_1 - time_0) / CLOCKS_PER_SEC;
	printf("Total time:	%5.3lf s;\n", time_tot);

	alx_cv_imwrite(img, "wash.png");

	status	= 0;
err:	deinit(img);
	return	status;
}

static
int	find_label	(img_s *restrict img)
{
	img_s		*tmp;
	conts_s		*conts;
	ptrdiff_t	l;
	const cont_s	*lbl;
	rect_rot_s	*rect_rot;
	rect_s		*rect;
	ptrdiff_t	ctr_x, ctr_y, w, h, x, y;
	int		status;

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
	alx_cv_clone(tmp, img);					dbg_show(tmp, NULL);
	alx_cv_cvt_color(tmp, ALX_CV_COLOR_BGR2HSV);		dbg_show(tmp, NULL);
	alx_cv_component(tmp, ALX_CV_CMP_S);			dbg_show(tmp, NULL);
	alx_cv_smooth(tmp, ALX_CV_SMOOTH_MEDIAN, 71);		dbg_show(tmp, NULL);
	alx_cv_threshold(tmp, ALX_CV_THRESH_BINARY_INV, 30);	dbg_show(tmp, NULL);
	alx_cv_erode_dilate(tmp, 80);				dbg_show(tmp, NULL);
	alx_cv_contours(tmp, conts);
	l	= alx_cv_conts_largest(conts);
	if (alx_cv_extract_conts_cont(&lbl, conts, l))
		goto err;
	alx_cv_min_area_rect(rect_rot, lbl);

	/* Align label */
	alx_cv_cvt_color(img, ALX_CV_COLOR_BGR2GRAY);		dbg_show(img, NULL);
	alx_cv_invert(img);					dbg_show(img, NULL);
	alx_cv_rotate_2rect(img, rect_rot);			dbg_show(img, NULL);

	/* Crop to label */
	status--;
	alx_cv_extract_rect_rot(rect_rot, &ctr_x, &ctr_y, &w, &h, NULL);
	x	= ctr_x - w / 2;
	y	= ctr_y - h / 2;
	if (alx_cv_init_rect(rect, x, y, w, h))
		goto err;
	alx_cv_roi_set(img, rect);				dbg_show(img, NULL);

	status	= 0;
err:	alx_cv_free_rect(rect);
err3:	alx_cv_free_rect_rot(rect_rot);
err2:	alx_cv_deinit_conts(conts);
	alx_cv_free_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_free_img(tmp);
	return	status;
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
