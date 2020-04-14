/******************************************************************************
 *	Copyright (C) 2020	Alejandro Colomar Andrés		      *
 *	SPDX-License-Identifier:	GPL-2.0-only			      *
 ******************************************************************************/


/******************************************************************************
 ******* include guard ********************************************************
 ******************************************************************************/
#pragma once	/* templates/templates.h */


/******************************************************************************
 ******* headers **************************************************************
 ******************************************************************************/
#include <stdint.h>

#include <libalx/base/compiler.h>
#include <libalx/extra/cv/cv.h>


/******************************************************************************
 ******* macros ***************************************************************
 ******************************************************************************/
#define TEMPLATES_DIR	"/usr/local/share/laundry-symbol-reader/templates/"
#define T_BASE_DIR	TEMPLATES_DIR "base/"
#define T_INNER_DIR	TEMPLATES_DIR "inner/"
#define TEMPLATES_EXT	"png"

#define CODE_BASE_POS	(1)
#define CODE_BASE_LEN	(3)
#define CODE_Y_N_POS	(CODE_BASE_POS + CODE_BASE_LEN)
#define CODE_IN_POS	(CODE_Y_N_POS + 1)
#define CODE_IN_LEN	(6)
#define CODE_OUT_POS	(CODE_IN_POS + CODE_IN_LEN)
#define CODE_OUT_LEN	(5)


/******************************************************************************
 ******* enum *****************************************************************
 ******************************************************************************/
enum	T_Base_Fname {
	T_BASE_WASH,
	T_BASE_BLEACH,
	T_BASE_DRY,
	T_BASE_IRON,
	T_BASE_PRO,

	T_BASE_QTY
};

enum	T_Inner_Meaning {
	T_INNER_EMPTY,
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

enum	T_Outer_Meaning {
	T_OUTER_EMPTY,
	T_OUTER_DELICATE,
	T_OUTER_VERY_DELICATE,

	T_OUTER_MEANING_QTY
};


/******************************************************************************
 ******* struct / union *******************************************************
 ******************************************************************************/


/******************************************************************************
 ******* variables ************************************************************
 ******************************************************************************/
extern	const char *const	t_base_meaning[T_BASE_QTY];
extern	const char *const	t_base_fnames[T_BASE_QTY];
extern	const char *const	t_inner_meaning[T_INNER_MEANING_QTY];
extern	const char *const	t_inner_fnames[T_INNER_QTY];
extern	const char *const	t_outer_meaning[T_OUTER_MEANING_QTY];

extern	img_s	*base_templates[ARRAY_SIZE(t_base_fnames)];
extern	img_s	*base_templates_not[ARRAY_SIZE(t_base_fnames)];
extern	img_s	*inner_templates[ARRAY_SIZE(t_inner_fnames)];


/******************************************************************************
 ******* prototypes ***********************************************************
 ******************************************************************************/
int	init_templates	(void);
void	deinit_templates(void);
int	load_templates	(void);
int	match_t_inner	(img_s *restrict sym, uint32_t *code);
int	match_t_outer	(img_s *restrict sym, uint32_t *code);
void	print_code	(uint32_t code);


/******************************************************************************
 ******* inline ***************************************************************
 ******************************************************************************/


/******************************************************************************
 ******* end of file **********************************************************
 ******************************************************************************/
