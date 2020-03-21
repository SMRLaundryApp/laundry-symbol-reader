/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "templates.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define ALX_NO_PREFIX
#include <libalx/base/compiler/size.h>
#include <libalx/base/stdio/printf/sbprintf.h>
#include <libalx/extra/cv/alx/compare.h>
#include <libalx/extra/cv/alx/median.h>
#include <libalx/extra/cv/core/array/bitwise.h>
#include <libalx/extra/cv/core/img/img.h>
#include <libalx/extra/cv/core/contours/contours.h>
#include <libalx/extra/cv/core/rect/rect.h>
#include <libalx/extra/cv/core/roi/roi.h>
#include <libalx/extra/cv/highgui/file.h>
#include <libalx/extra/cv/imgproc/filter/border.h>
#include <libalx/extra/cv/imgproc/filter/dilate_erode.h>
#include <libalx/extra/cv/imgproc/miscellaneous/fill.h>
#include <libalx/extra/cv/imgproc/miscellaneous/threshold.h>
#include <libalx/extra/cv/imgproc/shape/contours.h>
#include <libalx/extra/cv/imgproc/shape/rect.h>
#include <libalx/extra/cv/types.h>

#include "dbg.h"
#include "symbols.h"


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* enum / struct / union ************************************************
 ******************************************************************************/


/******************************************************************************
 ******* variables ************************************************************
 ******************************************************************************/
const char *const	t_base_meaning[] = {
	"bleach",
	"professional clean",
	"iron",
	"wash",
	"dry",

	"bleach not",
	"professional clean not",
	"iron not",
	"wash not",
	"dry not"
};

const char *const	t_base_fnames[] = {
	"bleach",
	"pro",
	"iron",
	"wash",
	"dry"
};
/*
const char *const	t_inner_meaning[] = {
	"low temp",
	"medium temp",
	"high temp",
	"30",
	"40",
	"50",
	"60",
	"70",
	"95",
	"any solvent",
	"petroleum only",
	"wet clean",
	"any solvent except TCE"
};
*/
const char *const	t_inner_fnames[] = {
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
	"W"
};

img_s	*base_templates[ARRAY_SIZE(t_base_fnames)];
img_s	*base_templates_not[ARRAY_SIZE(t_base_fnames)];
img_s	*inner_templates[ARRAY_SIZE(t_inner_fnames)];


/******************************************************************************
 ******* static prototypes ****************************************************
 ******************************************************************************/
static
int	load_t_base	(img_s *t, const char *fname);
static
int	load_t_inner	(img_s *t, const char *fname);


/******************************************************************************
 ******* global functions *****************************************************
 ******************************************************************************/
int	init_templates	(void)
{
	ptrdiff_t	i;
	ptrdiff_t	j;
	ptrdiff_t	k;

	for (i = 0; i < ARRAY_SSIZE(base_templates); i++) {
		if (alx_cv_init_img(&base_templates[i]))
			goto err0;
	}
	for (j = 0; j < ARRAY_SSIZE(base_templates_not); j++) {
		if (alx_cv_init_img(&base_templates_not[j]))
			goto err1;
	}
	for (k = 0; k < ARRAY_SSIZE(inner_templates); k++) {
		if (alx_cv_init_img(&inner_templates[k]))
			goto err2;
	}

	return	0;

err2:	for (k--; k >= 0; k--)
		alx_cv_deinit_img(inner_templates[k]);
err1:	for (j--; j >= 0; j--)
		alx_cv_deinit_img(base_templates_not[j]);
err0:	for (i--; i >= 0; i--)
		alx_cv_deinit_img(base_templates[i]);
	return	-1;
}

void	deinit_templates(void)
{

	alx_cv_destroy_all_windows();
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(inner_templates); i++)
		alx_cv_deinit_img(inner_templates[i]);
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(base_templates_not); i++)
		alx_cv_deinit_img(base_templates_not[i]);
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(base_templates); i++)
		alx_cv_deinit_img(base_templates[i]);
}

