/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "templates/base.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ALX_NO_PREFIX
#include <libalx/base/compiler/size.h>
#include <libalx/base/stdint/mask/bit.h>
#include <libalx/base/stdint/mask/field.h>
#include <libalx/base/stdio/printf/sbprintf.h>
#include <libalx/extra/cv/alx/compare.h>
#include <libalx/extra/cv/alx/median.h>
#include <libalx/extra/cv/core/array/bitwise.h>
#include <libalx/extra/cv/core/img/img.h>
#include <libalx/extra/cv/core/contours/extract.h>
#include <libalx/extra/cv/core/contours/init.h>
#include <libalx/extra/cv/core/rect/rect.h>
#include <libalx/extra/cv/core/roi/roi.h>
#include <libalx/extra/cv/highgui/file.h>
#include <libalx/extra/cv/imgproc/filter/border.h>
#include <libalx/extra/cv/imgproc/filter/dilate_erode.h>
#include "libalx/extra/cv/imgproc/geometric/resize.h"
#include <libalx/extra/cv/imgproc/miscellaneous/fill.h>
#include <libalx/extra/cv/imgproc/miscellaneous/threshold.h>
#include <libalx/extra/cv/imgproc/shape/contour/contours.h>
#include <libalx/extra/cv/imgproc/shape/rect.h>
#include <libalx/extra/cv/types.h>

#include "dbg.h"
#include "symbols.h"
#include "templates/templates.h"


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* enum / struct / union ************************************************
 ******************************************************************************/


/******************************************************************************
 ******* variables ************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* static prototypes ****************************************************
 ******************************************************************************/


/******************************************************************************
 ******* global functions *****************************************************
 ******************************************************************************/
int	match_t_base	(img_s *restrict sym, uint32_t *code, ptrdiff_t i)
{
	img_s		*base;
	img_s		*tmp;
	conts_s		*conts;
	double		match, m;
	int		status;

	/* init */
	status	= -1;
	if (alx_cv_init_img(&base))
		return	status;
	if (alx_cv_init_img(&tmp))
		goto err0;
	if (alx_cv_init_conts(&conts))
		goto err1;

	/* Find base match */
	status--;
	if (symbol_base(sym, base))
		goto err;					dbg_show(2, base);
	status--;
	match	= -INFINITY;
	BITFIELD_SET(code, CODE_BASE_POS, CODE_BASE_LEN);

	alx_cv_clone(tmp, base);
	alx_cv_resize_2largest(tmp, base_templates[i]);
	alx_cv_xor_2ref(tmp, base_templates[i]);		dbg_show(2, tmp);
	m = alx_cv_compare_bitwise(base, base_templates[i], 2);	dbg_printf(1, "match: %lf\n", m);
	if (m >= match) {
								dbg_printf(1, "	%s\n", t_base_meaning[i]);
		BITFIELD_WRITE(code, CODE_BASE_POS, CODE_BASE_LEN, i);
		BIT_SET(code, CODE_Y_N_POS);
		match	= m;					dbg_show(2, base_templates[i]);
	}

	alx_cv_clone(tmp, base);
	alx_cv_resize_2largest(tmp, base_templates_not[i]);
	alx_cv_xor_2ref(tmp, base_templates_not[i]);		dbg_show(2, tmp);
	m = alx_cv_compare_bitwise(base, base_templates_not[i], 2);	dbg_printf(2, "match: %lf\n", m);
	if (m >= match) {
								dbg_printf(1, "	%s not\n", t_base_meaning[i]);
		BITFIELD_WRITE(code, CODE_BASE_POS, CODE_BASE_LEN, i);
		BIT_CLEAR(code, CODE_Y_N_POS);
		match	= m;					dbg_show(2, base_templates_not[i]);
	}
								dbg_printf(2, "\n");

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err1:	alx_cv_deinit_img(tmp);
err0:	alx_cv_deinit_img(base);
	return	status;
}

int	load_t_base		(img_s *t, const char *fname)
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
//	alx_cv_holes_fill(t);					dbg_show(4, t);
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


/******************************************************************************
 ******* static function definitions ******************************************
 ******************************************************************************/


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
