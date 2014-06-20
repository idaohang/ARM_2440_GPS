#include "menu.h"
#include "socket.h"
#include "config.h"
#include "my_io.h"

#include<stdlib.h>
#include<ctype.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern int maxi;

int input_client_id(char *client_id);


void eatline(void){
	while(getchar() != '\n');
}

MENU_LIST* pop(MENU_LIST *top){
	top = top->next;
	return top;
}

MENU_LIST* push(MENU_LIST *top,MENU_LIST *p){
	p->next = top;
	top = p;
	return top;
}



void show_menu(MENU_LIST *menu_show);

void *showmenu(void *param){
	MENU_LIST *p = NULL,*top = NULL;
	int choice;
	MENU *select_menu = NULL;
	
	while(1){
		if(!top || select_menu){
			if((p = malloc(sizeof(MENU_LIST))) == NULL){
				perror("分配内存失败！");
				exit(1);
			}
			if(!top){					
				strcpy(p->title,"主菜单");
				p->menu = main_menu;
				p->num = 5;
			}
			else{
				strcpy(p->title,select_menu->title);
				p->menu = select_menu->cmenu;
				p->num = select_menu->cmenu_num;
			}
			top = push(top,p);	
		}
		
		select_menu = NULL;
		choice = 0;
		
		show_menu(top);
		printf("请选择菜单：");
		scanf("%d",&choice);
		eatline();
		if(choice < 0 || choice > top->num){
			printf("输入无效,请重新选择：\n");
			continue;
		}
		if(choice == top->num){
			p = top;
			top = pop(top);
			free(p);
			if(!top)
				break;
			else{
				continue;
			}
		}

		select_menu = &top->menu[choice - 1];

		//有处理函数的
		if(select_menu->proc){
			//(*select_menu->proc)(select_menu->param);
			char client_id[CLIENT_ID_MAX];
			int index;
			
			printf("请选择客户(车牌号): ");
get_again:
	
			//fgets(client_id, CLIENT_ID_MAX, stdin);
			while(!get_in(client_id, CLIENT_ID_MAX-1));
			while(index <= maxi){
				if(strncmp(client[index].client_id, client_id, CLIENT_ID_MAX - 1) == 0)  //去掉结束符和换行符
					break;
				index++;
			}

			if(index > maxi){
				printf("客户不存在或是未连接,请重新输入: ");
				goto get_again;
			}
			if(strcmp(select_menu->title,"定位车辆") == 0){
				send_data(client[index].socketfd, COMM_TYPE_SERVE, COMM_SERV_GPS, NULL, 0);
			}
			else if(strcmp(select_menu->title,"监视车辆内部情况") == 0){
				send_data(client[index].socketfd, COMM_TYPE_SERVE, COMM_SERV_BITMAP, NULL, 0);
			}
			else if(strcmp(select_menu->title,"下达锁车指令") == 0){
				send_data(client[index].socketfd, COMM_TYPE_SERVE, COMM_SERV_LOCK, NULL, 0);
			}
			else if(strcmp(select_menu->title,"解锁车辆") == 0){
				send_data(client[index].socketfd, COMM_TYPE_SERVE, COMM_SERV_UNLOCK, NULL, 0);
			}
			select_menu = NULL;
		}else if(!select_menu->cmenu){     //无处理函数并且无子菜单的
			select_menu = NULL;
			printf("输入无效,请重新选择：\n");
		}
	}
	exit(0);
}

void show_menu(MENU_LIST *menu_show){
	int i = 0;
	printf("\n    %s	\n",menu_show->title);
	for(i = 0; i < 15; i++)
		printf("-");
	printf("\n");
	for(i = 0; i < menu_show->num; i++)
		printf("[%d].%s\n",i+1,menu_show->menu[i].title);	
}

int input_client_id(char *client_id){
	int len = 0;
	int ch;
	while((ch = getch()) != '\r'){       //过滤掉非数字
		if(ch == 8 && len > 0){
			printf("\b \b");               //输入VBKeyBack时，实现删除效果
			len--;
		}
		else if(isalnum(ch)){
			if(isalpha(ch) || len >= (CLIENT_ID_MAX-1)){		       //是英文字母时给出提示音
				printf("\a");
			}           
			else{                         //是数字时才回显
				client_id[len++] = ch;
				printf("%c",ch);
			}
		}
	}
	printf("\n");
	if(len != (CLIENT_ID_MAX-1)){                 //输入有误返回0
		if(len == 0)
			puts("错误！车牌号不能为空！请重新输入");
		else
			printf("错误！车牌号须为%d位！请重新输入\n",11);
		return 0;
	}
	client_id[len] = '\0';
	return 1;
}



