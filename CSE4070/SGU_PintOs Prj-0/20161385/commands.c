#include "commands.h"

/*
 * cmdLine으로부터 받은 명령을 parsing해서 cmd와 args로 분리
 * cmd에는 명령에 관한 line이 들어가고 args에는 나머지 명령들이 들어감
 */
void sepCmdLine(char *cmdLine, char *cmd, char *args) {
	int i = 0, j = 0;
	cmdLine[strlen(cmdLine) - 1] = '\0';

	while (cmdLine[i] == ' ')
		i++;

	while (cmdLine[i] != ' ' && cmdLine[i] != '\0') {
		cmd[j] = cmdLine[i];
		i++, j++;
	}

	if (cmdLine[i] != '\0') {
		j = 0;
		while (cmdLine[i] == ' ' || cmdLine[i] == '\t')
			i++;
		while (cmdLine[i] != '\0') {
			args[j] = cmdLine[i];
			i++, j++;
		}
	}
}

// ============================================//
// 								list관련 함수들 구현						 //
// ============================================//

/* 
 * list에서 특정이름을 찾아본다. 
 */
int searchList(char *name) {
	int i;
	for(i = 0; i < listSize; i++) {
		if(!strcmp(listName[i], name))
			return i;
	}
	// 만일 원하는 이름을 못찾을 경우 listSize만큼 return
	return i;
}

/* 
 * 특정 list자료구조의 dumpdata를 실행 
 */
void dumpList(int listIdx) {
	struct list_item *item = NULL;
	struct list_elem *iter = NULL;
	// list가 empty일 경우 아무 것도 출력 안하고 리턴
	if(list_empty(&listHeader[listIdx])) {
		return;
	}
	for(iter = list_front(&listHeader[listIdx]); iter != &(listHeader[listIdx].tail); iter = iter->next) {
		item = list_entry(iter, struct list_item, elem);
		printf("%d ", item->data);
	}
		printf("\n");
}

/* 
 * 특정 list자료구조의 node들의 동적할당을 해제한다
 */
void deleteList(int listIdx) {
	// 할당해제를 위한 iterator
	struct list_item *delNode;
	struct list_elem *iter;
	// list가 empty일 경우 아무 것도 출력 안하고 리턴
	if(list_empty(&listHeader[listIdx])) {
		return;
	}

	// list_push_back을 하면서 동적할당해제
	while(true) {
		// 만일 listHeader[listIdx]에 node가 없으면 break
		if(list_empty(&listHeader[listIdx])) {
			break;
		}
		// 해제할 node가 있는 경우 동적해제를 진행한다
		iter = list_pop_back(&listHeader[listIdx]);
		// list_elem을 감싸는 list_item을 동적해제해야한다.
		delNode = list_entry(iter, struct list_item, elem);
		free(delNode);
	}
	// 할당된 자료구조가 하나 줄었으므로 사이즈를 감소
	listSize--;
}

/* 
 * list의 node에서 data의 대소비교를 한다
 * 만일 a < b이면 true를 리턴, 반대라면 false를 리턴
 */
bool listLessFunc(const struct list_elem *a, const struct list_elem *b, void *aux) {
	struct list_item *aItem = list_entry(a, struct list_item, elem);
	struct list_item *bItem = list_entry(b, struct list_item, elem);

	if(aItem->data < bItem->data)
		return true;
	else
		return false;
}

/* 
 * list_insert함수의 wrapper
 * 특정 list에 새로운 node를 insert
 */
void listInsert(int listIdx, int insertIdx, int val) {
	int i;
	struct list_elem *iter = NULL;
	struct list_item *item = (struct list_item*)malloc(sizeof(struct list_item));
	// list_item 타입의 *item을 동적할당한 후 값을 준다.
	item->elem.prev = NULL;
	item->elem.next = NULL;
	item->data = val;
	
	// list_insert명령을 수행한다.
	iter = list_begin(&listHeader[listIdx]);
	for(i = 0; i < insertIdx; i++) {
		iter = list_next(iter);
	}
	list_insert(iter, &(item->elem));
}

/* 
 * list_insert_order함수의 wrapper
 * 특정 list에 새로운 node를 오름차순으로 insert
 * 단, 이 함수만 이용해야 모든 원소가 ordered상태로 유지됨
 */
void listInsertOrdered(int listIdx, int val) {
	struct list_item *item = (struct list_item*)malloc(sizeof(struct list_item));
	item->data = val;
	// list_insert_ordered명령을 수행한다 이때 listLessFunc을 이용해서 크기 비교
	list_insert_ordered(&listHeader[listIdx], &(item->elem), listLessFunc, NULL);
}

