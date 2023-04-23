#include "csapp.h"

//---주식목록 로딩을 위한 BST관련 자료구조와 함수---//
typedef struct _StockItem *pItem;
typedef struct _StockItem
{
    int ID;
    int left_stock;
    int price;
    pItem leftChild;
    pItem rightChild;
} StockItem;

pItem treeHead;

void load_stocks();
void making_BST(int ID, int left_stock, int price);
void inOrder();
void freeTree(pItem pIter);
//---BST END---///

//---FileDescriptor관리를 위한 자료구조---//
typedef struct _FD_Node *pFD_Node;
typedef struct _FD_Node
{
    int fd;
    pFD_Node nextLink;
} FD_Node;

pFD_Node pFD_head = NULL;
pFD_Node pFD_tail = NULL;
fd_set watch_set, pending_set;

void new_fd(int fd);
void del_fd(int fd);
void free_fd();
//---FileDescriptor End---//

//---client command related function---//
void sig_int_handler(int sig);
void inorderUpdate(FILE *fp, pItem pIter);
void updateFile();
void parsing(int connfd);
void show(int fd);
void buy(int fd, int ID, int stockNum);
void sell(int fd, int ID, int stockNum);
void clientExit(int fd);
//---client command end---//

int main(int argc, char **argv)
{
    /* file descriptor를 event driven 방식에 따라
       바꿔서 처리할 수 있도록 구현 */
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    /* Enough space for any address */
    // line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    // file descriptor관련 변수들
    pFD_Node pIter;
    int fd_range;

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
    FD_ZERO(&watch_set);

    // 만일 client가 접속하면 FileDescriptor List에 추가
    listenfd = Open_listenfd(argv[1]);
    fd_range = listenfd;
    FD_SET(listenfd, &watch_set);
    new_fd(listenfd);

    while (true)
    {
        pending_set = watch_set; // watch_set은 원본이므로 pending_set에 옮겨서 저장
        // FileDescriptor중에 입력가능한 것이 있는 지 감시한다.
        Select(fd_range + 1, &pending_set, NULL, NULL, NULL);

        //  List에 등록된 FileDescriptor를 돌면서 변화를 감지한다.
        for (pIter = pFD_head; pIter != NULL;)
        {
            int currentFD = pIter->fd;
            pFD_Node nextPtr = pIter->nextLink;

            // fd가 설정되어있는데 서버로의 access요청이 아닌 명령을 내리는 경우
            if (FD_ISSET(currentFD, &pending_set) && (currentFD != listenfd))
            {
                parsing(currentFD);
            }
            // fd가 설정되어있고 서버로의 access요청인 경우
            else if (FD_ISSET(currentFD, &pending_set))
            {

                clientlen = sizeof(struct sockaddr_storage);
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
                printf("Connected to (%s, %s)\n", client_hostname, client_port);
                FD_SET(connfd, &watch_set);
                new_fd(connfd);

                if (fd_range < connfd)
                    fd_range = connfd;
            }

            pIter = nextPtr;
        }
    }
}
/* $end server_main */

void parsing(int connfd)
{
    int receivedByte;
    char ConvTmp[MAXLINE]; // client로 받은 명령어의 파라미터의 값을 integer로 바꾸기 위한 임시변수
    int ID, stockNum;      // client로부터 받은 주식의 ID와 개수(num)를 저장하는 변수
    char *argv[3];         // 명령어와 파라미터를 저장하는 문자열 배열
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    // sell id number; buy id number; show; exit
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
            clientExit(connfd);
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
        FD_CLR(connfd, &watch_set);
        del_fd(connfd);
        Close(connfd);
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
    node->leftChild = node->rightChild = NULL;

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
        sprintf(tmpList, "%d %d %d ", pIter->ID, pIter->left_stock, pIter->price);
        strcat(stockList, tmpList);
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

    // buy 명령을 통해 메모리에 로딩된 주식 정보를 업데이트
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

    //--잔여 주식 수와 비교한 후 buy명령을 실행해야 한다--//
    if (pIter->left_stock < stockNum)
        Rio_writen(fd, "Not enough left stock\n", strlen("Not enough left stock\n"));
    else
    {
        pIter->left_stock -= stockNum;
        Rio_writen(fd, "[buy] success\n", strlen("[buy] success\n"));
    }
}

