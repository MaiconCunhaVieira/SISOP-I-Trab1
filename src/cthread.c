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
int thread_exec_finished = 0;

ucontext_t escalonador;
FILA2 fila_exec;
FILA2 fila_apto;
FILA2 fila_bloq;
FILA2 fila_apto_susp;
FILA2 fila_bloq_susp;

// **************************************************************************
// Outras fun��es
void dispatcher()
{
    // ou termina execu��o de thread e coloca outra thread para executar (se thread atualmente executando terminou execu��o)
    // ou apenas coloca outra thread para executar (se thread deu cyield, cjoin ou cwait)

    TCB_t *thread;

    // muda thread do estado executando para t�rmino se thread terminou execu��o
    if(thread_exec_finished) { // se thread atualmente em execu��o terminou sua execu��o
        if(FirstFila2(&fila_exec) == 0) { // se alguma thread em execu��o na fila de execu��o, seta iterador para primeiro elemento da fila
            thread = GetAtIteratorFila2(&fila_exec); // salva thread
            thread->state = PROCST_TERMINO; // muda estado para t�rmino
            DeleteAtIteratorFila2(&fila_exec); // remove thread da fila de execu��o
            // checar se outras thread em bloqueado e bloqueado suspenso n�o esperavam recursos dessa thread terminada
            free(thread); // libera thread da mem�ria
        }
        thread_exec_finished = 0;
    }

    // muda thread do estado apto para executando
    if(FirstFila2(&fila_apto) == 0) { // se fila de aptos n�o estiver vazia, seta iterador para primeiro elemento da fila
        thread = GetAtIteratorFila2(&fila_apto); // pega primeira thread da fila de aptos
        thread->state = PROCST_EXEC; // muda estado da thread para executando
        AppendFila2(&fila_exec, (void *) thread); // insere thread na fila de executando
        DeleteAtIteratorFila2(&fila_apto); // remove thread da fial de aptos
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

    if(criaFilaExec == 0 && criaFilaApto == 0 && criaFilaBloq == 0 && criaFilaAptoSusp == 0 && criaFilaBloqSusp == 0) { // se cria��o de filas foi bem sucedida

        // inicializa escalonador
        getcontext(&escalonador);
        escalonador.uc_link = 0; // uc_link � um ponteiro para o contexto que vai ser executado quando este contexto  (� = 0?)
        //escalonador.uc_sigmask = ??; // uc_sigmask � o conjunto de sinais que est�o bloqueados quando este contexte est� ativado (talvez desnecess�rio)
        escalonador.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ)); // ss_sp � o ponteiro da pilha
        escalonador.uc_stack.ss_size = SIGSTKSZ; // ss_size � o tamanho da pilha // SIGSTKSZ: Default size in bytes for the alternate signal stack (de acordo com signal.h)
        //escalonador.uc_context.ss_flags = ??; // ss_flags s�o flags da pilha (talvez desnecess�rio)
        makecontext(&escalonador, (void (*) (void)) dispatcher, arg); // argumentos talvez incorretos (start? (talvez dispatcher), arg? (talvez 0))

        // cria a thread main
        TCB_t *thread_main = malloc(sizeof(TCB_t));
        thread_main->tid = 0; // como a thread main � sempre a primeira a ser criada, seu thread_id ser� sempre 0
        thread_main->state = PROCST_EXEC; // come�a executando, demais threads come�am na fila de aptos
        thread_main->prio = thread_priority;

        getcontext(&thread_main->context);
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
        new_thread->context.uc_link = &escalonador; // uc_link � um ponteiro para o contexto que vai ser executado quando este contexto retornar
        //new_thread->context.uc_sigmask = ??; // uc_sigmask � o conjunto de sinais que est�o bloqueados quando este contexte est� ativado (talvez desnecess�rio)
        new_thread->context.uc_stack.ss_sp = malloc(sizeof(SIGSTKSZ)); // ss_sp � o ponteiro da pilha
        new_thread->context.uc_stack.ss_size = SIGSTKSZ; // ss_size � o tamanho da pilha // SIGSTKSZ: Default size in bytes for the alternate signal stack (de acordo com signal.h)
        //new_thread->context.uc_context.ss_flags = ??; // ss_flags s�o flags da pilha (talvez desnecess�rio)
        makecontext(&new_thread->context, (void (*) (void)) start, arg); // argumentos talvez incorretos

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
            thread_exec_finished = 0;
            swapcontext(&thread->context, &escalonador);
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

int cjoin(int tid)//funcao de outro trabalho, adaptar variaveis
{
  lWait *Novo; // estrututa criada que guarda a tid da thread que está sendo trancada e a tid de quem libera ela
  	int verifica;
  	TCB_t *threadAtual;
  	if(inicializar == 0)
  	{
  		inicializa();//funcao ainda nao definida, pode ser alterada por inicializacao da thread
  		return 0;
  	}
  	if(tid<=tidAtual){ // Cjoin só é feito se a tid recebida como parametro já foi atribuida a alguem
  		if (thread_exec_finished(tid) == 0){ // e se a tid não pertence a uma thread que já terminou-----adaptar variavel
  			if(threadExec -> tid == tid) // uma thread não pode se autobloquear
  				return -1;
  			if(FirstFila2(&ListaEspera) != 0){ // Se não tem ninguém bloqueado por join, ele adiciona a thread nos bloqueados e na lista de espera que contem os tid's
  				Novo = (lWait *)malloc(sizeof(lWait));
  				Novo -> tidThread = threadExec -> tid;
  				Novo -> tidEspera = tid;
  				threadExec -> state = PROCST_BLOQ;
  				AppendFila2(&FilaBloqueado, (void *) threadExec);
  				AppendFila2(&ListaEspera, (void *) Novo);
  				swapcontext(&threadExec->context, schedulerContext);
  				return 0;
  			}
  			else{
  				while(NextFila2(&ListaEspera) == 0){ // percorre toda lista de espera para ver se a thread que ele quer esperar já não está sendo esperada por outra, -1 em caso de erro
  					threadAtual = (TCB_t *)GetAtIteratorFila2(&ListaEspera);
  					if (tid == threadAtual -> tid);
  						return -1;
  				} // senão vai para bloqueado e entra na lista de espera
  				Novo = (lWait *)malloc(sizeof(lWait));
  				Novo -> tidThread = threadExec -> tid;
  				Novo -> tidEspera = tid;
  				threadExec -> state = PROCST_BLOQ;
  				AppendFila2(&FilaBloqueado, (void *) threadExec);
  				AppendFila2(&ListaEspera, (void *) Novo);
  				swapcontext(&threadExec->context, schedulerContext);
  	    			return 0;
  	    		}
  		}
  		return -1;
  	}
  	return -1;
  }
}

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
            if(FirstFila2(&fila_apto) == 0) { // se fila de aptos n�o estiver vazia, seta iterador para primeiro elemento da fila
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
            if(FirstFila2(&fila_bloq) == 0) { // se fila de bloqueados n�o estiver vazia, seta iterador para primeiro elemento da fila
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
            if(FirstFila2(&fila_apto_susp) == 0) { // se fila de aptos_suspensos n�o estiver vazia, seta iterador para primeiro elemento da fila
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
            if(FirstFila2(&fila_bloq_susp) == 0) { // se fila de bloqueados_suspensos n�o estiver vazia, seta iterador para primeiro elemento da fila
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
    if(first_time_running) {
        init_scheduler();
    }

    sem->count = count; // indica se recurso est� ocupado ou n�o (livre se > 0, ocupado se <=0)
    sem->fila = malloc(sizeof(FILA2)); // ser� a fila de threads esperando o recurso

    if(CreateFila2(sem->fila) == 0) { // se fila criada com sucesso, termina com sucesso
        return 0;
    }
    else { // se n�o, retorna mensagem de erro
        printf("\nERRO: Falha ao inicializar semaforo.");
        return -1;
    }
}

int cwait(csem_t *sem)
{
    sem->count--; // decrementa a cada chamada de cwait
    if(sem->count > 0) { // se recurso livre, thread continua execu��o normalmente
        return 0;
    }
    else { // se recurso ocupado, colocar thread em estado bloqueado na fila do sem�foro
        if(FirstFila2(&fila_exec) == 0) { // se alguma thread executando, seta iterador para primeiro elemento da fila
            TCB_t *thread = GetAtIteratorFila2(&fila_exec);
            thread->state = PROCST_BLOQ;
            AppendFila2(&fila_bloq, (void *) thread);
            AppendFila2(sem->fila, (void *) thread);
            DeleteAtIteratorFila2(&fila_bloq);
            thread_exec_finished = 0;
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
    if(FirstFila2(sem->fila) == 0) { // se fila do sem�foro n�o estiver vazia, seta iterador para primeiro elemento da fila
        TCB_t *thread_sem = GetAtIteratorFila2(sem->fila); // salva thread da fila do sem�foro
        TCB_t *thread;
        do { // come�a a procurar thread na fila de bloqueado
            thread = GetAtIteratorFila2(&fila_bloq); // salva thread da fila de bloqueado
            if(thread_sem->tid == thread->tid) { // se � a mesma thread
                thread->state = PROCST_APTO; // muda estado para apto
                AppendFila2(&fila_apto, (void *) thread); // insere thread na fila de apto
                DeleteAtIteratorFila2(&fila_bloq); // remove thread da fila de bloqueado
                DeleteAtIteratorFila2(sem->fila); // remove thread da fila do sem�foro
                return 0;
            }
        } while(NextFila2(&fila_bloq) == 0); // procura na fila de bloqueado enquanto pr�ximo elemento n�o for final da fila

        do { // se n�o encontrou thread na fila de bloqueado, procura na fila de bloqueado_suspenso
            thread = GetAtIteratorFila2(&fila_bloq_susp);
            if(thread_sem->tid == thread->tid) { // se � a mesma thread
                thread->state = PROCST_APTO_SUS; // muda estado para apto_suspenso
                AppendFila2(&fila_apto_susp, (void *) thread); // insere thread na fila de apto_suspenso
                DeleteAtIteratorFila2(&fila_bloq_susp); // remove thread da fila de bloqueado_suspenso
                DeleteAtIteratorFila2(sem->fila); // remove thread da fila do sem�foro
                return 0;
            }
        }
    }
    else { // se fila do sem�foro estiver vazia
        printf("\nERRO: Fila do semaforo esta vazia.");
        return -1;
    }
}
