#include "csapp.h"
#define FDBUFSIZE 1024
//---주식목록 로딩을 위한 BST관련 자료구조와 함수---//
typedef struct _StockItem *pItem;
typedef struct _StockItem
{
    int ID;
    int left_stock;
    int price;
    int readCount;
    sem_t mutex;
    sem_t writer;
    pItem leftChild;
    pItem rightChild;
} StockItem;

pItem treeHead;

void load_stocks();
void making_BST(int ID, int left_stock, int price);
void inOrder(char *stockList, pItem pIter);
void freeTree(pItem pIter);
//---BST END---///

//---Mutex관련 자료구조, 변수---//
sem_t fileMutex;

typedef struct _fdbuf_t
{
    int *buf;
    int size;
    int front;
    int rear;
    sem_t mutex;
    sem_t push;
    sem_t pop;
} fdbuf_t;

fdbuf_t *fdbuf;

void initFDbuf();
void insertFDbuf(int connfd);
int removeFDbuf();
void freeFDbuf();
//---Mutex END--//

//---client command related function---//
void sig_int_handler(int sig);
void *parsing(void *vargp);
void show(int fd);
void buy(int fd, int ID, int stockNum);
void sell(int fd, int ID, int stockNum);
void clientExit(int fd);
void inorderUpdate(FILE *fp, pItem pIter);
void updateFile();
//---client command end---//

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Enough space for any address */
    // line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 서버가 종료 할 때 수행해야할 명령은 따로 interrup handler를 통해 설정
    // sigint handler등록
    Signal(SIGINT, sig_int_handler);
    // 메모리에 주식목록 로딩
    load_stocks();

    // File을 읽을 때 사용할 mutex 변수 초기화
    Sem_init(&fileMutex, 0, 1);
    // FileDescriptor에 관한 buffer 및 mutex/semaphore변수 초기화
    initFDbuf();
    listenfd = Open_listenfd(argv[1]);

    // Thread를 여러 개 만들어 놓는다(Thread pool)
    for (int i = 0; i < FDBUFSIZE; i++)
        Pthread_create(&tid, NULL, parsing, NULL);

    while (true)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        insertFDbuf(connfd);
    }
}
/* $end server_main */