/* 
 * list_push_front함수의 wrapper
 * 특정 list에 새로운 node를 front에 insert한다
 */
void listPushFront(int listIdx, int val) {
	struct list_item *item = (struct list_item*)malloc(sizeof(struct list_item));
	item->data = val;
	list_push_front(&listHeader[listIdx], &(item->elem));
}

/* 
 * list_push_back함수의 wrapper
 * 특정 list에 새로운 node를 back에 insert한다
 */
void listPushBack(int listIdx, int val) {
	struct list_item *item = (struct list_item*)malloc(sizeof(struct list_item));
	item->data = val;
	list_push_back(&listHeader[listIdx], &(item->elem));
}

/* 
 * list_remove함수의 wrapper
 * 특정 list의 index에 해당하는 node를 삭제한다
 */
void listRemove(int listIdx, int findIdx) {
	struct list_elem *iter;
	// 만일 list가 초기화만 되어있고 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;

	iter = list_begin(&listHeader[listIdx]);
	// index에 해당하는 node를 찾는다
	for(int i = 0; i < findIdx; i++) {
		iter = list_next(iter);
	}
	iter = list_remove(iter);
}

/* 
 * list_pop_front함수의 wrapper
 * 특정 list의 frontNode를 삭제한다
 */
void listPopFront(int listIdx) {
	struct list_elem *front;
	// 만일 list가 초기화만 되어있고 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	
	front = list_begin(&listHeader[listIdx]);
	// front값을 제거
	front = list_remove(front);
}

/* 
 * list_pop_back함수의 wrapper
 * 특정 list의 backNode를 삭제한다
 */
void listPopBack(int listIdx) {
	struct list_elem *back;
	// 만일 list가 초기화만 되어있고 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	
	back = list_back(&listHeader[listIdx]);
	// back값을 제거
	back = list_remove(back);
}

/*
 * list_max함수의 wrapper
 * 특정 list를 순회하면서 list에서 data의 최대값을 출력
 */
void listMax(int listIdx) {
	struct list_item *maxItem;
	struct list_elem *maxElem;
// 만일 list가 초기화만 되어있고 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	
	maxElem = list_max(&listHeader[listIdx], listLessFunc, NULL);
	maxItem = list_entry(maxElem, struct list_item, elem);
	printf("%d\n", maxItem->data);
}

/*
 * list_min함수의 wrapper
 * 특정 list를 순회하면서 list에서 data의 최소값을 출력
 */
void listMin(int listIdx) {
	struct list_item *minItem;
	struct list_elem *minElem;
// 만일 list가 초기화만 되어있고 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	
	minElem = list_min(&listHeader[listIdx], listLessFunc, NULL);
	minItem = list_entry(minElem, struct list_item, elem);
	printf("%d\n", minItem->data);
}

/*
 * 특정 list를 순회하면서 list에서 data의 size를 count하고 출력
 */
void listHeaderSize(int listIdx) {
	int listSize = 0;
	struct list_elem *iter, *end;
	if(list_empty(&listHeader[listIdx]))
		return;

	iter = list_begin(&listHeader[listIdx]);
	end = list_end(&listHeader[listIdx]);
	while(iter != end) {
		listSize++;
		iter = list_next(iter);
	}
	printf("%d\n", listSize);
}

/*
 * list_front함수의 wrapper
 * 특정 list의 맨 앞의 node data를 출력한다
 */
void listFront(int listIdx) {
	struct list_item *frontElem;
	struct list_elem *front;
	// 특정 list가 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	// list에 node가 있는 경우
	front = list_front(&listHeader[listIdx]);
	frontElem = list_entry(front, struct list_item, elem);
	printf("%d\n", frontElem->data);
}

/*
 * list_front함수의 wrapper
 * 특정 list의 맨 뒤의 node data를 출력한다
 */
void listBack(int listIdx) {
	struct list_item *backElem;
	struct list_elem *back;
	// 특정 list가 비어있는 경우
	if(list_empty(&listHeader[listIdx]))
		return;
	// list에 node가 있는 경우
	back = list_back(&listHeader[listIdx]);
	backElem = list_entry(back, struct list_item, elem);
	printf("%d\n", backElem->data);
}

/*
 * 특정 list의 index를 통해 a와 b를 찾아 swap한다
 */
void list_swap(struct list_elem *a, struct list_elem *b) {
	struct list_item *aItem, *bItem;
	int tmpData;

	aItem = list_entry(a, struct list_item, elem);
	bItem = list_entry(b, struct list_item, elem);

	tmpData = aItem->data;
	aItem->data = bItem->data;
	bItem->data = tmpData;
}

