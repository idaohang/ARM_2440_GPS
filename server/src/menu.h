#ifndef __MENU_H__
#define __MENU_H__

#include "types.h"

#define MENU_TITLE_MAX_LEN 20


typedef struct menu_s MENU;
struct menu_s{
	char title[MENU_TITLE_MAX_LEN];     /*菜单标题信息*/
	const MENU *cmenu;									/*子菜单*/
	int cmenu_num;											/*子菜单选项个数*/
	int proc;   												/*是否有处理程序*/
};

typedef struct menu_list MENU_LIST;
struct menu_list{
	char title[MENU_TITLE_MAX_LEN];
	MENU *menu;
	int num;
	MENU_LIST *next;
};

//这边不加上static不行
static const MENU main_menu[] = {
	{"定位车辆", NULL, 0, 1},						/*上传GPS*/
	{"监视车辆内部情况", NULL, 0, 1},		/*上传图片*/
	{"下达锁车指令", NULL, 0, 1},				/*锁车*/
	{"解锁车辆", NULL, 0, 1},						/*解锁*/
	{"退出", NULL, 0, 0}
};

extern void *showmenu(void *param);

#endif	/*__MENU_H__*/
