/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "exec_parser.h"
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>


static so_exec_t *exec;
static int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	struct sigaction semnal;
	int gasit = 0;
	//parcurg fiecare segemnt si vad daca adresa la care primesc page fault 
	//se afla in segment
	for(int i = 0; i < exec->segments_no; i++)
	{
		so_seg_t *segm = &exec->segments[i];
		if((int)info->si_addr >= segm->vaddr &&
			(int)info->si_addr <= segm->vaddr + segm->mem_size )
			{
				gasit=1;
				//aloc vectorul data unde voi retine ce pagina din segment a fost mapata
				if(segm->data == NULL)
				{
					int nr_pagini = segm->mem_size / getpagesize();
					segm->data= (void *) calloc(nr_pagini,sizeof(int));
				}
				//calculez distanta de la adresa unde s a produs page fault ul si inceputul paginii
				int distanta = ((int)info->si_addr) % getpagesize();
				//calculez offsetul fisierului de unde trebuiesc citite datele
				int offset = ((int)info->si_addr - distanta - segm->vaddr);
				//daca pagina a fost mapata,rulez handler-ul default
				if(((int*)segm->data)[ offset / getpagesize() ] == 1)
				{
					semnal.sa_sigaction(signum,info,context);
					return;
				}
				else
				{
					//vad care este ultima pagina de unde trebuiesc citite datele din fisier
					int nrpagina = segm->file_size / getpagesize();
                	void *mmap1;
			      	//daca page fault ul s a produs la ultima pagina:
                    //aloc pagina,citesc din fisier de la inceputul paginii unde s a produs page fault ul
                      //pana la file-size
                    //zerorizez restul paginii
					if(offset / getpagesize() == nrpagina)
					{
						int size = segm->file_size - offset;
						mmap1 = mmap((void *)(info->si_addr-distanta), getpagesize() ,PROT_WRITE | PROT_READ,
				 				MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE,-1, 0);
						char buffer[size];
						
						pread(fd, buffer, size,segm->offset + offset);	
						memcpy(mmap1, buffer, size);
					}
					//daca page fault ul nu s a produs la ultima pagina:
                    //aloc pagina si citesc din fisier dimensiunea unei pagini,
							//incepand cu pagina unde s a produs page fault ul
					else if(offset / getpagesize() < nrpagina)
					{
					 	mmap1 =mmap((void *)(info->si_addr-distanta), getpagesize() ,PROT_WRITE | PROT_READ,
				 					MAP_FIXED | MAP_PRIVATE ,fd , segm->offset + offset);
					
					}
					//daca page fault ul este intre mem-size si file-size,aloc memorie zerorizata
					else
					{
				 		mmap1 = mmap((void *)((int)info->si_addr-distanta), getpagesize() ,PROT_WRITE | PROT_READ,
				 					MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
				
					}
				//notez pagina ca a fost mapata si pun permisiunile necesare
				((int*)segm->data)[offset / getpagesize()] = 1;
				mprotect(mmap1 ,getpagesize() , segm->perm);
				return;
				}
			}					
		}
	//daca adresa unde a dat page fault nu se afla intre segmente
	//rulez handler-ul default
	if(gasit == 0)
	{
		semnal.sa_sigaction(signum,info,context);
		return;
	}
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	fd=open(path,O_RDONLY);
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);
	close(fd);

	return -1;
}