/*
 * list의 index를 랜덤으로 받아 무작위 셔플을 한다
 */
void list_shuffle(struct list *list) {
	srand((unsigned int)time(NULL));
	int counter = 0;
	int idx1, idx2;
	struct list_elem *iter = list_begin(list);
	struct list_elem *a, *b;

	while(iter != list_end(list)) {
		counter++;
		iter = list_next(iter);
	}

	// rand함수를 통해 0부터 list의 사이즈보다 작은 수를 무작위로 반환
	idx1 = rand() % counter;
	idx2 = rand() % counter;
	
	a = list_begin(list);
	b = list_begin(list);
	// index[0]에 해당하는 node를 찾는다
	for(int i = 0; i < idx1; i++) {
		a = list_next(a);
	}
	// index[1]에 해당하는 node를 찾는다
	for(int i = 0; i < idx2; i++) {
		b = list_next(b);
	}
	// index에 해당하는 두 node를 찾으면 swap을 진행한다
	list_swap(a, b);
}

/*
 * list_splice의 wrapper함수
 * 특정 list에 다른 list의 원소를 삽입한다
 */
void listSplice(int list1, int splitPoint, int list2, int start, int end) {
	int i;
	struct list_elem *iter1, *iter2, *iter3;

	iter1 = list_begin(&listHeader[list1]);
	iter2 = list_begin(&listHeader[list2]);
	iter3 = list_begin(&listHeader[list2]);

	for(i = 0; i < splitPoint; i++)
		iter1 = list_next(iter1);
	for(i = 0; i < start; i++)
		iter2 = list_next(iter2);
	for(i = 0; i < end - 1; i++)
		iter3 = list_next(iter3);

	iter2->prev->next = iter3->next;
	iter3->next->prev = iter2->prev;

	iter1->prev->next = iter2;
	iter2->prev = iter1->prev;
	iter1->prev = iter3;
	iter3->next = iter1;
}

/*
 * list_unique의 wrapper함수
 * 중복 data를 다른 list에 옮기거나 제거한다
 */
void listUnique(int listIdx1, int listIdx2) {
	if(listIdx2 == -1) {
		list_unique(&listHeader[listIdx1], NULL, listLessFunc, NULL);
	} else {
		list_unique(&listHeader[listIdx1], &listHeader[listIdx2], listLessFunc, NULL);
	}
}

//============================================//
// 					 hashtable관련 함수들 구현				    //
//============================================//

/* 
 * hash함수를 wrapping하고 있는 함수
 * 또한 hash_elem을 converting해서 data값을 가져올 수 있다
 * hashTable의 bucket index를 result로 return한다
 */
unsigned hashHashFunc(const struct hash_elem *e, void *aux) {
	struct hash_item *item;
	item = hash_entry(e, struct hash_item, elem);

	int value = item->data;
	unsigned result = hash_int(value);

	return result;
}

/* 
 * hash의 node에서 data의 대소비교를 한다
 * 만일 a < b이면 true를 리턴, 반대라면 false를 리턴
 */
bool hashLessFunc(const struct hash_elem *a, const struct hash_elem *b, void *aux){
	struct hash_item *aItem = hash_entry(a, struct hash_item, elem); 
	struct hash_item *bItem = hash_entry(b, struct hash_item, elem);

	if (aItem->data < bItem->data)
		return true;
	else
		return false;
}

/* 
 * hashTable에서 특정이름을 찾아본다
 */
int searchHash(char *name) {
	int i;
	for(i = 0; i < hashSize; i++) {
		if(!strcmp(hashName[i], name))
			return i;
	}
	// 만일 원하는 이름을 못찾을 경우 hashSize만큼 return
	return i;
}

/* 
 * 특정 hash_destroy의 action에 등록할 함수 
 * 자료구조의 node들의 동적할당을 해제한다
 */
void hashDestroyer(struct hash_elem *e, void *aux) {
	struct hash_item *deleteItem;
	deleteItem = hash_entry(e, struct hash_item, elem);
	free(deleteItem);
}

/* 
 * hash자료구조의 bucketTable에 저장되어 있는 데이터를 출력한다
 */
void dumpHash(int tableIdx) {
	struct hash_item *item;
	struct hash_iterator iter;

	if(hash_empty(&hashHeader[tableIdx]))
		return;

	hash_first(&iter, &hashHeader[tableIdx]);
	while(true) {
		if(!hash_next(&iter))
			break;
		item = hash_entry(hash_cur(&iter), struct hash_item, elem);
		printf("%d ", item->data);
	}
	printf("\n");
}

