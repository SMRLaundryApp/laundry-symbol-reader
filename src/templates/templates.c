/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include "templates/templates.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define ALX_NO_PREFIX
#include <libalx/base/compiler/size.h>
#include <libalx/base/stdint/mask/bit.h>
#include <libalx/base/stdint/mask/field.h>
#include <libalx/base/stdio/printf/sbprintf.h>
#include <libalx/extra/cv/cv.h>

#include "dbg.h"
#include "symbols.h"
#include "templates/base.h"


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
	"wash",
	"bleach",
	"dry",
	"iron",
	"professional clean"
};

const char *const	t_base_fnames[] = {
	"wash",
	"bleach",
	"dry",
	"iron",
	"pro"
};

const char *const	t_inner_meaning[] = {
	"",
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

const char *const	t_outer_meaning[] = {
	"",
	"delicate",
	"very delicate"
};

img_s	*base_templates[ARRAY_SIZE(t_base_fnames)];
img_s	*base_templates_not[ARRAY_SIZE(t_base_fnames)];
img_s	*inner_templates[ARRAY_SIZE(t_inner_fnames)];


/******************************************************************************
 ******* static prototypes ****************************************************
 ******************************************************************************/
static
int	load_t_inner		(img_s *t, const char *fname);
static
void	t_inner_fix_code	(uint32_t *code);


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

int	match_t_inner	(img_s *restrict sym, uint32_t *code)
{
	img_s		*in;
	conts_s		*conts;
	double		match, m;
	int		status;

	if (!BIT_READ(*code, CODE_Y_N_POS)) {
		BITFIELD_CLEAR(code, CODE_IN_POS, CODE_IN_LEN);
		return	1;
	}

	/* init */
	status	= -1;
	if (alx_cv_init_img(&in))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;

	/* Find inner match */
	status--;
	if (symbol_inner(sym, in))
		goto err;					dbg_show(2, in);
	status--;
	match	= -INFINITY;
	BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, 0);
	for (ptrdiff_t i = 0; i < ARRAY_SSIZE(inner_templates); i++) {
		m	= alx_cv_compare_bitwise(in, inner_templates[i], 2);
								dbg_printf(2, "match: %.4lf\n", m);
		if (m >= match) {
			BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, i);
			match		= m;
								dbg_printf(1, "%s\n", t_inner_fnames[i]);
		}
	}
								dbg_show(1, inner_templates[BITFIELD_READ(*code, CODE_IN_POS, CODE_IN_LEN)]);

	t_inner_fix_code(code);
								dbg_printf(1, "%s\n", t_inner_meaning[BITFIELD_READ(*code, CODE_IN_POS, CODE_IN_LEN)]);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(in);
	return	status;
}

int	match_t_outer	(img_s *restrict sym, uint32_t *code)
{
	img_s		*out;
	conts_s		*conts;
	ptrdiff_t	n_lines;
	int		status;

	if (!BIT_READ(*code, CODE_Y_N_POS)) {
		BITFIELD_CLEAR(code, CODE_OUT_POS, CODE_OUT_LEN);
		return	1;
	}

	/* init */
	status	= -1;
	if (alx_cv_init_img(&out))
		return	status;
	if (alx_cv_init_conts(&conts))
		goto err0;

	/* Find inner match */
	status--;
	if (symbol_outer(sym, out))
		goto err;					dbg_show(2, out);
	status--;

	alx_cv_contours(out, conts);
	alx_cv_extract_conts(conts, NULL, &n_lines);
	BITFIELD_WRITE(code, CODE_OUT_POS, CODE_OUT_LEN, n_lines);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(out);
	return	status;
}


/******************************************************************************
 ******* static function definitions ******************************************
 ******************************************************************************/
static
int	load_t_inner		(img_s *t, const char *fname)
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
		goto err;					dbg_show(4, t);
	alx_cv_threshold(t, ALX_CV_THRESH_BINARY_INV, ALX_CV_THR_OTSU);
	alx_cv_border_black(t, 5);
	alx_cv_clone(tmp, t);
	alx_cv_dilate_erode(tmp, 5);
	alx_cv_contours(tmp, conts);
	if (alx_cv_conts_largest_a(&cont, NULL, conts))
		goto err;
	alx_cv_bounding_rect(rect, cont);
	alx_cv_roi_set(t, rect);				dbg_show(4, t);

	/* deinit */
	status	= 0;
err:	alx_cv_deinit_rect(rect);
err1:	alx_cv_deinit_conts(conts);
err0:	alx_cv_deinit_img(tmp);
	return	status;
}

static
void	t_inner_fix_code	(uint32_t *code)
{
	uint8_t	base_code;
	uint8_t	in_code;

	base_code	= BITFIELD_READ(*code, CODE_BASE_POS, CODE_BASE_LEN);
	in_code		= BITFIELD_READ(*code, CODE_IN_POS, CODE_IN_LEN);
	switch (base_code) {
	case T_BASE_BLEACH:
	default:
		BITFIELD_SET(code, CODE_IN_POS, CODE_IN_LEN);
		break;
	case T_BASE_PRO:
		in_code	+= T_INNER_A - T_INNER_FNAME_A;
		BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
		break;
	case T_BASE_DRY:
	case T_BASE_IRON:
		in_code	+= T_INNER_LO_T - T_INNER_FNAME_1_DOT;
		BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
		break;
	case T_BASE_WASH:
		switch (in_code) {
		case T_INNER_FNAME_1_DOT ... T_INNER_FNAME_6_DOT:
			in_code	+= T_INNER_30 - T_INNER_FNAME_1_DOT;
			BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
			break;
		case T_INNER_FNAME_30 ... T_INNER_FNAME_60:
			in_code	+= T_INNER_30 - T_INNER_FNAME_30;
			BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
			break;
		case T_INNER_FNAME_95:
			in_code	+= T_INNER_95 - T_INNER_FNAME_95;
			BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
			break;
		case T_INNER_FNAME_A ... T_INNER_FNAME_W:
			in_code	+= T_INNER_A - T_INNER_FNAME_A;
			BITFIELD_WRITE(code, CODE_IN_POS, CODE_IN_LEN, in_code);
			break;
		}
		break;
	}
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