void *parsing(void *vargp)
{
    int receivedByte;
    char ConvTmp[MAXLINE]; // client로 받은 명령어의 파라미터의 값을 integer로 바꾸기 위한 임시변수
    int ID, stockNum;      // client로부터 받은 주식의 ID와 개수(num)를 저장하는 변수
    char *argv[3];         // 명령어와 파라미터를 저장하는 문자열 배열
    char buf[MAXLINE];
    rio_t rio;

    // 스레드를 독립적으로 동작시키기 위해 사용
    // -> detach로 생성된 스레드는 일이 끝난 후 자원을 스스로 반환한다
    Pthread_detach(pthread_self());

    while (true)
    {
        int connfd = removeFDbuf();

        while (true)
        {
            // sell id number; buy id number; show; exit
            Rio_readinitb(&rio, connfd);
            if ((receivedByte = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
            {
                printf("server received %d bytes\n", receivedByte);
                // 명령어와 파라미터를 구분함
                for (int i = 0; i < 3; i++)
                {
                    if (i == 0)
                        argv[i] = strtok(buf, " ");
                    else
                        argv[i] = strtok(NULL, " ");
                }

                // 파라미터를 string에서 integer로 바꿈
                if (argv[1] != NULL && argv[2] != NULL)
                {
                    strcpy(ConvTmp, argv[1]);
                    ID = atoi(ConvTmp);
                    strcpy(ConvTmp, argv[2]);
                    stockNum = atoi(ConvTmp);
                }

                // client의 command에 따른 동작 수행
                if (!strcmp(argv[0], "show\n"))
                    show(connfd);
                else if (!strcmp(argv[0], "buy"))
                    buy(connfd, ID, stockNum);
                else if (!strcmp(argv[0], "sell"))
                    sell(connfd, ID, stockNum);
                else if (!strcmp(argv[0], "exit\n"))
                {
                    clientExit(connfd);
                    break;
                }
                else
                {
                    Rio_writen(connfd, "type right command line!\n", strlen("type right command line!\n"));
                }
            }
            else
            {
                // 아무 것도 입력되지 않고 client가 종료된 경우
                // 예를 들어 client의 sigint종료 혹은 client 비정상종료
                updateFile();
                Close(connfd);
                break;
            }
        }
    }
}

void load_stocks()
{
    // 주식목록을 stock.txt에서 받아서 tree구조에 저장
    // tree는 bst로 정의된다
    FILE *fp;
    int ID, left_stock, price;

    if ((fp = fopen("stock.txt", "r")) == EOF)
    {
        fprintf(stderr, "File Open Error!\n");
        exit(0);
    }

    while (fscanf(fp, "%d %d %d", &ID, &left_stock, &price) != EOF)
        making_BST(ID, left_stock, price);

    fclose(fp);
}

void making_BST(int ID, int left_stock, int price)
{
    pItem node = (pItem)malloc(sizeof(StockItem));
    pItem p = treeHead;
    pItem parentNode = NULL;

    //--malloc이 제대로 됐는지 확인--//
    if (node == NULL)
    {
        fprintf(stderr, "Memory Allocation Error!\n");
        exit(0);
    }

    //--만든 node에 값을 저장--//
    node->ID = ID;
    node->left_stock = left_stock;
    node->price = price;
    node->readCount = 0;
    node->leftChild = node->rightChild = NULL;
    Sem_init(&(node->mutex), 0, 1);
    Sem_init(&(node->writer), 0, 1);

    //--BST를 ID를 기준으로 ordering한다--//
    if (treeHead == NULL)
        treeHead = node;
    else
    {
        while (true)
        {
            if ((p != NULL) && (node->ID > p->ID))
            {
                parentNode = p;
                p = p->rightChild;
            }
            else if ((p != NULL) && (node->ID < p->ID))
            {
                parentNode = p;
                p = p->leftChild;
            }
            else
            {
                if ((node->ID > parentNode->ID))
                    parentNode->rightChild = node;
                else
                    parentNode->leftChild = node;

                break;
            }
        }
    }
}

void inOrder(char *stockList, pItem pIter)
{
    //-- inOrder는 show명령어를 수행하는데 사용된다. --//
    char tmpList[100];

    if (treeHead == NULL)
    {
        printf("there is no stock_list!\n");
        return;
    }

    //-- recursion을 이용해 inOrder를 구현한다 --//
    if (pIter != NULL)
    {
        inOrder(stockList, pIter->leftChild);

        P(&(pIter->mutex));
        pIter->readCount++;
        if (pIter->readCount == 1)
            P(&(pIter->writer));
        V(&(pIter->mutex));

        //크리티컬 섹션 -> 읽기
        sprintf(tmpList, "%d %d %d ", pIter->ID, pIter->left_stock, pIter->price);
        strcat(stockList, tmpList);
        //크리티컬 섹션 -> 읽기

        P(&(pIter->mutex));
        pIter->readCount--;
        if (pIter->readCount == 0)
            V(&(pIter->writer));
        V(&(pIter->mutex));

        inOrder(stockList, pIter->rightChild);
    }
}

void show(int fd)
{
    // show 명령에 따른 주식목록을 client에게 전송
    char stockList[MAXLINE];
    stockList[0] = '\0';
    inOrder(stockList, treeHead);
    strcat(stockList, "\n");
    Rio_writen(fd, stockList, strlen(stockList));
}

void buy(int fd, int ID, int stockNum)
{
    // buy명령을 통해 메모리에 로딩된 주식 정보를 업데이트
    pItem pIter = treeHead;

    // bst이기 때문에 규칙에 맞게 key값인 ID를 서칭을 한다.
    while (pIter != NULL)
    {
        if (ID == pIter->ID)
            break;
        else if (ID > pIter->ID)
            pIter = pIter->rightChild;
        else
            pIter = pIter->leftChild;
    }

    if (pIter == NULL)
    {
        Rio_writen(fd, "stock doesn't exist\n", strlen("stock doesn't exist\n"));
        return;
    }

    P(&(pIter->writer));
    //--잔여 주식 수와 비교한 후 buy명령을 실행해야 한다--//
    if (pIter->left_stock < stockNum)
        Rio_writen(fd, "Not enough left stock\n", strlen("Not enough left stock\n"));
    else
    {
        pIter->left_stock -= stockNum;
        Rio_writen(fd, "[buy] success\n", strlen("[buy] success\n"));
    }
    V(&(pIter->writer));
}

void sell(int fd, int ID, int stockNum)
{
    // sell명령을 통해 메모리에 로딩된 주식 정보를 업데이트
    pItem pIter = treeHead;

    // bst이기 때문에 규칙에 맞게 key값인 ID를 서칭을 한다.
    while (pIter != NULL)
    {
        if (ID == pIter->ID)
            break;
        else if (ID > pIter->ID)
            pIter = pIter->rightChild;
        else
            pIter = pIter->leftChild;
    }

    if (pIter == NULL)
    {
        Rio_writen(fd, "stock doesn't exist\n", strlen("stock doesn't exist\n"));
        return;
    }

    P(&(pIter->writer));
    pIter->left_stock += stockNum;
    Rio_writen(fd, "[sell] success\n", strlen("[sell] success\n"));
    V(&(pIter->writer));
}

void clientExit(int fd)
{
    //--주식시장 퇴장시 stock.txt에 저장하고 연결을 끊는다.--//
    updateFile();
    Close(fd);
}

void inorderUpdate(FILE *fp, pItem pIter)
{
    //--inorder로 tree를 돌면서 txtfile을 update--//
    if (pIter != NULL)
    {
        inorderUpdate(fp, pIter->leftChild);

        P(&(pIter->mutex));
        pIter->readCount++;
        if (pIter->readCount == 1)
            P(&(pIter->writer));
        V(&(pIter->mutex));

        fprintf(fp, "%d %d %d\n", pIter->ID, pIter->left_stock, pIter->price);

        P(&(pIter->mutex));
        pIter->readCount--;
        if (pIter->readCount == 0)
            V(&(pIter->writer));
        V(&(pIter->mutex));

        inorderUpdate(fp, pIter->rightChild);
    }
}

void updateFile()
{
    P(&fileMutex);
    //--tree를 돌면서 stock.txt에 write를 한다--//
    FILE *fp = fopen("stock.txt", "w");

    if (fp == NULL)
        fprintf(stderr, "can't open stock.txt\n");

    inorderUpdate(fp, treeHead);
    fclose(fp);
    V(&fileMutex);
}

void freeTree(pItem pIter)
{
    // recursion을 이용한 stack을 통해 tree에 할당된 메모리를 해제해준다.
    if (pIter != NULL)
    {
        freeTree(pIter->leftChild);
        freeTree(pIter->rightChild);
        free(pIter);
    }
}

void sig_int_handler(int sig)
{
    /*SIGINT가 오면 tree에 저장된 지금까지의 데이터를
    txtfile에 update하고 모든 동적할당을 해제해준다.
    또한 연결된 file descriptor도 모두 close해준다.*/
    updateFile();
    freeTree(treeHead);
    freeFDbuf();
    exit(0);
}

void initFDbuf()
{
    // fdbuf를 동적할당으로 초기화
    fdbuf = (fdbuf_t *)malloc(sizeof(fdbuf_t));
    fdbuf->buf = (int *)calloc(FDBUFSIZE, sizeof(int));
    fdbuf->size = FDBUFSIZE;
    // 버퍼의 front와 rear index 초기화
    fdbuf->front = fdbuf->rear = 0;
    // 버퍼에 대한 뮤텍스 초기화
    Sem_init(&(fdbuf->mutex), 0, 1);
    // 버퍼에 대한 세마포어 초기화
    Sem_init(&(fdbuf->push), 0, FDBUFSIZE);
    Sem_init(&(fdbuf->pop), 0, 0);
}

void insertFDbuf(int connfd)
{
    P(&(fdbuf->push));
    P(&(fdbuf->mutex));
    fdbuf->buf[++(fdbuf->rear) % FDBUFSIZE] = connfd;
    V(&(fdbuf->mutex));
    V(&(fdbuf->pop));
}

int removeFDbuf()
{
    int removed_fd;

    P(&(fdbuf->pop));
    P(&(fdbuf->mutex));
    removed_fd = fdbuf->buf[++(fdbuf->front) % FDBUFSIZE];
    V(&(fdbuf->mutex));
    V(&(fdbuf->push));

    return removed_fd;
}

void freeFDbuf()
{
    Free(fdbuf->buf);
    Free(fdbuf);
}