int	load_templates	(void)
{
	char	fname[FILENAME_MAX];

	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(base_templates); i++) {
		if (sbprintf(fname, NULL, "%s/%s.%s", T_BASE_DIR,
					t_base_fnames[i], TEMPLATES_EXT))
			return	i + 110;
		if (load_t_base(base_templates[i], fname))
			return	i + 120;
	}
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(base_templates); i++) {
		if (sbprintf(fname, NULL, "%s/%s_not.%s", T_BASE_DIR,
					t_base_fnames[i], TEMPLATES_EXT))
			return	i + 210;
		if (load_t_base(base_templates_not[i], fname))
			return	i + 220;
	}
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(inner_templates); i++) {
		if (sbprintf(fname, NULL, "%s/%s.%s", T_INNER_DIR,
					t_inner_fnames[i], TEMPLATES_EXT))
			return	i + 310;
		if (load_t_inner(inner_templates[i], fname))
			return	i + 320;
	}
	return	0;
}


/******************************************************************************
 ******* static function definitions ******************************************
 ******************************************************************************/
static
int	load_t_base	(img_s *t, const char *fname)
{
	conts_s		*conts;
	const cont_s	*cont;
	rect_s		*rect;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_conts(&conts))
		return	status;
	if (alx_cv_init_rect(&rect))
		goto err0;

	status--;
	if (alx_cv_imread_gray(t, fname))
		goto err;					dbg_show(3, t);
	alx_cv_threshold(t, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(4, t);
	alx_cv_holes_fill(t);					dbg_show(4, t);
	alx_cv_contours(t, conts);				dbg_show(4, t);
	status--;
	if (alx_cv_conts_largest(&cont, NULL, conts))
		goto err;
	alx_cv_bounding_rect(rect, cont);
	alx_cv_roi_set(t, rect);				dbg_show(3, t);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err0:	alx_cv_deinit_conts(conts);
	return	status;
}

static
int	load_t_inner	(img_s *t, const char *fname)
{
	img_s		*tmp;
	conts_s		*conts;
	const cont_s	*cont;
	rect_s		*rect;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&tmp))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;
	if (alx_cv_init_rect(&rect))
		goto err1;

	status--;
	if (alx_cv_imread_gray(t, fname))
		goto err;					dbg_show(3, t);
	alx_cv_threshold(t, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
								dbg_show(4, t);
	alx_cv_border_black(t, 5);				dbg_show(4, t);
	alx_cv_clone(tmp, t);					dbg_show(4, tmp);
	alx_cv_dilate_erode(tmp, 5);				dbg_show(4, tmp);
	alx_cv_holes_fill(t);					dbg_show(4, tmp);
	alx_cv_contours(tmp, conts);				dbg_show(4, t);
	if (alx_cv_conts_largest(&cont, NULL, conts))
		goto err;
	alx_cv_bounding_rect(rect, cont);
	alx_cv_roi_set(t, rect);				dbg_show(3, t);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err1:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(tmp);
	return	status;
}

int	match_t_base	(img_s *restrict sym, int *code)
{
	img_s		*base;
	conts_s		*conts;
	double		match, m;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&base))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;

	/* Find base */
	status--;
	if (symbol_base(sym, base))
		goto err;					dbg_show(2, base);
	status--;
	match	= -INFINITY;
	*code	= -1;
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(base_templates); i++) {
		m	= alx_cv_compare_bitwise(base, base_templates[i], 2);
		printf("match: %lf\n", m);
		if (m >= match) {
			*code	= i;
			match	= m;				dbg_show(2, base_templates[i]);
		}
		m	= alx_cv_compare_bitwise(base, base_templates_not[i], 2);
		printf("match: %lf\n", m);
		if (m >= match) {
			*code	= i + T_BASE_NOT;
			match	= m;				dbg_show(2, base_templates_not[i]);
		}
	}

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(base);
	return	status;
}

int	match_t_inner	(img_s *restrict sym, int base_code, int *in_code)
{
	img_s		*in;
	conts_s		*conts;
	double		match, m;
	int		status;

	if (base_code >= T_BASE_NOT) {
		*in_code	= -1;
		return	1;
	}

	/* init */
	status	= -1;
	if (alx_cv_init_img(&in))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;

	/* Find inner */
	status--;
	if (symbol_inner(sym, in))
		goto err;					dbg_show(2, in);
	status--;
	match	= -INFINITY;
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(inner_templates); i++) {
		m	= alx_cv_compare_bitwise(in, inner_templates[i], 0);
		printf("match: %lf\n", m);
		if (m >= match) {
			*in_code	= i;
			match		= m;			dbg_show(2, inner_templates[i]);
		}
	}

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(in);
	return	status;
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
