#include "commands.h"

int main() {
	// 표준입력에서 받은 명령을 저장할 변수
	char cmdLine[100], cmd[50], args[50];
	char arg1[50], arg2[50], arg3[50];
	int idx[3], value, argCount, searchIdx, searchIdx2, tmp;
	size_t scanResult;
	bool boolValue, boolResult;
	struct list_item *item;
	struct list_elem *iter, *end;
	struct list_elem *a, *b;

	while(true) {
		// 문자열 저장에 사용했던 변수를 모두 초기화
		memset(cmdLine, '\0', sizeof(cmdLine));
		memset(cmd, '\0', sizeof(cmd));
		memset(args, '\0', sizeof(args));
		memset(arg1, '\0', sizeof(arg1));
		memset(arg2, '\0', sizeof(arg2));
		memset(arg3, '\0', sizeof(arg3));
		boolValue = false, boolResult = false;
		item = NULL, iter = NULL, end = NULL, a = NULL, b = NULL;
		// 표준입력으로부터 명령을 입력받는다
		fgets(cmdLine, sizeof(cmdLine), stdin);
		// 명령어 parsing해서 cmd와 args에 나머지 저장
		sepCmdLine(cmdLine, cmd, args);
		// list | hashtable | bitmap
		// 자료구조들이 공통으로 사용하는 명령어를 먼저구현
		// create, delete, dumpdata
		if(!strcmp(cmd, "create")) {
			// args에 있는 string을 parsing해서 argument별로 분리
			argCount = sscanf(args ,"%s %s %d", arg1, arg2, &value);
			// 할당된 argument가 2개라면 list || hashtable
			// 할당된 argument가 3개라면 bitmap
			if(argCount == 2) {
				if(!strcmp(arg1, "list")) {
					// 만들 list의 이름을 저장하고 list를 초기화한다
					strcpy(listName[listSize], arg2);
					list_init(&listHeader[listSize]);
					listSize++;
				} else if(!strcmp(arg1, "hashtable")) {
					// 만들 hashTable의 이름을 저장하고 hash를 초기화한다
					strcpy(hashName[hashSize], arg2);
					hash_init(&hashHeader[hashSize], hashHashFunc, hashLessFunc, NULL);
					hashSize++;
				}
			} else if(argCount == 3) {
				if(!strcmp(arg1, "bitmap")) {
					// value의 크기만큼 bitmap을 만들고 초기화한다
					strcpy(bitmapName[bitmapSize], arg2);
					bitHeader[bitmapSize]= bitmap_create(value);
					bitmapSize++;
				}
			}
		} else if(!strcmp(cmd, "delete")) {
			// 만일 argCount값이 1이 아니면 잘못 입력
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// delete하려는 자료구조가 어떤 것인지 이름을 가지고 찾는다
				// list의 이름저장소에 대해서 먼저 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// deleteList함수로 node에 할당된 메모리 모두 해제
					deleteList(searchIdx);
					continue;
				}
				// hashTable의 이름저장소에 대해서 순회
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					// hashTable의 자료구조에서 이름을 찾은 경우
					// deleteHash함수로 node에 할당된 메모리 모두 해제
					hash_destroy(&hashHeader[searchIdx], hashDestroyer);
					hashSize--;
					continue;
				}
				// bitmap의 이름저장소에 대해서 순회
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					bitmap_destroy(bitHeader[searchIdx]);
					bitmapSize--;
				}
			}
		} else if(!strcmp(cmd, "dumpdata")) {
			// argument를 parsing해서 dump하려는 대상을 구한다
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// dump하려는 자료구조가 어떤 것인지 이름을 가지고 찾는다
				// list의 이름저장소에 대해서 먼저 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// dumpList를 통해 해당 자료구조에 저장된 값을 출력
					dumpList(searchIdx);
					continue;
				}
				// hashTable의 이름저장소에 대해서 순회
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					// hashTable 자료구조에서 이름을 찾은 경우
					// dumpHash를 통해 해당 자료구조에 저장된 값을 출력
					dumpHash(searchIdx);
					continue;
				}
				// bitmap의 이름저장소에 대해서 순회
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// bitmap자료구조에서 이름을 찾은 경우
					// bitmap_size와 bitmap_test를 이용해 해당 자료구조에 저장된 값을 출력
					tmp = bitmap_size(bitHeader[searchIdx]);
					for(int i = 0; i < tmp; i++) {
						printf("%d", bitmap_test(bitHeader[searchIdx], i));
					}
					printf("\n");
				}
			}
		} else if(!strcmp(cmd, "quit")) {
			// 프로그램을 종료
			// 종료하기 전에 모든 자료구조에 동적할당했던 메모리를 해제해야한다
			exit(0);
		}

		/*
		 * ===============   list관련 함수   ======================
		 */
		else if(!strcmp(cmd, "list_insert")) {
			// list_insert
			argCount = sscanf(args ,"%s %d %d", arg1, &idx[0], &value);
			// argCount가 3이 나오지 않는다면 잘못 입력된 명령이다
			if(argCount == 3) {
				// 저장된 list목록에서 자료구조의 이름을 찾는다
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// list_insert wrapper함수인 listInsert에 값을 전달
					listInsert(searchIdx, idx[0], value);
				}
			}
		} else if(!strcmp(cmd, "list_insert_ordered")) {
			// list_insert_ordered
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수인 listInsertOrdered에 값 전달
					listInsertOrdered(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "list_push_front")) {
			// list_push_front
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수인 listPushFront에 값 전달
					listPushFront(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "list_push_back")) {
			// list_push_back
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수인 listPushBack에 값 전달
					listPushBack(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "list_remove")) {
			// list_remove
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수인 listRemove에 삭제할 index전달
					listRemove(searchIdx, idx[0]);
				}
			}
		} else if(!strcmp(cmd, "list_pop_front")) {
			// list_pop_front
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수에 특정 list의 index값을 전달
					listPopFront(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_pop_back")) {
			// list_pop_back
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수에 특정 list의 index값을 전달
					listPopBack(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_empty")) {
			// list_empty
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					if(list_empty(&listHeader[searchIdx])) {
						// 특정 list가 비어있는 경우
						printf("true\n");
					} else {
						// 특정 list에 node가 있는 경우
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "list_front")) {
			// list_front
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					listFront(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_back")) {
			// list_back
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					listBack(searchIdx);
				}
			}
		}  else if(!strcmp(cmd, "list_size")) {
			// list_size
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					listHeaderSize(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_max")) {
			// list_max
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수에 특정 list의 index값을 전달
					listMax(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_min")) {
			// list_min
			// 만일 argCount값이 1이 아니면 잘못 입력한 것이다
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				// list의 이름저장소 순회
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					// list 자료구조에서 이름을 찾은 경우
					// wrapper함수에 특정 list의 index값을 전달
					listMin(searchIdx);
				}
			}
		} else if(!strcmp(cmd, "list_swap")) {
			// list_swap
			argCount = sscanf(args ,"%s %d %d", arg1, &idx[0], &idx[1]);
			if(argCount == 3) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
						if(list_empty(&listHeader[searchIdx]))
							continue;
						// node searching을 위해 list의 front부터 시작한다
						a = list_begin(&listHeader[searchIdx]);
						b = list_begin(&listHeader[searchIdx]);
						// index[0]에 해당하는 node를 찾는다
						for(int i = 0; i < idx[0]; i++) {
							a = list_next(a);
						}
						// index[1]에 해당하는 node를 찾는다
						for(int i = 0; i < idx[1]; i++) {
							b = list_next(b);
						}
						// index에 해당하는 두 node를 찾으면 swap을 진행한다
						list_swap(a, b);
				}
			}
		} else if(!strcmp(cmd, "list_shuffle")) {
			// list_swap
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
						if(list_empty(&listHeader[searchIdx]))
							continue;
						// index에 해당하는 두 node를 찾으면 swap을 진행한다
						list_shuffle(&listHeader[searchIdx]);
				}
			}
		} else if(!strcmp(cmd, "list_reverse")) {
			// list_reverse
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					if(list_empty(&listHeader[searchIdx]))
						continue;
					list_reverse(&listHeader[searchIdx]);		
				}
			}
		} else if(!strcmp(cmd, "list_sort")) {
			// list_sort
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					if(list_empty(&listHeader[searchIdx]))
						continue;
					list_sort(&listHeader[searchIdx], listLessFunc, NULL);
				}
			}
		} else if(!strcmp(cmd, "list_splice")) {
			// list_splice
			argCount = sscanf(args ,"%s %d %s %d %d", arg1, &idx[0], arg2, &idx[1], &idx[2]);
			if(argCount == 5) {
				searchIdx = searchList(arg1);
				searchIdx2 = searchList(arg2);
				if(searchIdx < listSize && searchIdx2 < listSize) {
					if(list_empty(&listHeader[searchIdx]))
						continue;
					if(list_empty(&listHeader[searchIdx2]))
						continue;
					// list_splice의 wrapper함수
					listSplice(searchIdx, idx[0], searchIdx2, idx[1], idx[2]);
				}
			}
		} else if(!strcmp(cmd, "list_unique")) {
			// list_splice
			argCount = sscanf(args ,"%s %s", arg1, arg2);
			if(argCount == 1) {
				searchIdx = searchList(arg1);
				if(searchIdx < listSize) {
					if(list_empty(&listHeader[searchIdx]))
						continue;
					// list_unique의 wrapper함수
					listUnique(searchIdx, -1);
				}
			} else if(argCount == 2) {
				searchIdx = searchList(arg1);
				searchIdx2 = searchList(arg2);
				if(searchIdx < listSize && searchIdx2 < listSize) {
					if(list_empty(&listHeader[searchIdx]))
						continue;
					// list_unique의 wrapper함수
					listUnique(searchIdx, searchIdx2);
				}
			}
		}
	/*
	 * ===============  hashtable관련 함수  ======================
	 */
		else if(!strcmp(cmd, "hash_insert")) {
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					hashInsert(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "hash_delete")) {
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					hashDelete(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "hash_find")) {
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					hashFind(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "hash_replace")) {
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					hashReplace(searchIdx, value);
				}
			}
		} else if(!strcmp(cmd, "hash_empty")) {
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					if(hash_empty(&hashHeader[searchIdx])) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "hash_size")) {
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					if(hash_empty(&hashHeader[searchIdx]))
						continue;
					printf("%zu\n", hash_size(&hashHeader[searchIdx]));
				}
			}
		} else if(!strcmp(cmd, "hash_clear")) {
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					if(hash_empty(&hashHeader[searchIdx]))
						continue;
					hash_clear(&hashHeader[searchIdx], hashDestroyer);
				}
			}
		} else if(!strcmp(cmd, "hash_apply")) {
			argCount = sscanf(args ,"%s %s", arg1, arg2);
			if(argCount == 2) {
				searchIdx = searchHash(arg1);
				if(searchIdx < hashSize) {
					if(hash_empty(&hashHeader[searchIdx]))
						continue;
					if(!strcmp(arg2, "square")) {
						hash_apply(&hashHeader[searchIdx], hashSquare);
					} else if(!strcmp(arg2, "triple")) {
						hash_apply(&hashHeader[searchIdx], hashTriple);
					}
				}
			}
		}
	/*
	 * ===============  hashtable관련 함수  ======================
	 */
		else if(!strcmp(cmd, "bitmap_mark")) {
			// bitmap_mark
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에 해당하는 비트를 1로 만든다
					bitmap_mark(bitHeader[searchIdx], idx[0]);
				}
			}
		} else if(!strcmp(cmd, "bitmap_set")) {
			// bitmap_set
			argCount = sscanf(args ,"%s %d %s", arg1, &idx[0], arg2);
			if(argCount == 3) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에 해당하는 비트를 boolValue(true or false)로 세팅한다
						if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					bitmap_set(bitHeader[searchIdx], idx[0], boolValue);
				}
			}
		} else if(!strcmp(cmd, "bitmap_set_multiple")) {
			// bitmap_set_multiple
			argCount = sscanf(args ,"%s %d %d %s", arg1, &idx[0], &idx[1], arg2);
			if(argCount == 4) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// arg2의 값이 true인지 false인지 체크 후 함수를 부른다
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					// bitmap이 가진 비트들 중 idx[0]부터 idx[1]개를 세서 boolValue값으로 세팅
					bitmap_set_multiple(bitHeader[searchIdx], idx[0], idx[1], boolValue);
				}
			}
		} else if(!strcmp(cmd, "bitmap_set_all")) {
			// bitmap_set_all
			argCount = sscanf(args ,"%s %s", arg1, arg2);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// arg2의 값이 true인지 false인지 체크 후 함수를 부른다
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					// bitmap이 가진 모든 bit값을 boolValue값으로 세팅
					bitmap_set_all(bitHeader[searchIdx], boolValue);
				}
			}
		} else if(!strcmp(cmd, "bitmap_reset")) {
			// bitmap_reset
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에 해당하는 비트를 0으로 만든다
					bitmap_reset(bitHeader[searchIdx], idx[0]);
				}
			}
		} else if(!strcmp(cmd, "bitmap_flip")) {
			// bitmap_flip
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에 해당하는 비트를 반대값으로 만든다
					bitmap_flip(bitHeader[searchIdx], idx[0]);
				}
			}
		} else if(!strcmp(cmd, "bitmap_scan")) {
			// bitmap_scan
			argCount = sscanf(args ,"%s %d %d %s", arg1, &idx[0], &idx[1], arg2);
			if(argCount == 4) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					scanResult = bitmap_scan(bitHeader[searchIdx], idx[0], idx[1], boolValue);
					printf("%zu\n", scanResult);
				}
			}
		} else if(!strcmp(cmd, "bitmap_scan_and_flip")) {
			// bitmap_scan_and_flip
			argCount = sscanf(args ,"%s %d %d %s", arg1, &idx[0], &idx[1], arg2);
			if(argCount == 4) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					scanResult = bitmap_scan_and_flip(bitHeader[searchIdx], idx[0], idx[1], boolValue);
					printf("%zu\n", scanResult);
				}
			}
		} else if(!strcmp(cmd, "bitmap_count")) {
			// bitmap_size
			argCount = sscanf(args ,"%s %d %d %s", arg1, &idx[0], &idx[1], arg2);
			if(argCount == 4) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					// bitmap에 idx[0]부터 idx[1]개 만큼 비트 수를 세서
					// boolValue에 해당하는 비트 값이 몇 개 있는 지를 리턴한다
					scanResult = bitmap_count(bitHeader[searchIdx], idx[0], idx[1], boolValue);
					printf("%zu\n", scanResult);
				}
			}
		} else if(!strcmp(cmd, "bitmap_none")) {
			// bitmap_none
			argCount = sscanf(args ,"%s %d %d", arg1, &idx[0], &idx[1]);
			if(argCount == 3) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// 범위 내에서 idx[0]에서부터 idx[1]개를 카운트하면서 1값이 없으면
					// true를 리턴하고 1값이 하나라도 있으면 false를 리턴한다
					boolResult = bitmap_none(bitHeader[searchIdx], idx[0], idx[1]);
					if(boolResult) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "bitmap_test")) {
			// bitmap_test
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에 해당하는 비트의 값이 true인지 false인지 체크한다
					boolValue = bitmap_test(bitHeader[searchIdx], idx[0]);
					if(boolValue) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "bitmap_contains")) {
			// bitmap_all
			argCount = sscanf(args ,"%s %d %d %s", arg1, &idx[0], &idx[1], arg2);
			if(argCount == 4) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					if(!strcmp(arg2 ,"true")){
						boolValue = true;
					} else if(!strcmp(arg2, "false")) {
						boolValue = false;
					}
					// 범위 내에 boolValue값이 존재하면 true를 리턴한다
					boolResult = bitmap_contains(bitHeader[searchIdx], idx[0], idx[1], boolValue);
					if(boolResult) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "bitmap_all")) {
			// bitmap_all
			argCount = sscanf(args ,"%s %d %d", arg1, &idx[0], &idx[1]);
			if(argCount == 3) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에서부터 제시한 숫자만큼 bit를 검사한다 만일 모두 true이면 true리턴
					boolValue = bitmap_all(bitHeader[searchIdx], idx[0], idx[1]);
					if(boolValue) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "bitmap_any")) {
			// bitmap_any
			argCount = sscanf(args ,"%s %d %d", arg1, &idx[0], &idx[1]);
			if(argCount == 3) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// index에서부터 제시한 숫자만큼 bit를 검사하고 1이 하나라도 있으면 true리턴
					boolValue = bitmap_any(bitHeader[searchIdx], idx[0], idx[1]);
					if(boolValue) {
						printf("true\n");
					} else {
						printf("false\n");
					}
				}
			}
		} else if(!strcmp(cmd, "bitmap_size")) {
			// bitmap_size
			argCount = sscanf(args ,"%s %d", arg1, &idx[0]);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// bitmap자료구조에 할당된 비트 개수를 리턴한다
					scanResult = bitmap_size(bitHeader[searchIdx]);
					printf("%zu\n", scanResult);
				}
			}
		} else if(!strcmp(cmd, "bitmap_expand")) {
			// bitmap_expand
			argCount = sscanf(args ,"%s %d", arg1, &value);
			if(argCount == 2) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// 특정 bitmap을 주어진 사이즈만큼 확장한다
					bitHeader[searchIdx] = bitmap_expand(bitHeader[searchIdx], value);
				}
			}
		} else if(!strcmp(cmd, "bitmap_dump")) {
			// bitmap_dump
			argCount = sscanf(args ,"%s", arg1);
			if(argCount == 1) {
				searchIdx = searchBitmap(arg1);
				if(searchIdx < bitmapSize) {
					// 특정 bitmap의 값을 16진수로 출력
					bitmap_dump(bitHeader[searchIdx]);
				}
			}
		}
	}
	return 0;
}	
