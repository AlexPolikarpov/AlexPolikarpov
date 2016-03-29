#include "stdio.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "dirent.h"
#include "sys/wait.h"


void WorkWithDirectory(char *, char *);  //Функция работы с каталогом
void FileCopy(char *, char *);  //Функция копирования файла из каталога "А" в "Б"

int main(int argc, char* argv[])
{
	pid_t PID;
	int m = atoi(argv[1]);	//Преобразование введённого m в формат int

	if (argc != 4)		//Проверка правильности ввода
		error("incorrect input");    
	if(mkdir(argv[2], 0777) != -1)  //Проверка сущ-ия исходного каталога
	{
		printf("this directory does not exist, or the path is incorrect\n");
		rmdir(argv[2]);
		exit(-1);
	}
	else
	{		
		for(int i = 0; i < m; i=i+1)  //Создание m потоков
		{
			PID = fork();
			if(PID == 0)
				WorkWithDirectory(argv[2], argv[3]); 
		}
		for(int i = 0; i < m; i=i+1)  //Нужна для корректного вывода в консоли (чтобы процесс-родитель не завершился до конца работы процессов-потомков)
			wait(0);
		return 0;
	}
}

void WorkWithDirectory(char *dirFrom, char *dirTo)
{
	int count = 0;  //Счётчик копированных файлов
	
	DIR *dirFromCat = opendir(dirFrom);  //Открытие каталога, откуда копировать
	char dirFrom_name[256];		//Имя каталога, откуда копировать
	char dirFrom_path[256];		//Путь до файла, который копируем
	char dirTo_name[256];		//Имя каталога куда копируем
	char dirTo_path[256];		//Путь до файла, который копируем

	strcpy(dirFrom_name, dirFrom);
	strcpy(dirTo_name, dirTo);
	strcat(dirFrom_name, "/");	//Нужно для корректной работы, при создании каталога
	strcat(dirTo_name, "/");

	if(!opendir(dirTo))		//Если не существует катлога "куда" - создаём его
		mkdir(dirTo_name, 0777);
	DIR *dirToCat = opendir(dirTo);  //Открытие каталога куда копировать

	struct dirent *tmp;		//Получаем указатель на структуру директории
	while((tmp = readdir(dirFromCat))!= NULL)//Пока не закончатся файлы
	{
		if ((strcmp(tmp->d_name, "..") != 0) && (strcmp(tmp->d_name, ".") != 0))	//Служит для предотвращения зацикливания
		{
			strcpy(dirFrom_path,dirFrom_name); //Создаём корректный путь до файла который копируем
			strcat(dirFrom_path,tmp->d_name);
			strcpy(dirTo_path,dirTo_name); //создаём корректный путь до файла, который будет после копирования
			strcat(dirTo_path,tmp->d_name);
			if(access(dirTo_path, 0) == -1)	{  //Проверка, существует ли в папке назначения данный файл, если нет, то создаём его
				if(tmp->d_type != DT_DIR) //Проверка на папки внутри каталога
				{ //если не папка - то копируем
					FileCopy(dirFrom_path, dirTo_path);
					count = count + 1;
				}
				else  //Если папка - то вся функция заного, но уже с учётом этой папки
					WorkWithDirectory(dirFrom_path, dirTo_path);
			
			}
		}
	}
	printf("[Process PID: %d] copied: %i \n", getpid(), count);
	kill(getpid(), SIGHUP);  //Убиваем процесс
}




void FileCopy(char *dirFrom, char *dirTo)
{
	int f1, f2;
	int n;
	char buf[BUFSIZ];

	if ((f1 = open(dirFrom, O_RDONLY, 0)) == -1) //Открытие файла
		error("cannot open file %s", dirFrom);
	if ((f2 = creat(dirTo, 0666)) == -1) //Создание файла
		error("cannot make file %s, mode %03o", dirTo, 0666);
	while ((n = read(f1, buf, BUFSIZ)) > 0) //Копирование файла
		if (write(f2, buf, n) != n)
			error("error writing to file %s", dirTo);
}
