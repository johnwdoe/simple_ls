/*
 * main.c
 *
 *  Created on: May 13, 2020
 *      Author: william
 */

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

int showDirContent(char *path);

int main(int argc, char** argv)
{
	int i, res;

	if (argc <= 1) return showDirContent("."); //если нет параметров - выводим текущую директорию

	//иначе проходимся по всем перечисленным
	for (i=1; i<argc; i++){
		printf("%s:\n", argv[i]);
		res = showDirContent(argv[i]);
		if (res) return res; //при ошибке сразу выходим
	}

return 0;
}

/*
 * функция выводит содержимое каталога а-ля ls -l
 */
int showDirContent(char *path)
{
	DIR *dir = opendir(path);
		if(dir){
			struct dirent *ent;
			int dfd = dirfd(dir); //получаем дескриптор (для использования в fstatat())
			while((ent = readdir(dir)) != NULL){

				struct stat st;
				if (fstatat(dfd, ent->d_name, &st, 0) == -1) perror(ent->d_name);
				else {
					//выясняем, какому типу соответствует текущий объект (файл, директория, блочный девайс и т.д.)
					char f_type;
					switch(st.st_mode & S_IFMT){
						case S_IFSOCK: f_type = 's'; break;
						case S_IFLNK: f_type = 'l'; break;
						case S_IFREG: f_type = '-'; break;
						case S_IFBLK: f_type = 'b'; break;
						case S_IFDIR: f_type = 'd'; break;
						case S_IFCHR: f_type = 'c'; break;
						case S_IFIFO: f_type = 'f'; break;
					}

					char permissions[sizeof(char)*9 + 1]; //строка разрешений - всегда 9 символов + завершающий 0x00
					//fill permissions string
					permissions[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
					permissions[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
					permissions[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
					permissions[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
					permissions[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
					permissions[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
					permissions[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
					permissions[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
					permissions[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
					permissions[9] = '\0';

					//выдергиваем владельца объекта и группу
					struct passwd *pwd = getpwuid(st.st_uid);
					struct group *grp = getgrgid(st.st_gid);

					//Формируем строку с временем полседней модификации
					char modTimeStr[20];
					strftime(modTimeStr, 20, "%b %d %H:%M", localtime((time_t*)&st.st_mtim));

					//выводим
					printf("%c%s %ld %s %s %ld %s %s\n", f_type, permissions, (unsigned long)st.st_nlink, pwd->pw_name, grp->gr_name, (unsigned long)st.st_size, modTimeStr, ent->d_name);
				}
			}
		}else{
			perror("Error opening directory\n");
			return 1;
		}
		return 0;
}


