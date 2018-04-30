/*
 * cdata.h: arquivo de inclusï¿½o de uso apenas na geraï¿½ï¿½o da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida.
 * Se necessï¿½rio, pode-se ACRESCENTAR campos ao TCP
 *
 * VERSï¿½O: 17/03/2018
 *
 */
#ifndef __cdata__
#define __cdata__
#include "ucontext.h"

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4
#define	PROCST_APTO_SUS	5
#define	PROCST_BLOQ_SUS	6

/* Os campos "tid", "state" e "context" dessa estrutura devem ser mantidos e usados convenientemente
   Pode-se acrescentar outros campos APï¿½S os campos obrigatï¿½rios dessa estrutura
*/
typedef struct s_TCB {
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
<<<<<<< HEAD
					// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado; 4: Término; 5: Apto-Suspenso; 6: Bloqueado-Suspenso
	int 		prio;		// Prioridade associada a thread NÃO USADO NESSA IMPLEMENTAÇÃO
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos)

	/* Se necessário, pode-se acresecentar campos nessa estrutura A PARTIR DAQUI! */
	int waitingThreadID;    // identificador da thread que desbloqueia essa thread (-1 se não espera por nenhuma thread)

} TCB_t;
=======
					// 0: Criaï¿½ï¿½o; 1: Apto; 2: Execuï¿½ï¿½o; 3: Bloqueado; 4: Tï¿½rmino; 5: Apto-Suspenso; 6: Bloqueado-Suspenso
	int 		prio;		// Prioridade associada a thread Nï¿½O USADO NESSA IMPLEMENTAï¿½ï¿½O
	ucontext_t 	context;	// contexto de execuï¿½ï¿½o da thread (SP, PC, GPRs e recursos)

	/* Se necessï¿½rio, pode-se acresecentar campos nessa estrutura A PARTIR DAQUI! */


} TCB_t; 
>>>>>>> 132742040320566dc76b23ffb3bbbc598eef5aec

typedef struct ListaWait{ // Criada para guardar a tid da thread que estÃ¡ sendo bloqueada e no tid espera Ã© guardado o tid da thread que desbloqueia ela
	int tidThread; // tid thread bloqueada
	int tidEspera; // tid thread que desbloqueia a de cima
} lWait;

#endif
