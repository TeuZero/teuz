#include <iostream>
#include <windows.h>
#include <fstream>
#include <locale.h>

typedef unsigned __int64 QWORD, *PQWORD;

/*
* Estrutura de dados
*/

typedef struct{
	QWORD nameS;
	DWORD tv;
	DWORD addressV;
	DWORD tamCru;
	DWORD addressCru;
	DWORD addressReloc;
	DWORD nLinesReloc;
	WORD nLinesR;
	WORD nLinesRnotUsed;
	DWORD caracteristicasS;
}IMAGE_D_DADOS;

/*
* Estrutura de seção
*/

typedef struct {
	QWORD nameS;
	DWORD tv;
	DWORD addressV;
	DWORD tamCru;
	DWORD addressCru;
	DWORD addressReloc;
	DWORD nLinesReloc;
	WORD nLinesR;
	WORD nLinesRnotUsed;
	DWORD caracteristicasS;
}IMAGE_D_SECION;

int main(int argc, char** argv) {
	setlocale(LC_ALL, "Portuguese");
	DWORD key = 0xFF;
	
	//IMAGE DO ARQUIVO
	//================================================================================			
	IMAGE_DOS_HEADER* mz;
	mz = (IMAGE_DOS_HEADER*)malloc(sizeof(IMAGE_DOS_HEADER));
	
	
	IMAGE_NT_HEADERS32* inh;
	inh = ( IMAGE_NT_HEADERS32*)malloc(sizeof(IMAGE_NT_HEADERS32));
	
	//Seção de cabeçalho de dados
	IMAGE_D_DADOS* idd;
	idd = (IMAGE_D_DADOS*)malloc(sizeof(IMAGE_D_DADOS));
	
	//Seção de cabeçalho de código
	IMAGE_D_SECION* ids;
	ids = (IMAGE_D_SECION*)malloc(sizeof(IMAGE_D_SECION));
	//================================================================================
	
	
	/*
	*
	*	Abri o arquivo, lê todos os bytes dele e grava na variavel buffer.
	*	
	*/
	
	std::fstream is (argv[1], std::ios::binary | std::ios::out | std::ios::in); // abrir o arquivo modo leitura e escrita em binary
	
	if(!is) // se não conseguir abrir retorna 1
		return 1;
		
	is.seekp(0,is.end); // seta para o final do arquivo
	int length = is.tellp(); // diz quandos bytes tem até onde ele tá 
	is.seekp(0, is.beg); // devolve para o inicio
	
	char *buffer = new char[length]; // alocar espaço na variavel do tamanho do arquivo

	std::cout << "\n Reading: " << length << " bytes...\n\n";
	
	is.read(buffer,length); // lê o tamanho length no arquivo e armazena no buffer
	is.close(); // fecha arquivo
	
	
	/*
	*
	*	Copia bytes de IMAGE no arquivo para as estruturas IMAGE.
	*	
	*/
	
	memcpy(mz,buffer,sizeof(IMAGE_DOS_HEADER));
	memcpy(inh,buffer+mz->e_lfanew,sizeof(IMAGE_NT_HEADERS32));
		
	
		
	/*
	*
	*	Mostra informações da "Estrutura de dados" na tela.
	*	
	*/
	
	DWORD offset;
	DWORD offsetHData;
	
	printf("\n================== File data header information  ==================\n\n");
	
	for(int i = 0; i<length; i++){
		offset++;
		if(buffer[i+1] == '.' && buffer[i+2] == 'd' && buffer[i+3] == 'a' && buffer[i+4] == 't'){
		
			std::cout << " [+] Cabeçalho da seção de dados começa no: Offset: 0x"<< std::hex << offset << std::endl;
			offsetHData = offset;
			offset+8;
			i+8;
			break;
		}
	}
	
	//Copia bytes de IMAGE no arquivo para as "Estrutura de dados".
	memcpy(idd,buffer+offsetHData,sizeof(IMAGE_D_DADOS));
	
	std::cout << " [+] Size Of Raw Data: 0x"<< std::hex << idd->tamCru << std::endl;
	std::cout << " [+] Virtual Size: 0x"<< std::hex << idd->tv << std::endl;
	std::cout << " [+] Pointer To Raw Data: 0x" << std::hex << idd->addressCru << std::endl;
	std::cout << " [+] Virtual Address: 0x"<< std::hex << idd->addressV << std::endl;
	
	printf("\n===================================================================\n\n");
		
	
	/*
	*
	*	Encripta seção de DADOS.
	*	
	*/
	
	DWORD terminateD = idd->addressCru+idd->tamCru;
	
	for(int i = 0; i<length; i++){
		offset++;
		if(i == idd->addressCru){
			for(;i<length;i++){
				//buffer[i] = buffer[i] + 0x26;
				buffer[i] = buffer[i] ^ key;
				//buffer[i] = buffer[i] + 0x05;		
				
				if(i == terminateD)
					break;
			}
		}
	}

	
	/*
	*
	*	Mostra informações da "Estrutura de seção" na tela.
	*	
	*/
	
	memcpy(ids,buffer+mz->e_lfanew+248,sizeof(IMAGE_D_SECION));
	DWORD terminateS = ids->addressCru + ids->tamCru;
	DWORD bytesLivre = ids->tamCru - ids->tv;
	
	printf("\n================== File code header information  ==================\n\n");
	
	std::cout << " [+] Seção de código termina no offset: 0x"<< std::hex  << std::hex << terminateS << std::endl;
	std::cout << " [+] Size Of Raw Data: 0x" << std::hex << ids->tamCru << std::endl;
	std::cout << " [+] Virtual Size: 0x"  << std::hex << ids->tv << std::endl;
	std::cout << " [+] Pointer To Raw Data: 0x" << std::hex << ids->addressCru << std::endl;
	std::cout << " [+] Virtual Address: 0x" << std::hex << ids->addressV << std::endl;
	std::cout << " [+] Bytes livres no fim da: "<< std::dec << bytesLivre << std::endl;
	
	printf("\n===================================================================\n\n");
	
	
	/*
	*
	*	Adiciona função decripter no programa.
	*	
	*/
		
	DWORD offsetEntryPointEndCode = terminateS - bytesLivre - ids->addressCru + (ids->addressV+32); 
	DWORD offsetSecionDados = inh->OptionalHeader.ImageBase + idd->addressV;
	DWORD terminateDd = inh->OptionalHeader.ImageBase+idd->addressV  + idd->tamCru;
	DWORD loopOffsetEnc = offsetEntryPointEndCode + 5;
	DWORD entryPointV = inh->OptionalHeader.ImageBase+offsetEntryPointEndCode;
	DWORD entryPointOriginal = inh->OptionalHeader.ImageBase + inh->OptionalHeader.AddressOfEntryPoint;
	
	printf("\n================== Necessary informations  ==================\n\n");
	
	std::cout << " [+] EntryPointer Original: 0x" << std::hex << entryPointOriginal << std::endl;	
	std::cout << " [+] EntryPointerV: 0x" << std::hex << entryPointV << std::endl;	
	std::cout << " [+] Novo EntryPointer virtual: 0x" << std::hex << offsetEntryPointEndCode << std::endl;
	std::cout << " [+] Offset Virtual da seção de dados: 0x" << std::hex << offsetSecionDados << std::endl;
	std::cout << " [+] Seção de dados virtual termina no offset: 0x" << std::hex << terminateDd << std::endl;
	std::cout << " [+] Offset virtual do loop do Encripter 0x" << std::hex << loopOffsetEnc << std::endl;
	
	printf("\n===================================================================\n\n");
	
	
	
	char offsetEntryPointEndCode2[4];
	char offsetSecionDados2[4];
	char terminateD2[4];
	char loopOffsetEnc2[4];
	char entryPointV2[4];
	char entryPointOriginal2[4];
	
	std::cout << " [+] Digite o EntryPointer Original aqui: 0x"; 
	scanf("%x",&entryPointOriginal2);
	std::cout << std::endl;
	
	std::cout << " [*] Digite o novo EntryPointer aqui: ";
	scanf("%x",&offsetEntryPointEndCode2);
	std::cout << std::endl;
	 
	std::cout << " [*] Digite Offset Virtual da seção de dados aqui: " ;
	scanf("%x",&offsetSecionDados2);
	std::cout << std::endl;
	
	std::cout << " [*] Digite onde a Seção de dados termina: "  ;
	scanf("%x",&terminateD2);
	std::cout << std::endl;
	
	std::cout << " [+] Digite Offset virtual do loop do Encripter aqui: " ;
	scanf("%x",&loopOffsetEnc2);
	std::cout << std::endl;
	
	std::cout << " [+] Digite EntryPointerV: " ;
	scanf("%x",&entryPointV2);
	std::cout << std::endl;
	
	offset = 0;
	for(int p =0;p < length;p++){
		offset++;
		if (p == mz->e_lfanew+40){
			//printf("\n entry: 0x%X%X%X%X",enc[p],enc[p+1],enc[p+2],enc[p+3]);
			buffer[p] = offsetEntryPointEndCode2[0];
			buffer[p+1] = offsetEntryPointEndCode2[1];
			buffer[p+2] = offsetEntryPointEndCode2[2];	
			buffer[p+3] = offsetEntryPointEndCode2[3];
			offset++;
			for(;offset < length;p++){
				offset++;
				//printf("\n%X",offsety);
				if(p == (terminateS - bytesLivre) +32){
					//printf("\nacou %X",offsety);
					offset++;
						buffer[p] = 0xB8;
						buffer[p+1] = offsetSecionDados2[0];
						buffer[p+2] = offsetSecionDados2[1];
						buffer[p+3] = offsetSecionDados2[2];
						buffer[p+4] = offsetSecionDados2[3];
						
						buffer[p+5] = 0x80;
						buffer[p+6] = 0x30;
						buffer[p+7] = 0xFF;
						buffer[p+8] = 0x40;
						
			
						buffer[p+9] = 0x3D;
						buffer[p+10] = terminateD2[0];
						buffer[p+11] = terminateD2[1];
						buffer[p+12] = terminateD2[2];
						buffer[p+13] = terminateD2[3];
						
						buffer[p+14] = 0x7E;
						buffer[p+15] = loopOffsetEnc2[0];
						buffer[p+16] = loopOffsetEnc2[1];
						buffer[p+17] = loopOffsetEnc2[3];
									
						buffer[p+18] = 0x68;
						
						buffer[p+19] = entryPointOriginal2[0];
						buffer[p+20] = entryPointOriginal2[1];
						buffer[p+21] = entryPointOriginal2[2];
						buffer[p+22] = entryPointOriginal2[3];
						offset++;
						/*
						if( p == r2 - 2){
						printf("\nachou %X",offsety);
							break;
						
					}
					break;
					*/
				}
			}
		}
	}
	

	std::fstream is2("test3.exe", std::ios::binary | std::ios::out); // abrir o arquivo modo leitura e escrita em binary
	is2.write(buffer,length);
	is2.close();

	free(mz);
	free(inh);
	free(idd);
	free(ids);
}
