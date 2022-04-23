#ifndef _CONF_H_
#define _CONF_H_

/*
 *从配置文件中读取相应的值
 *输入参数：1，配置文件路径 2，匹配标记 3，输出存储空间
 *并且排除了空行，“=”前后无内容，无“=”的情况
 */
int conf_read(char *conf_path,char *conf_name,char *cnf_val);

/*
 *加载配置文件内容
 */
int conf_load(char *conf_path);

/*
 *添加修改文件（当配置文件中存在标记字段，则进行修改，若不存在则进行添加）
 *
 *输入参数：1，配置文件路径 2，匹配标记 3，替换或添加的内容
 *
 */
int conf_update(char *conf_path,char *conf_name,char *config_buff);

//int conf_append(char *conf_path,char *conf_name,char *conf_value);

/*
 *删除配置文件内容（
 *
 *输入参数：1，配置文件路径 2，匹配标记 
 *
 */
int conf_delete(char *conf_path,char *conf_name);

#define MX_KC 32
#define LINE_SIZE 512
int kc;

char kv[MX_KC][2][LINE_SIZE];

#endif
