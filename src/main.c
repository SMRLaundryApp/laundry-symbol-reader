/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#define ALX_NO_PREFIX
#include <libalx/base/compiler/size.h>
#include <libalx/base/stdio/printf/b.h>
#include <libalx/base/stdlib/getenv/getenv_s.h>
#include <libalx/extra/cv/core/img/img.h>
#include <libalx/extra/cv/highgui/file.h>
#include <libalx/extra/cv/highgui/window.h>
#include <libalx/extra/cv/types.h>

#include "dbg.h"
#include "label.h"
#include "symbols.h"
#include "templates.h"


/******************************************************************************
 ******* macro ****************************************************************
 ******************************************************************************/
#define ENV_IMG_FNAME		"IMG_FNAME"


/******************************************************************************
 ******* enum / struct / union ************************************************
 ******************************************************************************/


/******************************************************************************
 ******* static functions (prototypes) ****************************************
 ******************************************************************************/
static
int	init	(char fname[static restrict FILENAME_MAX],
		 img_s **restrict img);
static
void	deinit	(img_s *restrict img);


/******************************************************************************
 ******* main *****************************************************************
 ******************************************************************************/
int	main	(void)
{
	char		fname[FILENAME_MAX];
	img_s		*img;
	clock_t		time_0;
	clock_t		time_1;
	double		time_tot;
	uint32_t	code;
	int		status;

	status	= 1;
	if (init(fname, &img))
		return	1;
	time_0 = clock();

	status++;
	if (load_templates())
		goto err;
	status++;
	if (alx_cv_imread(img, fname))
		goto err;
	status++;
	if (find_label(img))
		goto err;
	status++;
	if (find_symbols_vertically(img))
		goto err;
	status++;
	if (find_symbols_horizontally(img))
		goto err;
	status++;
	if (align_symbols(img))
		goto err;
	status++;
	if (extract_symbols(img))
		goto err;
	status++;
	for (ptrdiff_t i = 0; i < nsyms; i++) {
		code	= 0;
		if (clean_symbol(symbols[i]))
			goto err;
		if (match_t_base(symbols[i], &code, i))
			goto err;
		if (match_t_inner(symbols[i], &code) < 0)
			goto err;
		if (match_t_outer(symbols[i], &code) < 0)
			goto err;
		printf("0b%'.16b\n", code);
	}

	time_1 = clock();
	time_tot = ((double) time_1 - time_0) / CLOCKS_PER_SEC;
	dbg_printf(1, "Total time:	%5.3lf s;\n", time_tot);

	alx_cv_imwrite(img, "/tmp/wash.png");

	status	= 0;
err:	deinit(img);
	return	status;
}


/******************************************************************************
 ******* static functions (definitions) ***************************************
 ******************************************************************************/
static
int	init	(char fname[static restrict FILENAME_MAX],
		 img_s **restrict img)
{

	if (getenv_s(fname, FILENAME_MAX, ENV_IMG_FNAME))
		return	-1;
	if (alx_cv_init_img(img))
		return	-2;
	if (init_symbols())
		goto err0;
	if (init_templates())
		goto err1;
	if (DBG)
		alx_cv_named_window("dbg", ALX_CV_WINDOW_NORMAL);
	printf_b_init();

	return	0;

err1:	deinit_symbols();
err0:	alx_cv_deinit_img(*img);
	return	-2;
}

static
void	deinit	(img_s *restrict img)
{

	if (DBG)
		alx_cv_destroy_all_windows();
	deinit_templates();
	deinit_symbols();
	alx_cv_deinit_img(img);
}


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
