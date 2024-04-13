#include <stdio.h>
#include<string.h>
#include <stdlib.h> 
#include <malloc.h>
#pragma warning(disable : 4996)


typedef enum process_state {
	RUNNING,
	READY,
	TERMINATED
} STATE;
typedef struct process {
	char* name;
	int size;
	int exec_time;
	int time_slice;
	STATE state;
} PROC;
typedef struct list_node {
	PROC* proc;
	struct list_node* next;
} NODE;

PROC* init() {
	int num_procs;
	FILE* fptr = fopen("workload.txt", "r");
	if (!fptr)
		return NULL;

	fscanf(fptr, "%d", &num_procs);

	PROC* proc_arr = (PROC*)malloc(sizeof(PROC) * num_procs);
	for (int i = 0; i < num_procs; i++) {
		proc_arr[i].name = (char*)malloc(sizeof(char) * 256);
		fscanf(fptr, "%s %d\n", proc_arr[i].name, &proc_arr[i].size);
		proc_arr[i].exec_time = 0;
		proc_arr[i].time_slice = 10;
		proc_arr[i].state = READY;
	}

	fclose(fptr);

	return proc_arr;
}


NODE* pushNode(NODE** rqP) {
	NODE* head = *rqP;
	NODE* returnNode = head->next;
	NODE* temp = head;
	while (temp->next) {
		temp = temp->next;
	}
	temp->next = head;
	head->next = NULL;
	return returnNode;
}


NODE* deleteNode(NODE** rqP) {
	NODE* head = *rqP;
	NODE* nextHead = head->next;
	free(head);

	return nextHead;
}

void print(int MS, NODE* rq, char* term) {
	if (!rq) {
		printf("%3d ms: end of execution, terminated: %s", MS, term);
		return;
	}
	if (term) {
		NODE* temp = rq;
		printf("%3d ms: run %s, [", MS,  rq->proc->name);
		if (temp->next) {
			temp = temp->next;
			while (temp) {
				printf("%s", temp->proc->name);
				if (temp->next) {
					printf(", ");
				}
				temp = temp->next;
			}
		}
		printf("], terminated: %s\n", term);

		return;
	}
	NODE* temp = rq;
	printf("%3d ms: run %s, [", MS, rq->proc->name);
	if (temp->next) {
		temp = temp->next;
		while (temp) {
			printf("%s", temp->proc->name);
			if (temp->next) {
				printf(", ");
			}

			temp = temp->next;
		}
	}
	printf("]\n");

	return;
	
}

int sched(PROC** running, NODE** rqP) {
	static int MS;
	char* terminated;
	NODE* rq = *rqP;
	PROC* run = *running;
	if (!rq)
	{
		return 0;
	}

	if (!run) {
		*running = rq->proc;
		print(MS, rq, NULL);
	}

	if ((rq->proc->size - rq->proc->exec_time) == 0) {
		terminated = rq->proc->name;
		*rqP = deleteNode(rqP);
		rq = *rqP;
		if (rq)
		{
			rq->proc->state = RUNNING;
		}

		print(MS, rq, terminated);
		return 1;
	}
	if (rq->proc->time_slice == 0)
	{
		rq->proc->state = READY;
		rq->proc->time_slice = 10;
		if (rq->next) {
			*rqP = pushNode(rqP);
			rq = *rqP;
			rq->proc->state = RUNNING;

			print(MS, rq, NULL);
			return 1;
		}
		rq->proc->state = RUNNING;
		return 1;
	}

	rq->proc->exec_time++;
	rq->proc->time_slice--;
	MS++;

	return 1;
}

void cleanup(PROC* proc_Arr, NODE* rqP) {
	for (int i = _msize(proc_Arr) / sizeof(PROC) - 1; i >= 0; i--) {
		free(proc_Arr[i].name);
	}
	free(proc_Arr);
	NODE* temp = rqP;
	NODE* deleteNode;
	while (temp) {
		deleteNode = temp;
		temp = temp->next;
		free(deleteNode);

	}
	free(temp);

	return;
}

void rqinit(NODE** rqP, PROC* proc_Arr) {
	NODE* head = (NODE*)malloc(sizeof(NODE));
	head->next = NULL;
	NODE* curNode = NULL;
	for (int i = 0; i < _msize(proc_Arr) / sizeof(PROC); i++) {
		NODE* tempNode = (NODE*)malloc(sizeof(NODE));
		if (head->next == NULL) {
			tempNode->proc = &proc_Arr[i];
			tempNode->next = NULL;
			head->next = tempNode;
			curNode = tempNode;
			continue;
		}
		tempNode->proc = &proc_Arr[i];
		tempNode->next = NULL;
		curNode->next = tempNode;
		curNode = tempNode;
	}

	*rqP = head->next;
	free(head);

	return;
}

int main() {
	NODE* rq = NULL;
	PROC* running = NULL;
	//proc_Arr에 프로세스 배열 할당
	PROC* proc_Arr = init();
	//proc_Arr가 비어있다면 프로그램 종료
	if (proc_Arr == NULL) return 0;
	//proc_Arr가 비어있지않다면 running과 rq.proc에 첫번째 프로세스를 넣는다.
	else {
		rqinit(&rq, proc_Arr);
		NODE* temp = rq;
		printf("  init: [");
		while (temp) {
			printf("%s", temp->proc->name);
			if (temp->next) {
				printf(", ");
			}
			temp = temp->next;
		}
		printf("]\n");
	}
	while (sched(&running, &rq));
	cleanup(proc_Arr, rq);
	return 0;
}
