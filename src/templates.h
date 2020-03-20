/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* include guard ********************************************************
 ******************************************************************************/
#pragma once	/* templates.h */


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include <libalx/base/compiler/size.h>
#include <libalx/extra/cv/types.h>


/******************************************************************************
 ******* macros ***************************************************************
 ******************************************************************************/
#define TEMPLATES_DIR	"templates/"
#define T_BASE_DIR	TEMPLATES_DIR "base/"
#define T_INNER_DIR	TEMPLATES_DIR "inner/"
#define TEMPLATES_EXT	"png"


/******************************************************************************
 ******* enum *****************************************************************
 ******************************************************************************/
enum	T_Base_Fname {
	T_BASE_BLEACH,
	T_BASE_PRO,
	T_BASE_IRON,
	T_BASE_WASH,
	T_BASE_DRY,

	T_BASE_QTY,

	T_BASE_NOT	= T_BASE_QTY
};

enum	T_Inner_Meaning {
	T_INNER_LO_T,
	T_INNER_MED_T,
	T_INNER_HI_T,
	T_INNER_30,
	T_INNER_40,
	T_INNER_50,
	T_INNER_60,
	T_INNER_70,
	T_INNER_95,
	T_INNER_A,
	T_INNER_F,
	T_INNER_P,
	T_INNER_W,

	T_INNER_MEANING_QTY
};

enum	T_Inner_Fname {
	T_INNER_FNAME_1_DOT,
	T_INNER_FNAME_2_DOT,
	T_INNER_FNAME_3_DOT,
	T_INNER_FNAME_4_DOT,
	T_INNER_FNAME_5_DOT,
	T_INNER_FNAME_6_DOT,
	T_INNER_FNAME_30,
	T_INNER_FNAME_40,
	T_INNER_FNAME_50,
	T_INNER_FNAME_60,
	T_INNER_FNAME_95,
	T_INNER_FNAME_A,
	T_INNER_FNAME_F,
	T_INNER_FNAME_P,
	T_INNER_FNAME_W,

	T_INNER_QTY
};


/******************************************************************************
 ******* struct / union *******************************************************
 ******************************************************************************/


/******************************************************************************
 ******* variables ************************************************************
 ******************************************************************************//*
extern	const char *const	t_base_meaning[T_BASE_QTY];
*/
extern	const char *const	t_base_fnames[T_BASE_QTY];
/*
extern	const char *const	t_inner_meaning[T_INNER_MEANING_QTY];
*/
extern	const char *const	t_inner_fnames[T_INNER_QTY];

extern	img_s	*base_templates[ARRAY_SIZE(t_base_fnames)];
extern	img_s	*base_templates_not[ARRAY_SIZE(t_base_fnames)];
extern	img_s	*inner_templates[ARRAY_SIZE(t_inner_fnames)];


/******************************************************************************
 ******* prototypes ***********************************************************
 ******************************************************************************/
int	init_templates	(void);
void	deinit_templates(void);
int	load_templates	(void);
int	match_t_base	(img_s *restrict sym, int *code);


/******************************************************************************
 ******* inline ***************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
