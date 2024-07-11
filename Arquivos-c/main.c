#include "../Arquivos-h/menu.h"

int main(){
    menu();
    return 0;
}

int menu(){
    Assembly *AssemblyInst = NULL;
    MemoriaDados *memDados = NULL;
    Backup *backup = NULL, *backupInicio = NULL;
    NodoPilha *NodoPilha = NULL;
    descPilha *descPilha = NULL;
    char escolha;
    unsigned int tamLinhas, program_counter = 0, contadorCiclo = 0, cont = 0; //UNSIGNED IMPOSSIBILITA QUE PROGRAM_COUNTER CHEGUE A MENOR QUE 0
    int Etapa = 1, i, auxiliar;
    Sinais *sinal = NULL;
    PipeRegisters pipes;
    IF *regif = NULL;
    ID *id = NULL;
    EX *ex = NULL;
    MEM *mem = NULL;
    WB *wb = NULL;
    instrucao *memInst = NULL; //RESPONSAVEL POR COLETAR  A INSTRUÇÃO
    type_instruc *instrucoesDecodificadas = NULL;
    int regs[8];
    //int *regs = NULL; //registradores como um inteiro mesmo
    char dat[300]; //Recebe o nome do arquivo.dat
    //regs = (int*)malloc(8 * sizeof(int));
    for (i=0;i<8;i++){ //zerando registradores, caso contrario dá números inconsistentes
        regs[i] = 0;
    }

    do{
        //getchar();
        escolha = terminal(&contadorCiclo, &program_counter, memInst, tamLinhas, instrucoesDecodificadas, regs, regif, id, ex, mem, wb);

        switch (escolha)
        {
        case '0':
            freeALL(regs, AssemblyInst, memInst, memDados, instrucoesDecodificadas, regif, id, ex, mem, wb , sinal);
            system("clear");
            printf("Programa Encerrado!\n");
            break;
            
        case '1': //Carregar memória de Instruções e inicializa tudo
            if(memInst != NULL){
                printf("Memoria ja preenchida\n");
                break;
            }
            memInst = inicializaMemInst(); //inicializa memoria de instruções
            parser(memInst, &tamLinhas);
            pipes = inicializaRegsPipe();
            IF *regif = pipes.regif;
            ID *id = pipes.id;
            EX *ex = pipes.ex;
            MEM *mem = pipes.mem;
            WB *wb = pipes.wb;
            inicializaRegsPipe(regif, id, ex, mem, wb);
            memDados = inicializaMemDados(); //inicializa memoria de dados
            descPilha = inicializaBackup();
            instrucoesDecodificadas = calloc(tamLinhas, sizeof(type_instruc));
            if (instrucoesDecodificadas == NULL) {
                fprintf(stderr, "Falha ao alocar memória para instruções decodificadas.\n");
                return -1;
            }
            
            AssemblyInst = calloc((tamLinhas + 1), sizeof(Assembly));
            
            if (AssemblyInst == NULL) {
                fprintf(stderr, "Falha ao alocar memória para instrucoes assembly.\n");
                return -1;
            }
            //FAZ UM "BACKUP" PARA O BACKSTEP   
                backup = ColetaTudo(regs, memDados, regif, id, ex, mem, wb, sinal, AssemblyInst, &program_counter, &Etapa);
                backupInicio = ColetaTudo(regs, memDados, regif, id, ex, mem, wb, sinal, AssemblyInst, &program_counter, &Etapa);
                NodoPilha = inicializaNodo(backup);
                descPilha = PUSH(descPilha, NodoPilha);
            break;

        case '2': //Carregar Memória de Dados
            if (program_counter == 0){
                strcpy(dat,carregamemDados(memDados));
                printf("\n");
                puts(dat);
                printf("\n");
                if (strcmp(dat,"ERRO")!=0){
                    cont = 1;
                }
            }
            else 
                printf("Programa nao deve ja ter sido inicializado.");
            break;

        case '3': //Imprimir memória de instruções
            INST(memInst);
            break;

        case '4': //Imprimir memória de dados
            DATA(memDados);
            break;

        case '5': //Imprimir instruções Assembly
            ASSEMBLYTERMINAL(AssemblyInst);
            break;
            
        case '6': //salvar arquivo .asm (com as instruções traduzidas para a linguagem intermediária Assembly)
            SaveASM(AssemblyInst, tamLinhas);
            break;

        case '7': //Salvar arquivo DATA.dat
            escreverArquivoMemoria(memDados);
            break;

        case '8': //Chamar função responsável pela execução do programa
            if (memInst == NULL){
                printf("Carregue a memoria com instrucoes antes.\n");
                break;
            }
            if(Etapa == 6){
                printf("MIPS ja executou as instrucoes\n");
                break;
            }
            program_counter = 0;
            Etapa = controller(&contadorCiclo, 1, tamLinhas, regs, memInst, memDados, &program_counter, instrucoesDecodificadas, regif, id, ex, mem, wb, sinal, Etapa, descPilha, backup, NodoPilha, AssemblyInst);
            AsmCopy(instrucoesDecodificadas, AssemblyInst, tamLinhas);
            //FAZ UM "BACKUP" PARA O BACKSTEP   
                backup = ColetaTudo(regs, memDados, regif, id, ex, mem, wb, sinal, AssemblyInst, &program_counter, &Etapa);
                NodoPilha = inicializaNodo(backup);
                descPilha = PUSH(descPilha, NodoPilha);
            break;

        case '9': //Chamar função responsável pela execução do programa passo a passo
            if (memInst == NULL){
                printf("Carregue a memoria com instrucoes antes.\n");
                break;
            }
            if(Etapa == 6){
                printf("MIPS ja executou as instrucoes\n");
                break;
            }
            Etapa = controller(&contadorCiclo, 2, tamLinhas, regs, memInst, memDados, &program_counter, instrucoesDecodificadas, regif, id, ex, mem, wb, sinal, Etapa, descPilha, backup, NodoPilha, AssemblyInst);
            AsmCopy(instrucoesDecodificadas, AssemblyInst, tamLinhas);
            printf("\n");
            //FAZ UM "BACKUP" PARA O BACKSTEP   
                backup = ColetaTudo(regs, memDados, regif, id, ex, mem, wb, sinal, AssemblyInst, &program_counter, &Etapa);
                NodoPilha = inicializaNodo(backup);
                descPilha = PUSH(descPilha, NodoPilha);
            contadorCiclo++;
            break;

        case 'A':
        case 'a': //Chamar função responsável por retornar uma instrução (PC--)
            descPilha = Realoca(descPilha, regs, memDados, regif, id, ex, mem, wb, sinal, AssemblyInst, &program_counter, &Etapa);
            printw("Retornamos para:\nWB-Instrucao:[%s]\nMEM-Instrucao:[%s]\nEX-Instrucao:[%s]\nID-Instrucao[%s]\nIF-Instrucao:[%s]\n", wb->InstrucaoASM.InstructsAssembly, mem->InstrucaoASM.InstructsAssembly, ex->InstrucaoASM.InstructsAssembly, id->InstrucaoASM.InstructsAssembly, regif->InstrucaoASM.InstructsAssembly);
            sleep(2);
            if(descPilha->Topo == NULL){
                printf("Estamos no início do programa.\n");
                NodoPilha = inicializaNodo(backupInicio);
                descPilha = PUSH(descPilha, NodoPilha);
                contadorCiclo = 0;
                break;
            } 
            contadorCiclo--;
            break;

        default:
            printf("Opcao invalida.\n");
            break;
        }
    }while(escolha != '0');

    return 0;
}
