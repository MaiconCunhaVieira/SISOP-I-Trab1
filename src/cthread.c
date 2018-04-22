/*

Universidade Federal do Rio Grande do Sul
Instituto de Informática
INF01142 - Sistemas Operacionais I N - 2018/1 - Turma B
Prof. Dr. Sérgio Luis Cechin
Trabalho Prático I

Integrantes:
Cassio Ramos - 193028
Giovani Tirello - 252741
Maicon Vieira - 242275

*/

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int first_time_running = 1; // indica se é a primeira vez criando uma thread. Se sim, é preciso inicializar variáveis de fila e escalonador
int next_thread_id = 1; // indica qual o thread_id da próxima thread a ser criada (valor é incrementado a cada thread criada com sucesso)
int thread_priority = 0; // deve ser sempre 0, como especificado na definição do trabalho

ucontext_t escalonador;
FILA2 fila_exec;
FILA2 fila_apto;
FILA2 fila_bloq;
FILA2 fila_apto_susp;
FILA2 fila_bloq_susp;

// **************************************************************************
// Outras funções
void init_scheduler()
{
    int criaFilaExec = CreateFila2(&fila_exec);
    int criaFilaApto = CreateFila2(&fila_apto);
    int criaFilaBloq = CreateFila2(&fila_bloq);
    int criaFilaAptoSusp = CreateFila2(&fila_apto_susp);
    int criaFilaBloqSusp = CreateFila2(&fila_bloq_susp);

    if(criaFilaExec == 0 && criaFilaApto == 0 && criaFilaBloq == 0 && criaFilaAptoSusp == 0 && criaFilaBloqSusp == 0) { // se criação de filas foi bem sucedida

        // inicializa escalonador
        //
        //
        //
        //
        //

        // cria a thread main
        TCB_t *thread_main = malloc(sizeof(TCB_t));
        thread_main->tid = 0; // como a thread main é sempre a primeira a ser criada, seu thread_id será sempre 0
        thread_main->state = PROCST_EXEC; // começa executando, demais threads começam na fila de aptos
        thread_main->prio = thread_priority;

        getcontext(&thread_main->context);
        //
        if(AppendFila2(&fila_exec, (void *) thread_main) != 0) {
            printf("\nERRO: Falha ao criar a thread main.");
        }

        first_time_running = 0;
    }
    else {
        printf("\nERRO: Falha ao criar as filas de apto, executando e bloqueado.");
    }
}

// **************************************************************************
// Funcões obrigatórias (do tipo csomething)
int cidentify (char *name, int size)
{
    char names[] = "\nCassio Ramos - 193028\nGiovani Tirello - 252741\nMaicon Vieira - 242275\n";

    if(sizeof(names) > size) { // Se o tamanho da váriavel 'names' for maior do que o informado em 'size'
        return -1;
    }
    else {
        strcpy(name, names);
        return 0;
    }
}

int ccreate (void *(*start)(void *), void *arg, int prio)
{
    if(first_time_running) { // se primeira vez criando uma thread, inicializa escalonador e filas
        init_scheduler();
    }

    //cria a nova thread
    TCB_t *new_thread = malloc(sizeof(TCB_t));
    new_thread->tid = next_thread_id;
    new_thread->state = PROCST_CRIACAO; // thread começa no estado de criação
    new_thread->prio = thread_priority; // sempre 0

    // inicializa contexto da nova thread
    if(getcontext(&new_thread->context) == 0) { // se inicializou corretamente

        // inicializa campos do contexto
        //
        //
        //
        //

        // insere thread na fila de aptos
        if(AppendFila2(&fila_apto, (void *) new_thread) == 0) { // se inseriu corretamente
            new_thread->state = PROCST_APTO; // muda para estado de apto
            next_thread_id++; // incrementa indicador de ID das threads a serem criadas
            return new_thread->tid; // retorno  com sucesso
        }
        else { // se não inseriu corretamente na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir a nova thread %d na fila de aptos.", new_thread->tid);
            free(new_thread);
            return -1;
        }
    }
    else { // se não inicializou corretamente o contexto da nova thread, mostra erro
        printf("\nERRO: Falha ao criar contexto para a nova thread %d.", new_thread->tid);
        free(new_thread);
        return -1;
    }
}

int cyield(void)
{
    if(FirstFila2(&fila_exec) == 0) { // se alguma thread está executando
        TCB_t *yield_thread = GetAtIteratorFila2(&fila_exec); // guarda essa thread
        if(AppendFila2(&fila_apto, (void *) yield_thread) == 0) { // se inseriu essa thread na fila de aptos
            yield_thread->state = PROCST_APTO; // muda seu estado para apto
            DeleteAtIteratorFila2(&fila_exec); // remove-a da fila de execução
            // swapcontext(&thread->context, &escalonador);
            return 0;
        }
        else { // se não foi possível inserir na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir thread na fila de aptos.");
            return -1;
        }
    }
    else { // se nenhuma thread em execução, mostra erro
        printf("\nERRO: Nenhuma thread em execucao para ceder processamento.");
        return -1;
    }
}
/*
int cjoin(int tid)
{

}*/
/*
int csuspend(int tid)
{

}*/
/*
int cresume(int tid)
{

}*/

int csem_init(csem_t *sem, int count)
{
    /*if(first_time_running) {
        init_scheduler();
    }*/ // talvez necessário, precisa testar

    sem->count = count; // quantidade de recursos controlados pelo semaforo
    sem->fila = malloc(sizeof(FILA2));

    if(CreateFila2(sem->fila) == 0) { // se fila criada com sucesso, termina com sucesso
        return 0;
    }
    else { // se não, retorna mensagem de erro
        printf("\nERRO: Falha ao inicializar semaforo.");
        return -1;
    }
}
/*
int cwait(csem_t *sem)
{

}*/
/*
int csignal(csem_t *sem)
{

}*/
