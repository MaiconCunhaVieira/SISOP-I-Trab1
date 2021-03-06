/*

Universidade Federal do Rio Grande do Sul
Instituto de Informatica
INF01142 - Sistemas Operacionais I N - 2018/1 - Turma B
Prof. Dr. Sergio Luis Cechin
Trabalho Pratico I

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

int first_time_running = 1; // indica se eh a primeira vez criando uma thread. Se sim, eh preciso inicializar variaveis de fila e escalonador
int next_thread_id = 1; // indica qual o thread_id da proxima thread a ser criada (valor eh incrementado a cada thread criada com sucesso)
int thread_priority = 0; // deve ser sempre 0, como especificado na definicao do trabalho
int thread_exec_not_finished = 0;

ucontext_t escalonador;
FILA2 fila_exec;
FILA2 fila_apto;
FILA2 fila_bloq;
FILA2 fila_apto_susp;
FILA2 fila_bloq_susp;

// **************************************************************************
// Outras funcoes
void dispatcher()
{
    // ou termina execucao de thread e coloca outra thread para executar (se thread atualmente executando terminou execucao)
    // ou apenas coloca outra thread para executar (se thread deu cyield, cjoin ou cwait)

    TCB_t *thread;

    // muda thread do estado executando para termino se thread terminou execucao
    if(!thread_exec_not_finished) { // se thread terminou sua execucao
        if(FirstFila2(&fila_exec) == 0) { // se alguma thread em execucao na fila de execucao, seta iterador para primeiro elemento da fila
            thread = GetAtIteratorFila2(&fila_exec); // salva thread
            thread->state = PROCST_TERMINO; // muda estado para termino
            DeleteAtIteratorFila2(&fila_exec); // remove thread da fila de execucao

            // checar se outras thread em bloqueado e bloqueado suspenso nao esperavam recursos dessa thread terminada
            if(FirstFila2(&fila_bloq) == 0) {
                TCB_t *thread_bloq;
                do {
                    thread_bloq = GetAtIteratorFila2(&fila_bloq);
                    if(thread_bloq->waitingThreadID == thread->tid) { // se thread em fila de bloqueado esperava thread terminada, passa para fila de apto
                        thread_bloq->state = PROCST_APTO;
                        AppendFila2(&fila_apto, (void *) thread_bloq);
                        DeleteAtIteratorFila2(&fila_bloq);
                    }
                } while(NextFila2(&fila_bloq) == 0);
            }

            if(FirstFila2(&fila_bloq_susp) == 0) {
                TCB_t *thread_bloq_susp;
                do {
                    thread_bloq_susp = GetAtIteratorFila2(&fila_bloq_susp);
                    if(thread_bloq_susp->waitingThreadID == thread->tid) { // se thread em fila de bloqueado suspenso esperada thread terminada, passa para fila de apto suspenso
                        thread_bloq_susp->state = PROCST_APTO_SUS;
                        AppendFila2(&fila_apto_susp, (void *) thread_bloq_susp);
                        DeleteAtIteratorFila2(&fila_bloq_susp);
                    }
                } while(NextFila2(&fila_bloq_susp) == 0);
            }

            free(thread); // libera thread da memoria
        }
        thread_exec_not_finished = 0;
    }

    // muda thread do estado apto para executando
    if(FirstFila2(&fila_apto) == 0) { // se fila de aptos nao estiver vazia, seta iterador para primeiro elemento da fila
        thread = GetAtIteratorFila2(&fila_apto); // pega primeira thread da fila de aptos
        thread->state = PROCST_EXEC; // muda estado da thread para executando
        AppendFila2(&fila_exec, (void *) thread); // insere thread na fila de executando
        DeleteAtIteratorFila2(&fila_apto); // remove thread da fila de aptos
        setcontext(&thread->context); // retorna para o contexto da thread
    }
}

void init_scheduler()
{
    int criaFilaExec = CreateFila2(&fila_exec);
    int criaFilaApto = CreateFila2(&fila_apto);
    int criaFilaBloq = CreateFila2(&fila_bloq);
    int criaFilaAptoSusp = CreateFila2(&fila_apto_susp);
    int criaFilaBloqSusp = CreateFila2(&fila_bloq_susp);

    if(criaFilaExec == 0 && criaFilaApto == 0 && criaFilaBloq == 0 && criaFilaAptoSusp == 0 && criaFilaBloqSusp == 0) { // se criacao de filas foi bem sucedida

        // inicializa escalonador
        getcontext(&escalonador);
        escalonador.uc_link = 0; // uc_link eh um ponteiro para o contexto que vai ser executado quando este contexto
        //escalonador.uc_sigmask = ??; // uc_sigmask eh o conjunto de sinais que estao bloqueados quando este contexto esta ativado
        escalonador.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ)); // ss_sp � o ponteiro da pilha
        escalonador.uc_stack.ss_size = SIGSTKSZ; // ss_size eh o tamanho da pilha // SIGSTKSZ: Default size in bytes for the alternate signal stack (de acordo com signal.h)
        //escalonador.uc_context.ss_flags = ??; // ss_flags s�o flags da pilha
        makecontext(&escalonador, (void (*) (void)) dispatcher, 0);

        // cria a thread main
        TCB_t *thread_main = malloc(sizeof(TCB_t));
        thread_main->tid = 0; // como a thread main eh sempre a primeira a ser criada, seu thread_id sera sempre 0
        thread_main->state = PROCST_EXEC; // comeca executando, demais threads comecam na fila de aptos
        thread_main->prio = thread_priority;

        getcontext(&thread_main->context);
        if(AppendFila2(&fila_exec, (void *) thread_main) != 0) {
            printf("\nERRO: Falha ao criar a thread main.");
        }

        first_time_running = 0;
    }
    else {
        printf("\nERRO: Falha ao criar as filas de executando, apto, apto suspenso, bloqueado e bloqueado suspenso.");
    }
}

// **************************************************************************
// Funcoes obrigatorias (do tipo csomething)
int cidentify (char *name, int size)
{
    char names[] = "\nCassio Ramos - 193028\nGiovani Tirello - 252741\nMaicon Vieira - 242275\n";

    if(sizeof(names) > size) { // Se o tamanho da variavel 'names' for maior do que o informado em 'size'
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
    new_thread->state = PROCST_CRIACAO; // thread come�a no estado de criacao
    new_thread->prio = thread_priority; // sempre 0
    new_thread->waitingThreadID = -1;

    // inicializa contexto da nova thread
    if(getcontext(&new_thread->context) == 0) { // se inicializou corretamente

        // inicializa campos do contexto
        new_thread->context.uc_link = &escalonador; // uc_link � um ponteiro para o contexto que vai ser executado quando este contexto retornar
        //new_thread->context.uc_sigmask = ??; // uc_sigmask � o conjunto de sinais que est�o bloqueados quando este contexte est� ativado
        new_thread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ)); // ss_sp � o ponteiro da pilha
        new_thread->context.uc_stack.ss_size = SIGSTKSZ; // ss_size � o tamanho da pilha // SIGSTKSZ: Default size in bytes for the alternate signal stack (de acordo com signal.h)
        //new_thread->context.uc_context.ss_flags = ??; // ss_flags s�o flags da pilha
        makecontext(&new_thread->context, (void (*) (void)) start, 1, arg);

        // insere thread na fila de aptos
        if(AppendFila2(&fila_apto, (void *) new_thread) == 0) { // se inseriu corretamente
            new_thread->state = PROCST_APTO; // muda para estado de apto
            next_thread_id++; // incrementa indicador de ID das threads a serem criadas
            return new_thread->tid; // retorno com sucesso
        }
        else { // se nao inseriu corretamente na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir a nova thread %d na fila de aptos.", new_thread->tid);
            free(new_thread);
            return -1;
        }
    }
    else { // se nao inicializou corretamente o contexto da nova thread, mostra erro
        printf("\nERRO: Falha ao criar contexto para a nova thread %d.", new_thread->tid);
        free(new_thread);
        return -1;
    }
}

int cyield(void)
{
    if(FirstFila2(&fila_exec) == 0) { // se alguma thread esta executando
        TCB_t *yield_thread = GetAtIteratorFila2(&fila_exec); // guarda essa thread
        if(AppendFila2(&fila_apto, (void *) yield_thread) == 0) { // se inseriu essa thread na fila de aptos
            yield_thread->state = PROCST_APTO; // muda seu estado para apto
            DeleteAtIteratorFila2(&fila_exec); // remove-a da fila de execucao
            thread_exec_not_finished = 1;
            swapcontext(&yield_thread->context, &escalonador);
            return 0;
        }
        else { // se nao foi possivel inserir na fila de aptos, mostra erro
            printf("\nERRO: Falha ao inserir thread na fila de aptos.");
            return -1;
        }
    }
    else { // se nenhuma thread em execucao, mostra erro
        printf("\nERRO: Nenhuma thread em execucao para ceder processamento.");
        return -1;
    }
}

int cjoin(int tid)
{
    if(first_time_running) {
        init_scheduler();
    }

    int found_thread = 0;

    if(FirstFila2(&fila_exec) == 0) { // se fila de execu��o n�o estiver vazia
        TCB_t *thread = GetAtIteratorFila2(&fila_exec); // salva thread da fila de execu��o
        if(thread->tid != tid) { // se n�o � um auto-cjoin
            // procura nas quatro filas se thread do par�metro tid existe (thread ainda n�o terminou execu��o)
            if(FirstFila2(&fila_apto) == 0) {
                TCB_t *thread_apto;
                do {
                    thread_apto = GetAtIteratorFila2(&fila_apto);
                    if(thread_apto->tid == tid)
                        found_thread = 1;
                } while(NextFila2(&fila_apto) == 0);
            }

            if(FirstFila2(&fila_apto_susp) == 0) {
                TCB_t *thread_apto_susp;
                do {
                    thread_apto_susp = GetAtIteratorFila2(&fila_apto_susp);
                    if(thread_apto_susp->tid == tid)
                        found_thread = 1;
                } while(NextFila2(&fila_apto_susp) == 0);
            }

            if(FirstFila2(&fila_bloq) == 0) {
                TCB_t *thread_bloq;
                do {
                    thread_bloq = GetAtIteratorFila2(&fila_bloq);
                    if(thread_bloq->tid == tid)
                        found_thread = 1;
                } while(NextFila2(&fila_bloq) == 0);
            }

            if(FirstFila2(&fila_bloq_susp) == 0) {
                TCB_t *thread_bloq_susp;
                do {
                    thread_bloq_susp = GetAtIteratorFila2(&fila_bloq_susp);
                    if(thread_bloq_susp->tid == tid)
                        found_thread = 1;
                } while(NextFila2(&fila_bloq_susp) == 0);
            }

            if(found_thread) { // se thread do par�metro tid se encontra na fila de apto, apto suspenso, bloqueado ou bloqueado suspenso
                // procura na fila de bloqueado se existe alguma thread esperando pela thread do par�metro tid
                if(FirstFila2(&fila_bloq) == 0) { // se fila de bloqueados n�o estiver vazia
                    TCB_t *thread_bloq;
                    do {
                        thread_bloq = GetAtIteratorFila2(&fila_bloq); // salva thread da fila de bloqueado
                        if(thread_bloq->waitingThreadID == tid) { // se a thread em bloq est� esperando a thread tid
                            printf("\nERRO: Ja existe thread na fila de bloqueado esperando por essa thread.");
                            return -1;
                        }
                    } while(NextFila2(&fila_bloq) == 0); // procura na fila de bloqueado enquanto pr�ximo elem. n�o for final da fila
                }

                // procura na fila de bloqueado suspenso se existe alguma thread esperando por tid
                if(FirstFila2(&fila_bloq_susp) == 0) { // se bloqueado suspenso n�o vazio
                    TCB_t *thread_bloq_susp;
                    do {
                        thread_bloq_susp = GetAtIteratorFila2(&fila_bloq_susp); // salva thread de bloq suspenso
                        if(thread_bloq_susp->waitingThreadID == tid) { // se a thread em bloq susp est� esperando por tid
                            printf("\nERRO: Ja existe thread na fila de bloqueado suspenso esperando por essa thread");
                            return -1;
                        }
                    } while(NextFila2(&fila_bloq_susp) == 0); // procura na fila de bloqueado suspenso enquanto pr�ximo elem n�o for final da fila
                }

                // se n�o encontrou na fila de bloqueado e bloqueado suspenso uma thread que j� espera por tid, cjoin pode ocorrer
                thread->state = PROCST_BLOQ;
                AppendFila2(&fila_bloq, (void *) thread);
                DeleteAtIteratorFila2(&fila_exec);
                thread->waitingThreadID = tid;
                thread_exec_not_finished = 1;
                swapcontext(&thread->context, &escalonador);
                return 0;
            }
            else {
                printf("\nERRO: Thread ID invalido ou thread ja terminou execucao.");
                return -1;
            }
        }
        else {
            printf("\nERRO: Nao eh possivel um thread fazer auto-join");
            return -1;
        }
    }
    else {
        printf("\nERRO: Fila de execucao vazia.");
        return -1;
    }
}

int csuspend(int tid)
{
    if(FirstFila2(&fila_exec) != 0) { // se fila de executando vazia, mostrar erro
        printf("\nERRO: Fila de execu��o vazia. Nao foi possivel verificar se thread tentou se auto-suspender.");
        return -1;
    }
    else { // fila de execucao nao vazia
        TCB_t *thread_exec = GetAtIteratorFila2(&fila_exec); // salva thread executando
        if(thread_exec->tid == tid) { // se thread executando pediu para se auto-suspender
            printf("\nERRO: Nao eh possivel uma thread se auto-suspender.");
            return -1;
        }
        else { // se nao foi tentativa de se auto-suspender
            // procurar na fila de aptos
            if(FirstFila2(&fila_apto) == 0) { // se fila de aptos nao estiver vazia, seta iterador para primeiro elemento da fila
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_apto); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como parametro, encontrou a thread que deve ser suspensa
                        thread->state = PROCST_APTO_SUS; // muda status para apto_suspenso
                        AppendFila2(&fila_apto_susp, (void *) thread); // insere thread na fila de apto_suspenso
                        DeleteAtIteratorFila2(&fila_apto); // remove thread da fila de apto
                        return 0;
                    }

                } while (NextFila2(&fila_apto) == 0); // procura na fila de apto enquanto proximo elemento nao for final da fila
            }

            // se nao encontrou thread na fila de aptos, procura na fila de bloqueados
            if(FirstFila2(&fila_bloq) == 0) { // se fila de bloqueados nao estiver vazia, seta iterador para primeiro elemento da fila
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_bloq); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como parametro
                        thread->state = PROCST_BLOQ_SUS; // muda status para bloqueado_suspenso
                        AppendFila2(&fila_bloq_susp, (void *) thread); // insere thread na fila de bloqueado_suspenso
                        DeleteAtIteratorFila2(&fila_bloq); // remove thread da fila de bloqueado
                        return 0;
                    }
                } while (NextFila2(&fila_bloq) == 0); // procura na fila de bloqueado enquanto proximo elemento nao for final da fila
            }

            // se tamb�m n�o encontrou na fila de bloqueado, ou thread n�o existe ou n�o estava em apto ou bloqueado
            printf("\nERRO: Thread ID invalido ou thread nao eh passivel de suspensao.");
            return -1;
        }
    }
}

int cresume(int tid)
{
    if(FirstFila2(&fila_exec) != 0) { // se fila de executando vazia, mostrar erro
        printf("\nERRO: Fila de execu��o vazia. Nao foi possivel verificar se thread tentou se auto-resumir.");
        return -1;
    }
    else { // fila de execucao nao vazia
        TCB_t *thread_exec = GetAtIteratorFila2(&fila_exec); // salva thread executando
        if(thread_exec->tid == tid) { // se thread executando pediu para se auto-resumir
            printf("\nERRO: Nao eh possivel uma thread se auto-resumir.");
            return -1;
        }
        else { // se nao foi tentativa de se auto-resumir
            // procurar na fila de aptos_suspensos
            if(FirstFila2(&fila_apto_susp) == 0) { // se fila de aptos_suspensos nao estiver vazia, seta iterador para primeiro elemento da fila
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_apto_susp); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como parametro, encontrou a thread que deve ser resumida
                        thread->state = PROCST_APTO; // muda status para apto
                        AppendFila2(&fila_apto, (void *) thread); // insere thread na fila de apto
                        DeleteAtIteratorFila2(&fila_apto_susp); // remove thread da fila de apto_suspenso
                        return 0;
                    }

                } while (NextFila2(&fila_apto_susp) == 0); // procura na fila de apto_suspenso enquanto proximo elemento nao for final da fila
            }

            // se nao encontrou thread na fila de aptos_suspensos, procura na fila de bloqueados_suspensos
            if(FirstFila2(&fila_bloq_susp) == 0) { // se fila de bloqueados_suspensos nao estiver vazia, seta iterador para primeiro elemento da fila
                TCB_t *thread;
                do {
                    thread = GetAtIteratorFila2(&fila_bloq_susp); // salva thread apontada pelo iterador atual
                    if(thread->tid == tid) { // se a thread salva tem id igual ao tid passado como parametro, encontrou thread que deve ser resumida
                        thread->state = PROCST_BLOQ; // muda status para bloqueado
                        AppendFila2(&fila_bloq, (void *) thread); // insere thread na fila de bloqueado
                        DeleteAtIteratorFila2(&fila_bloq_susp); // remove thread da fila de bloqueado_suspenso
                        return 0;
                    }
                } while (NextFila2(&fila_bloq_susp) == 0); // procura na fila de bloqueado_suspenso enquanto proximo elemento nao for final da fila
            }

            //se tambem nao encontrou na fila de bloqueado_suspenso, ou thread nao existe ou nao estava suspensa
            printf("\nERRO: Thread ID invalido ou thread nao se encontra suspensa.");
            return -1;
        }
    }
}

int csem_init(csem_t *sem, int count)
{
    if(first_time_running) {
        init_scheduler();
    }

    sem->count = count; // indica se recurso esta ocupado ou nao (livre se > 0, ocupado se <=0)
    sem->fila = malloc(sizeof(FILA2)); // sera a fila de threads esperando o recurso

    if(CreateFila2(sem->fila) == 0) { // se fila criada com sucesso, termina com sucesso
        return 0;
    }
    else { // se nao, retorna mensagem de erro
        printf("\nERRO: Falha ao inicializar semaforo.");
        return -1;
    }
}

int cwait(csem_t *sem)
{
    sem->count--; // decrementa a cada chamada de cwait
    if(sem->count > 0) { // se recurso livre, thread continua execucao normalmente
        return 0;
    }
    else { // se recurso ocupado, colocar thread em estado bloqueado na fila do semaforo
        if(FirstFila2(&fila_exec) == 0) { // se alguma thread executando, seta iterador para primeiro elemento da fila
            TCB_t *thread = GetAtIteratorFila2(&fila_exec);
            thread->state = PROCST_BLOQ;
            AppendFila2(&fila_bloq, (void *) thread);
            AppendFila2(sem->fila, (void *) thread);
            DeleteAtIteratorFila2(&fila_bloq);
            thread_exec_not_finished = 1;
            swapcontext(&thread->context, &escalonador);
            return 0;
        }
        else {
            return -1;
        }
    }
}

int csignal(csem_t *sem)
{
    sem->count++; // incrementa a cada chamada da csignal
    if(FirstFila2(sem->fila) == 0) { // se fila do semaforo nao estiver vazia, seta iterador para primeiro elemento da fila
        TCB_t *thread_sem = GetAtIteratorFila2(sem->fila); // salva thread da fila do semaforo
        TCB_t *thread;
        do { // comeca a procurar thread na fila de bloqueado
            thread = GetAtIteratorFila2(&fila_bloq); // salva thread da fila de bloqueado
            if(thread_sem->tid == thread->tid) { // se eh a mesma thread
                thread->state = PROCST_APTO; // muda estado para apto
                thread->waitingThreadID = -1;
                AppendFila2(&fila_apto, (void *) thread); // insere thread na fila de apto
                DeleteAtIteratorFila2(&fila_bloq); // remove thread da fila de bloqueado
                DeleteAtIteratorFila2(sem->fila); // remove thread da fila do semaforo
                return 0;
            }
        } while(NextFila2(&fila_bloq) == 0); // procura na fila de bloqueado enquanto proximo elemento nao for final da fila

        do { // se nao encontrou thread na fila de bloqueado, procura na fila de bloqueado_suspenso
            thread = GetAtIteratorFila2(&fila_bloq_susp);
            if(thread_sem->tid == thread->tid) { // se eh a mesma thread
                thread->state = PROCST_APTO_SUS; // muda estado para apto_suspenso
                thread->waitingThreadID = -1;
                AppendFila2(&fila_apto_susp, (void *) thread); // insere thread na fila de apto_suspenso
                DeleteAtIteratorFila2(&fila_bloq_susp); // remove thread da fila de bloqueado_suspenso
                DeleteAtIteratorFila2(sem->fila); // remove thread da fila do semaforo
                return 0;
            }
        } while(NextFila2(&fila_bloq_susp) == 0); // procura na fila de bloqueado suspenso enquanto pr�ximo elemento n�o for final da fila

        printf("\nERRO: Thread n�o encontrada.");
        return -1;
    }
    else { // se fila do semaforo estiver vazia
        printf("\nERRO: Fila do semaforo esta vazia.");
        return -1;
    }
}
