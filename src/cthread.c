/*

Universidade Federal do Rio Grande do Sul
Instituto de Inform�tica
INF01142 - Sistemas Operacionais I N - 2018/1 - Turma B
Prof. Dr. S�rgio Luis Cechin
Trabalho Pr�tico I

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

int first_time_running = 1; // indica se � a primeira vez criando uma thread. Se sim, � preciso inicializar vari�veis de fila e escalonador
int next_thread_id = 1; // indica qual o thread_id da pr�xima thread a ser criada (valor � incrementado a cada thread criada com sucesso)
int thread_priority = 0; // deve ser sempre 0, como especificado na defini��o do trabalho

ucontext_t escalonador;
FILA2 fila_exec;
FILA2 fila_apto;
FILA2 fila_bloq;
FILA2 fila_apto_susp;
FILA2 fila_bloq_susp;

// **************************************************************************
// Outras fun��es
void init_scheduler()
{
    int criaFilaExec = CreateFila2(&fila_exec);
    int criaFilaApto = CreateFila2(&fila_apto);
    int criaFilaBloq = CreateFila2(&fila_bloq);
    int criaFilaAptoSusp = CreateFila2(&fila_apto_susp);
    int criaFilaBloqSusp = CreateFila2(&fila_bloq_susp);

    if(criaFilaExec == 0 && criaFilaApto == 0 && criaFilaBloq == 0 && criaFilaAptoSusp == 0 && criaFilaBloqSusp == 0) { // se cria��o de filas foi bem sucedida

        // inicializa escalonador
        //
        //
        //
        //
        //

        // cria a thread main
        TCB_t *thread_main = malloc(sizeof(TCB_t));
        thread_main->tid = 0; // como a thread main � sempre a primeira a ser criada, seu thread_id ser� sempre 0
        thread_main->state = PROCST_EXEC; // come�a executando, demais threads come�am na fila de aptos
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
// Func�es obrigat�rias (do tipo csomething)
int cidentify (char *name, int size)
{
    char names[] = "\nCassio Ramos - 193028\nGiovani Tirello - 252741\nMaicon Vieira - 242275\n";

    if(sizeof(names) > size) { // Se o tamanho da v�riavel 'names' for maior do que o informado em 'size'
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
    new_thread->state = PROCST_CRIACAO; // thread come�a no estado de cria��o
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
        else { // se n�o inseriu corretamente na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir a nova thread %d na fila de aptos.", new_thread->tid);
            free(new_thread);
            return -1;
        }
    }
    else { // se n�o inicializou corretamente o contexto da nova thread, mostra erro
        printf("\nERRO: Falha ao criar contexto para a nova thread %d.", new_thread->tid);
        free(new_thread);
        return -1;
    }
}

int cyield(void)
{
    if(FirstFila2(&fila_exec) == 0) { // se alguma thread est� executando
        TCB_t *yield_thread = GetAtIteratorFila2(&fila_exec); // guarda essa thread
        if(AppendFila2(&fila_apto, (void *) yield_thread) == 0) { // se inseriu essa thread na fila de aptos
            yield_thread->state = PROCST_APTO; // muda seu estado para apto
            DeleteAtIteratorFila2(&fila_exec); // remove-a da fila de execu��o
            // swapcontext(&thread->context, &escalonador);
            return 0;
        }
        else { // se n�o foi poss�vel inserir na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir thread na fila de aptos.");
            return -1;
        }
    }
    else { // se nenhuma thread em execu��o, mostra erro
        printf("\nERRO: Nenhuma thread em execucao para ceder processamento.");
        return -1;
    }
}
/*
int cjoin(int tid)
{

}*/

