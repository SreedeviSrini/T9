#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXKEYS 9
#define MAXWORDLEN 100
#define MAXBUFLEN 256

#define TRUE 1
#define FALSE 0

char wordlist[MAXBUFLEN];
unsigned int wi = 0;
char noword[] = "noword";

char loadedwords[MAXBUFLEN];

typedef struct listNode
{
	struct listNode* next;
	char *cptr;
}listNode;
typedef struct treeNode
{
	struct 	treeNode* tptr[MAXKEYS];
	listNode* lptr;
}treeNode;
treeNode *head = NULL;
treeNode *curPtr = NULL;
treeNode *loadPtr = NULL;

char word[MAXWORDLEN];
void DisplayWords(treeNode* tNode);
void addWordToTree(treeNode* tptr, char* word);

int getIndex(char c)
{
	int index = 0;
	if ( c >= 'A' && c <= 'R') 
		return ((c - 'A')/3);

	if (c >= 'a' && c <= 'r')
		return ((c - 'a')/3);

	switch (c)
	{
		case 't':
		case 'T':
		case 'u':
		case 'U':
			index = 6;
			break;
		case 'w':
		case 'W':
		case 'x':
		case 'X':
			index = 7;
			break;
		case '\'':
		case '-':
			index = 8;
			break;
	}
	if ( index )
		return index;
	if ( (c >= 's') && (c <= 'z') )
		return (( c - 'a')/3 - 1);
	if ( ( c >= 'S') && (c <= 'Z'))
		return (( c - 'A')/3 - 1);
	return -1;
}

#if STAND_ALONE
void WriteToUserFile(const char* fName,char* word)
{

	FILE* fp=fopen(fName, "w+");
	char commaword[] =",";
	if (!fp)
	{
		printf("\n User Dictionary File %s could not be opened!",fName);
		return;
	}
	fwrite(word, 1, strlen(word), fp);
	fwrite(commaword,1, strlen(commaword),fp);
	fclose(fp);

}
#endif
int putWordToList(treeNode* tempPtr, char *word, int append)
{
	if (!tempPtr->lptr)
	{
		tempPtr->lptr = (listNode*)calloc(1,sizeof(listNode));
		tempPtr->lptr->cptr = (char*) calloc(1,(strlen(word)+1));
		strncpy(tempPtr->lptr->cptr,word,strlen(word)+1);
		tempPtr->lptr->next = NULL;
	}
	else
	{
		// Create node for holding the word
		listNode* temp = (listNode*)calloc(1,sizeof(listNode));
		temp->cptr = (char*) calloc(1,strlen(word)+1);
		strncpy(temp->cptr,word,strlen(word)+1);
	
		// Add to the list	
		listNode* templPtr =tempPtr->lptr;
		if (append)
		{
			// Traverse to the end of the list
			while (templPtr->next)
				templPtr = templPtr->next;
			templPtr->next = temp;
		}
		else
		{
			temp->next = templPtr;
			tempPtr->lptr = temp;
		}
	}
	return 0;
}
void addWord(const char *word)
{
	addWordToTree(head,(char*)word);
}
void addWordToTree(treeNode* tptr, char* word)
{
	unsigned int i = 0; 
	int index = 0;
	if (!tptr && !word)
		return;

	for ( i = 0; i < strlen(word); i++ )
	{
		index = getIndex(word[i]);
		if ( index == -1 ) 
			return; /* Invalid Word */

		if ( index <= MAXKEYS )
		{
			if(!tptr->tptr[index])
			{
				tptr->tptr[index] = (treeNode*)calloc(1,sizeof(treeNode));
			}
			tptr = tptr->tptr[index];
		}
	}
	if (tptr)
	{
		putWordToList(tptr, word, FALSE);
	}
#if STAND_ALONE
	// Write to user.csv
	WriteToUserFile("user.csv",word);
#endif
	
}
void ResetCurPtr()
{
	curPtr = head;
}

int loadWords(const char* buf, unsigned int len)
{

	unsigned int i = 0;
	int index = 0;


	if (!head)
	{
		//allocate it first
		head = (treeNode*)calloc(1,sizeof(struct treeNode));
		loadPtr = head;
	}
	if (!loadPtr)
		return -1;
	while( i < len )
	{
			if ( buf[i] == ',' )
			{
				// end of word, Store it now
				word[wi] = '\0';
				putWordToList(loadPtr, word,TRUE);

				//reset head now
				loadPtr = head;
				memset(word, 0,MAXWORDLEN);
				wi = 0;
				
				i++;
				continue;
			}
			if(buf[i] == '\n' || buf[i] == '\r')
			{
				i++;
				continue;
			}

			if ( buf[i] == '\0')
			{
				//let the word be completed in the next file read, just break
				// going to read the next buffer, start index from 0
				i = 0;
				break;
			}

			word[wi] = buf[i];
			index = getIndex(buf[i]);
			if ( index == -1 )
			{
				// Discard this word with unknown letter
				// Get to the next word
				while (buf[i++] != ',');
				continue;
			}

			if ( index <= MAXKEYS )
			{
				if(!loadPtr->tptr[index])
				{
					loadPtr->tptr[index] = (treeNode*)calloc(1,sizeof(treeNode));

				}
				loadPtr = loadPtr->tptr[index];
				i++;
		        wi++;	
			}
	}
	return 0;
}