void sell(int fd, int ID, int stockNum)
{
    // sell 명령을 통해 메모리에 로딩된 주식 정보를 업데이트
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

    pIter->left_stock += stockNum;
    Rio_writen(fd, "[sell] success\n", strlen("[sell] success\n"));
}

void inorderUpdate(FILE *fp, pItem pIter)
{
    //--inorder로 tree를 돌면서 txtfile을 update--//
    if (pIter != NULL)
    {
        inorderUpdate(fp, pIter->leftChild);
        fprintf(fp, "%d %d %d\n", pIter->ID, pIter->left_stock, pIter->price);
        inorderUpdate(fp, pIter->rightChild);
    }
}

void updateFile()
{
    //--tree를 돌면서 stock.txt에 write를 한다--//
    FILE *fp = fopen("stock.txt", "w");

    if (fp == NULL)
        fprintf(stderr, "can't open stock.txt\n");

    inorderUpdate(fp, treeHead);
    fclose(fp);
}

void clientExit(int fd)
{
    //--주식시장 퇴장시 stock.txt에 저장하고 연결을 끊는다.--//
    updateFile();
    FD_CLR(fd, &watch_set);
    del_fd(fd);
    Close(fd);
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
    free_fd();
    exit(0);
}

void new_fd(int fd)
{
    // 새로운 client가 access하면 linked list에 fd를 추가함
    // 이것을 통해 추가된 fd는 감시대상이 된다
    pFD_Node node = (pFD_Node)malloc(sizeof(FD_Node));
    if (node == NULL)
    {
        fprintf(stderr, "Memory Allocation Error\n");
        exit(0);
    }

    node->fd = fd;
    node->nextLink = NULL;

    if (pFD_head == NULL)
    {
        pFD_head = node;
        pFD_tail = pFD_head;
    }
    else
    {
        pFD_tail->nextLink = node;
        pFD_tail = pFD_tail->nextLink;
    }
}

void del_fd(int fd)
{
    // client의 exit명령에 따라 fd를 linked list에서 제거
    pFD_Node pFD_Iter = pFD_head;
    pFD_Node pFD_parent = NULL;

    if (pFD_head == NULL)
    {
        printf("\nNo FileDescriptors\n");
        return;
    }

    while (pFD_Iter != NULL)
    {
        if (pFD_Iter->fd == fd)
        {
            if (pFD_parent == NULL)
            {
                pFD_head = pFD_Iter->nextLink;
                free(pFD_Iter);
                // pFD_Iter = pFD_head;
            }
            else
            {
                pFD_parent->nextLink = pFD_Iter->nextLink;
                free(pFD_Iter);
                // pFD_Iter = pFD_parent->nextLink;
            }
            break;
        }
        else
        {
            pFD_parent = pFD_Iter;
            pFD_Iter = pFD_Iter->nextLink;
        }
    }

    // pFD_tail의 위치를 다시 잡아준다.
    pFD_Iter = pFD_head;
    while (pFD_Iter)
    {
        if (pFD_Iter->nextLink == NULL)
            pFD_tail = pFD_Iter;
        pFD_Iter = pFD_Iter->nextLink;
    }
}

void free_fd()
{
    // server의 종료에 따른 linked list 자료구조 해제
    pFD_Node pFreeNode, pFD_Iter = pFD_head;
    pFD_head = pFD_tail = NULL;

    if (pFD_Iter == NULL)
    {
        printf("\nNo FileDescriptor\n");
        return;
    }

    while (pFD_Iter != NULL)
    {
        pFreeNode = pFD_Iter;
        pFD_Iter = pFD_Iter->nextLink;
        free(pFreeNode);
    }
}