int csuspend(int tid)
{
    if(FirstFila2(&fila_exec) != 0) { // se fila de executando vazia, mostrar erro
        printf("\nERRO: Fila de execu��o vazia. Nao foi possivel verificar se thread tentou se auto-suspender.");
        return -1
    }
    else { // fila de execu��o n�o vazia
        TCB_t *thread_exec = GetAtIteratorFila2(&fila_exec); // salva thread executando
        if(thread_exec->tid == tid) { // se thread executando pediu para se auto-suspender
            printf("\nERRO: Nao eh possivel uma thread se auto-suspender.");
            return -1;
        }
        else { // se n�o foi tentativa de se auto-suspender
            // procurar na fila de aptos
            if(FirstFila2(&fila_apto) == 0) { // se fila de aptos n�o estiver vazia
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_apto); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como par�metro, encontrou a thread que deve ser suspensa
                        thread->state = PROCST_APTO_SUS; // muda status para apto_suspenso
                        AppendFila2(&fila_apto_susp, (void *) thread); // insere thread na fila de apto_suspenso
                        DeleteAtIteratorFila2(&fila_apto); // remove thread da fila de apto
                        return 0;
                    }

                } while (NextFila2(&fila_apto) == 0); // procura na fila de apto enquanto pr�ximo elemento n�o for final da fila
            }

            // se n�o encontrou thread na fila de aptos, procura na fila de bloqueados
            if(FirstFila2(&fila_bloq) == 0) { // se fila de bloqueados n�o estiver vazia
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_bloq); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como par�metro
                        thread->state = PROCST_BLOQ_SUS; // muda status para bloqueado_suspenso
                        AppendFila2(&fila_bloq_susp, (void *) thread); // insere thread na fila de bloqueado_suspenso
                        DeleteAtIteratorFila2(&fila_bloq); // remove thread da fila de bloqueado
                        return 0;
                    }
                } while (NextFila2(&fila_bloq) == 0); // procura na fila de bloqueado enquanto pr�ximo elemento n�o for final da fila
            }

            //se tamb�m n�o encontrou na fila de bloqueado, ou thread n�o existe ou n�o estava em apto ou bloqueado
            printf("\nERRO: Thread ID inv�lido ou thread n�o se passivel de suspensao.");
            return -1;
        }
    }
}

int cresume(int tid)
{
    if(FirstFila2(&fila_exec) != 0) { // se fila de executando vazia, mostrar erro
        printf("\nERRO: Fila de execu��o vazia. Nao foi possivel verificar se thread tentou se auto-resumir.");
        return -1
    }
    else { // fila de execu��o n�o vazia
        TCB_t *thread_exec = GetAtIteratorFila2(&fila_exec); // salva thread executando
        if(thread_exec->tid == tid) { // se thread executando pediu para se auto-resumir
            printf("\nERRO: Nao eh possivel uma thread se auto-resumir.");
            return -1;
        }
        else { // se n�o foi tentativa de se auto-resumir
            // procurar na fila de aptos_suspensos
            if(FirstFila2(&fila_apto_susp) == 0) { // se fila de aptos_suspensos n�o estiver vazia
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_apto_susp); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como par�metro, encontrou a thread que deve ser resumida
                        thread->state = PROCST_APTO; // muda status para apto
                        AppendFila2(&fila_apto, (void *) thread); // insere thread na fila de apto
                        DeleteAtIteratorFila2(&fila_apto_susp); // remove thread da fila de apto_suspenso
                        return 0;
                    }

                } while (NextFila2(&fila_apto_susp) == 0); // procura na fila de apto_suspenso enquanto pr�ximo elemento n�o for final da fila
            }

            // se n�o encontrou thread na fila de aptos_suspensos, procura na fila de bloqueados_suspensos
            if(FirstFila2(&fila_bloq_susp) == 0) { // se fila de bloqueados_suspensos n�o estiver vazia
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_bloq_susp); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como par�metro, encontrou thread que deve ser resumida
                        thread->state = PROCST_BLOQ; // muda status para bloqueado
                        AppendFila2(&fila_bloq, (void *) thread); // insere thread na fila de bloqueado
                        DeleteAtIteratorFila2(&fila_bloq_susp); // remove thread da fila de bloqueado_suspenso
                        return 0;
                    }
                } while (NextFila2(&fila_bloq_susp) == 0); // procura na fila de bloqueado_suspenso enquanto pr�ximo elemento n�o for final da fila
            }

            //se tamb�m n�o encontrou na fila de bloqueado_suspenso, ou thread n�o existe ou n�o estava suspensa
            printf("\nERRO: Thread ID inv�lido ou thread n�o se encontra suspensa.");
            return -1;
        }
    }
}

int csem_init(csem_t *sem, int count)
{
    /*if(first_time_running) {
        init_scheduler();
    }*/ // talvez necess�rio, precisa testar

    sem->count = count; // quantidade de recursos controlados pelo semaforo
    sem->fila = malloc(sizeof(FILA2));

    if(CreateFila2(sem->fila) == 0) { // se fila criada com sucesso, termina com sucesso
        return 0;
    }
    else { // se n�o, retorna mensagem de erro
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