void DisplayWords(treeNode* tNode)
{
	if (!tNode)
	{
		return;
	}
	else
	{
		listNode* tempPtr = tNode->lptr;
		while ( tempPtr )
		{

			if ( tempPtr->cptr )
			{
				printf("\nWord: %s",tempPtr->cptr);
			}
			else
			{
				printf("\nNo Word");
			}
			tempPtr = tempPtr->next;
		}
	}
	int i = 0;
	for (i = 0; i < MAXKEYS; i++ )
	{
		if ( tNode->tptr[i] )
		{
			DisplayWords(tNode->tptr[i]);
		}
	}
	return;
}

int isValidInput(char letter)
{
	if ( letter <= '0' || letter >'9')
		return FALSE;
	return TRUE;
}

int isValidWord(char* word)
{
	unsigned int i =0;
	for (i = 0; i < strlen(word);i++)
		if (!isValidInput(word[i]))
			return FALSE;
	return TRUE;
}

void updateList(const char *word)
{
	listNode *startNode, *currNode, *prevNode;
	if (!head)
	{
		return;	
	}
	if (!curPtr)
	{
		return;	
	}
	if ( curPtr && curPtr->lptr )
	{
		listNode* ltempPtr = curPtr->lptr;
		startNode = ltempPtr;
		currNode = ltempPtr;
		while(curPtr->lptr && curPtr->lptr->cptr)
		{
			if(!strncmp(curPtr->lptr->cptr,word,strlen(word)))
			{
				//If first node itself matches
				if(startNode == curPtr->lptr)
				{
					// do nothing just return
					return;
				}
				else
				{
					prevNode->next = currNode->next;
					currNode->next = startNode;
					return;
				}
			}
			//currNode is prevNode before moving to next
			prevNode = currNode;
			curPtr->lptr = curPtr->lptr->next;
			currNode = curPtr->lptr; 
		}
	}
	else
	{
		return;	
	}
}
char* GetPrediction(const char* word)
{
	unsigned int i =0;
	memset(wordlist,0,MAXBUFLEN);

	if (!head)
		return NULL;	
	if (!curPtr)
		curPtr = head;
	for( i = 0; i < strlen(word); i++)
	{
		if (curPtr && curPtr->tptr[word[i] - '1' - 1])
		{
			curPtr	= curPtr->tptr[word[i] - '1' - 1];
		}
		else
		{
			return noword;	
		}
	}

	if ( curPtr && curPtr->lptr )
	{
		listNode* ltempPtr = curPtr->lptr;
		while(ltempPtr && ltempPtr->cptr)
		{
			strcat(wordlist,ltempPtr->cptr);
			strcat(wordlist," ");
			ltempPtr = ltempPtr->next;
		}
	}
	else
	{
		return noword;	
	}

        return wordlist;
}

void dumpDict()
{
	DisplayWords(head);
}

#if STAND_ALONE

int printUserInterface()
{
	char word[MAXWORDLEN] = {0};
	int ret = TRUE;
	printf("\n Enter word:\n");
	printf(" 1     2      3\n");
	printf("      abc    def\n");
	printf(" 4     5      6\n");
	printf("ghi   jkl    mno\n");
	printf(" 7     8      9\n");
	printf("pqrs  tuv    wxyz\n");

	//letter = getchar();
	//printf("You entered: %c", letter);
	scanf("%s",word);
	ret = isValidWord(word);
        if (!strcmp(word,"1"))
	{
		ResetCurPtr();
		return ret;
	}
	if (ret)
		printf("\n Strings are %s",GetPrediction((const char*)word));
	printf("\n The word entered is %s", ret? "Valid":"not valid Exiting...");
	return ret;
}

int readDictionaryFromCsv(const char *fName)
{
	FILE* fp=fopen(fName, "r");
	if (!fp || feof(fp))
	{
		printf("\n Dictionary File Empty!");
		return -1;
	}

	// Read File in a loop
	char buf[MAXBUFLEN];
	memset(buf, 0,MAXBUFLEN);

	while ( fgets(buf, MAXBUFLEN, fp))
	{
		// put all the words to the tree
    		loadWords(buf, MAXBUFLEN);             
	} 
	fclose(fp);
	return 0;
}

int initDictionary()
{
	int ret = TRUE;
	if ( !head )
	{
	printf("Welcome to T9 word Predictor!!\n");
	readDictionaryFromCsv("user.csv");
	readDictionaryFromCsv("dict.csv");
	addWordToTree(head,"Sreedevi");
	ret = FALSE;
	}
	return ret;
}

int main()
{
	int ret = TRUE;
	initDictionary();
	DisplayWords(head);

	while(ret)
	{
	ret = printUserInterface();
	}
}
#endif
