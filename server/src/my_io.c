/**
 * Copyright(c) 2012, Free software organization.
 * All Rights Reserved.
 * 
 * @file  my_io.h
 * @brief 自己定义的输入函数。
 */

#include <termios.h> /* for tcxxxattr, ECHO, etc */
#include <unistd.h> /* for STDIN_FILENO */
#include<ctype.h>
#include<stdio.h>

/**
 * @brief 模拟window下的getch()。
 */
int getch (void){
	int ch;
	struct termios oldt, newt;

	// get terminal input's attribute
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;

	//set termios' local mode
	newt.c_lflag &= ~(ECHO|ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	//read character from terminal input
	ch = getchar();

	//recover terminal's attribute
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}


/**
 * @brief 输入数字。
 * 
 * @param num 要读入数字的数组。
 * @param ch 作为每个字符的中转。
 * @param len 返回的输入的数字长度。
 * @param num_max_size 能够输入的数字的最大长度。
 */
int in_num(char num[],int num_max_size){
	int ch,len = 0;
	while((ch = getch()) != '\n'){       //过滤掉非数字
		if(ch == 127 && len > 0){
			printf("\b \b");               //输入VBKeyBack时，实现删除效果
			len--;
		}
		else if(isalnum(ch)){
			if(isalpha(ch) || len >= num_max_size){		       //是英文字母时给出提示音
				printf("\a");
			}           
			else{                         //是数字时才回显
				num[len++] = ch;
				printf("%c",ch);
			}
		}
	}
	printf("\n");
	num[len] = '\0';
	return len;
}

int get_in(char num[],int num_max_size){
	int ch,len = 0;
	while((ch = getch()) != '\n'){
		if(ch == 127 && len > 0){
			printf("\b \b");
			len--;
		}else if(len >= num_max_size){
			printf("\a");
		}else{
			num[len++] = ch;
			printf("%c",ch);
		}
	}
	printf("\n");
	num[len] = '\0';
	return len;
}
