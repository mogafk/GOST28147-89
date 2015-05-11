#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include <string.h>

#include <math.h>

#include <ctime>

/**
 * Циклический сдвиг влево
 * @param   input  число в котором происходит сдвиг
 * @param   shift  количетсво битов на которое сдвигаем
 * @returns результат бинарной операции
 */
uint32_t rol(int32_t input, int shift){
    return ((input << shift) | (input >> (32 - shift)));
}

/**
 * Переводит строку в 64 битное число
 * @param   str указатель на строку
 * @returns 64битное число
 */
int64_t str2int(char str[]){
	int64_t resultInt = 0;
	int i;

	resultInt = resultInt | (int)str[i];
	for(i=0; i<8; i++){
		resultInt = resultInt << 8;
		resultInt = resultInt | (int)str[i];
	}

	return resultInt;
}

/**
 * Переводит число в строку
 * @param   input  число
 * @returns укатель на строку
 */
char *int2str(int64_t input){
	static char result[8];

	int i;
	for(i=7; i>=0; i--){
		result[i] = input & (0xFF);
		input = input >> 8;
	}

    return result;
}

/**
 * Соединяет массив длиной 8 по 4 бита
 * @param   splited32
 * @returns 
 */
uint64_t join32bit(int *splited32){
	uint64_t result = 0;

	int i;
	for(i=0; i<8; i++){
		result = result | splited32[i];
		result = result << 4;
	}
	result = result >> 4; //один сдвиг лишний
	return result;
}

/**
 * Разбивает 32битное число на 8 по 4 бита в массив
 * @param   input число для разбивки
 * @returns указатель на массив.
 */
int *split32bit(uint32_t input){
    static int result[8];
    
    int i=0;
    for(i=7; i>=0; i--){ //last elem of result it last 4 bit from input
        result[i] = (input & (0xF));
        input = input >> 4;
    }

    return result;
}

/**
 * Генерирует 8 ключей по 32 бита и записывает в массив.
 * @returns указатель на массив
 */
int32_t *keygen(){
	static int result[8];
	srand(time(NULL));

	int i;
	for(i=0; i<8; i++){
		result[i] = rand()%0xFFFFFFFF;
	}

	return result;
}

/**
 * Производит замену по sbox. Функция имутабельна(аргумент изменяется по ссылке)
 * @param   splited32 массив из 8 элементов
 *                    по 4 бита. Результат split32bit
 */
void rSbox(int *splited32){//finally
    int Sbox[8][16] = {
	    {0x5,0x9,0x2,0x4,0x8,0x7,0x3,0xA,0xF,0x1,0xD,0xB,0x6,0xE,0x0,0xC},
	    {0x7,0xD,0x5,0xC,0x2,0xE,0x4,0x3,0x0,0x8,0xA,0x9,0x1,0x6,0xB,0xF},
	    {0xC,0x9,0xB,0x4,0xF,0x5,0xA,0x1,0xE,0x3,0x7,0xD,0x8,0x6,0x0,0x2},
	    {0xA,0x8,0x4,0x0,0x6,0xF,0xB,0x3,0xC,0xD,0x2,0x1,0x7,0x5,0x9,0xE},
	    {0x8,0xA,0xF,0x6,0x9,0xB,0x4,0xC,0x7,0x1,0x3,0x5,0x0,0x0,0x2,0xD},
	    {0xD,0x1,0xA,0x0,0x5,0x8,0x2,0xE,0x7,0xF,0x9,0xC,0xB,0x3,0x6,0x4},
	    {0xF,0x7,0x6,0xA,0xC,0x4,0x1,0xB,0xE,0x9,0x8,0x0,0x2,0x5,0xD,0x3},
	    {0xF,0x7,0x6,0xA,0xC,0x4,0x1,0xB,0xE,0x9,0x8,0x0,0x2,0x5,0xD,0x3}
	};

    int i;
    for(i=0; i<8; i++){
    	splited32[i] = Sbox[i][splited32[i]];
    }
}

#define DECRYPT 0
#define ENCRYPT 1


/**
 * ГОСТ28147
 * @param   input что расшифровываем\зашифровываем
 * @param   keys  массив ключей из 8 элементов
 * @param   FLAG_CRYPT {{DECRYPT|ENCRYPT}} число для разбивки
 * @returns зашифрованные\расшифрованные данные.
 */