/* 
 * 특정 hashTable의 bucket에 node를 insert한다
 */
void hashInsert(int tableIdx, int val) {
	struct hash_item *item = (struct hash_item *)malloc(sizeof(struct hash_item));
	item->data = val;
	hash_insert(&hashHeader[tableIdx], &(item->elem));
}

/* 
 * 특정 hashTable의 bucket에서 특정 값을 찾아서 삭제한다
 */
void hashDelete(int tableIdx, int val) {
	struct hash_item *delItem;
	struct hash_elem *delElem;

	if(hash_empty(&hashHeader[tableIdx]))
		return;
	delItem = (struct hash_item*)malloc(sizeof(struct hash_item));
	delItem->data = val;
	delElem = hash_find(&hashHeader[tableIdx], &(delItem->elem));
	
	if(delElem != NULL) {
		hash_delete(&hashHeader[tableIdx], delElem);
	}
}

/* 
 * 특정 hashTable의 bucket에서 찾고자하는 데이터가 있는지 확인한다
 */
void hashFind(int tableIdx, int val) {
	struct hash_item *findItem, *findResult;
	struct hash_elem *findElem;

	if(hash_empty(&hashHeader[tableIdx]))
		return;
	findItem = (struct hash_item*)malloc(sizeof(struct hash_item));
	findItem->data = val;

	findElem = hash_find(&hashHeader[tableIdx], &(findItem->elem));
	if(findElem != NULL) {
		findResult = hash_entry(findElem, struct hash_item, elem);
		printf("%d\n", findResult->data);
	}
}

/* 
 * 특정 hashTable의 bucket에서 찾고자하는 데이터가 있는지 확인 후 데이터를 replace한다
 */
void hashReplace(int tableIdx, int val) {
	struct hash_item *replaceItem;

	replaceItem = (struct hash_item*)malloc(sizeof(struct hash_item));
	replaceItem->data = val;

	hash_replace(&hashHeader[tableIdx], &(replaceItem->elem));
}

/* 
 * hash_apply함수에 등록해서 사용하는 함수
 * hash_action_func에 의해 치환되며,
 * hash에 저장된 모든 data를 제곱한다
 */
void hashSquare(struct hash_elem *e, void *aux) {
	struct hash_item *squareItem;
	int squareData;

	squareItem = hash_entry(e, struct hash_item, elem);
	squareData = squareItem->data;
	squareItem->data = squareData*squareData;
}

/*
 * hash_apply함수에 등록해서 사용하는 함수
 * hash_action_func에 의해 치환되며,
 * hash에 저장된 모든 data를 세제곱한다
 */
void hashTriple(struct hash_elem *e, void *aux) {
	struct hash_item *tripleItem;
	int tripleData;

	tripleItem = hash_entry(e, struct hash_item, elem);
	tripleData = tripleItem->data;
	tripleItem->data = tripleData * tripleData * tripleData;
}

/* 
 * integer값을 받아서 자신만의 hash함수를 만든다
 */
unsigned hash_int_2(int i) {
	double doubleTmp;
	int intTmp;

	doubleTmp = (double)i * 0.839623832;
	intTmp = (int)doubleTmp;
	if(intTmp < 0)
		intTmp--;
	doubleTmp -= intTmp;
	doubleTmp *= 7;
	i = (int)doubleTmp;

	return hash_bytes(&i, sizeof(i));
}
//============================================//
// 					   bitmap관련 함수들 구현				   		//
//============================================//

/* 
 * bitmap에서 특정이름을 찾아본다
 */
int searchBitmap(char *name) {
	int i;
	for(i = 0; i < hashSize; i++) {
		if(!strcmp(bitmapName[i], name))
			return i;
	}
	// 만일 원하는 이름을 못찾을 경우 bitmapSize만큼 return
	return i;
}

/* 
 * 특정 bitmap의 크기를 주어진 argument만큼 확장한다
 */
struct bitmap *bitmap_expand(struct bitmap *bitmap, int size) {
	int i;
	size_t beforeSize, expendSize;
	struct bitmap *expendBitmap;
	bool bitChker;

	beforeSize = bitmap_size(bitmap);
	expendSize = beforeSize + size;
	expendBitmap = bitmap_create(expendSize);

	for(i = 0; i < beforeSize; i++) {
		bitChker = bitmap_test(bitmap, i);
		bitmap_set(expendBitmap, i, bitChker);
	}
	return expendBitmap;
}