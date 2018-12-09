#include "sem.h"

void handler(int signo) { }

int errCheck(int returnVal, char *message) {
    if (returnVal < 0 ) {
        fprintf(stderr,"%s: %s\n", message, strerror(errno));
        return -1;
    }
}

void sem_init(struct sem *s, int count) {
    s->count = count;
	s->lock = 0;
    memset(s->waiting,0,NUMPROCS);

    // set signal disposistion
    sigfillset(&s->sigmask);
	sigdelset(&s->sigmask,SIGINT);
	sigdelset(&s->sigmask,SIGUSR1);
    errCheck(signal(SIGUSR1,handler) == SIG_ERR, "Failed setting signal handler");
}

int sem_try(struct sem *s) {
    while(tas(&s->lock))
        ;
	if (s->count > 0) {
		s->count--;
		s->lock = 0;
		return 1;
	} else {
		s->lock = 0;
		return 0;
	}
}

void sem_wait(struct sem *s) {
    s->waiting[myProcnum] = 1;
	s->process[myProcnum] = getpid();

	while (1) {
		while(tas(&s->lock))
            ;
		if (s->count > 0 ) {
			s->waiting[myProcnum] = 0;
			s->count--;
			s->lock = 0;
			return;
		} else {
			s->lock = 0;
			sigsuspend(&s->sigmask);
		}
	}
}

void sem_inc(struct sem *s) {
    while(tas(&s->lock))
        ;
    s->count++;
    for (int i = 0; i < NUMPROCS; i++) {
        if (s->waiting[i]) {
            kill(s->process[i],SIGUSR1);
            break;
        }
    }
    s->lock = 0;
}