uint64_t crypt(uint64_t input, int32_t *keys, int FLAG_CRYPT){
    uint64_t R = input & 0xFFFFFFFF;
    input = input >> 32;
    uint64_t L = input & 0xFFFFFFFF;

    int i;
    uint64_t temp;
    int *splited32;
	for(i = 0;
		(FLAG_CRYPT == ENCRYPT ? i<24:i<8);
		i++){
  		temp = (R+keys[i%8])%(uint64_t)pow(2.0, 32.0);
  	
  		splited32 = split32bit(temp);
  		rSbox(splited32);
  		temp = join32bit(splited32);

  		temp = rol(temp, 11);

  		temp = temp ^ L;

  		L = R;
  		R = temp;
    }

	for(i = FLAG_CRYPT == ENCRYPT ? 7: 23;
		(FLAG_CRYPT == ENCRYPT ? i>=0:i>=0);
		i--){
  		temp = (R+keys[i%8])%(uint64_t)pow(2.0, 32.0);

  		splited32 = split32bit(temp);
  		rSbox(splited32);
  		temp = join32bit(splited32);

  		temp = rol(temp, 11);
  		temp = temp ^ L;

  		L = R;
  		R = temp;

    }

    temp = L;
    L = R;
    R = temp;
    

    return L<<32|R;
}

int main(){

	int32_t *key = keygen();

	char str[] = "test";
	int64_t intString = str2int(str);
	printf("str2int: %"PRIu64"\n", intString);

	uint64_t eCrypt = crypt(intString, key, ENCRYPT);
	printf("result encode: %"PRIu64"\n", eCrypt);

	uint64_t dCrypt = crypt(eCrypt, key, DECRYPT);
	printf("result decode: %"PRIu64"\n", dCrypt);

	char *strInteger = int2str(dCrypt);
	printf("int2str: %s\n", strInteger);



#ifdef DEBUG //gcc gost.cpp -D DEBUG -o gost
	/*
	* ТЕСТИМ ФУНКЦИИ str2int и int2str
	*/
	char strings[8][9] = {{0}};
	strcpy(strings[0], "testing!");
	strcpy(strings[1], "cats");
	strcpy(strings[2], "eat");
	strcpy(strings[3], "mice");
	strcpy(strings[4], "somebody");
	strcpy(strings[5], "!@#$)^*(");
	strcpy(strings[6], "12345678");
	strcpy(strings[7], "gap gap");
	int k;
	int64_t integerStr;
	char newStr[8];
	for(k=0; k<8; k++){
		integerStr = str2int(strings[k]);
		strcpy(newStr, int2str(integerStr));
		if(strcmp(newStr, strings[k]) != 0){
			printf("FAILED: тест для str2int и int2str\n");
			printf("	%s\n", strings[k]);
			break;
		}
	}

	/*
	*ТЕСТИМ ЦИКЛИЧЕСКИЙ СДВИГ
	*/
	int32_t input[8] = {
		0b1111100100100101101110101110111,
		0b1100100100101101110101110111,
		0b100101101110101110111,
		0b10010010010110111010111011,
		0b000100100000000001110111,
		0b0000101110111,
		0b0000010001101110101110111,
		0b0000000010010000001000101110111
	};
	int32_t results[8] = {
		0b10010110111010111011101111100100,
		0b10010110111010111011100001100100,
		0b10010110111010111011100000000000,
		0b01001011011101011101100000010010,
		0b10010000000000111011100000000000,
		0b10111011100000000000,
		0b1000110111010111011100000000000,
		0b01000000100010111011100000000010
	};
	int32_t result;
	for(k=0; k<8; k++){
		result = rol(input[k], 11);
		// printf("%"PRIu32" ||%"PRIu32" == %"PRIu32"\n", input[k], result, results[k]);
		if(result != results[k]){
			printf("FAILED: тест для rol\n");
			printf("	%"PRIu32"\n", input[k]);
			break;
		}
	}

	/*
	*	я заебался маяться хуйней. Буду надяться, что регрессионые баги тут не возникнут.
	*/
#endif
    
    return 0;
}