#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "list.h"
#include "hash.h"
#include "bitmap.h"
#include "debug.h"
#include "hex_dump.h"
#include "limits.h"
#include "round.h"

struct list_item {
	struct list_elem elem;
	int data;
};

struct hash_item {
	struct hash_elem elem;
	int data;
};

char listName[10][20];
struct list listHeader[10];
int listSize;

char hashName[10][20];
struct hash hashHeader[10];
int hashSize;

char bitmapName[10][20];
struct bitmap *bitHeader[10];
int bitmapSize;

void sepCmdLine(char *cmdLine, char *cmd, char *args);
int searchList(char *name);
int searchHash(char *name);
int searchBitmap(char *name);
void dumpList(int listIdx);
void dumpHash(int hashIdx);
void deleteList(int listIdx);
void deleteHash(int hashIdx);

bool listLessFunc(const struct list_elem *a, const struct list_elem *b, void *aux);
void listInsert(int listIdx, int idx, int val);
void listInsertOrdered(int listIdx, int val);
void listPushFront(int listIdx, int val);
void listPushBack(int listIdx, int val);
void listRemove(int listIdx, int val);
void listPopFront(int listIdx);
void listPopBack(int listIdx);
void listFront(int listIdx);
void listBack(int listIdx);
void list_swap(struct list_elem *a, struct list_elem *b);
void list_shuffle(struct list *list);
void listSplice(int list1, int splitPoint, int list2, int start, int end);
void listUnique(int listIdx1, int listIdx2);
void listMax(int listIdx);
void listMin(int listIdx);
void listHeaderSize(int listIdx);

unsigned hashHashFunc(const struct hash_elem *e, void *aux);
bool hashLessFunc(const struct hash_elem *a, const struct hash_elem *b, void *aux);
int searchHash(char *name);
void hashDestroyer(struct hash_elem *e, void *aux);
void dumpHash(int hashIdx);
void hashInsert(int tableIdx, int val);
void hashDelete(int tableIdx, int val);
void hashFind(int tableIdx, int val);
void hashReplace(int tableIdx, int val);

struct bitmap *bitmap_expand(struct bitmap *bitmap, int size);
void hashSquare(struct hash_elem *e, void *aux);
void hashTriple(struct hash_elem *e, void *aux);

