#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "util/linkedlist.c"

int main(void)
{
    int i = 0;
    struct node_t head = { count : 0, index : 0, next : NULL, last : NULL };
    struct node_t *last = NULL;
    struct node_t *node = NULL;
    head.last = &head;

    for (; i < 10; i++)
    {
        node_add_last(&head, i + 1);
    }

    printf("count: %d\n", head.count);

    // 使用方法加几个
    node_add_last(&head, 77);
    node_add_last(&head, 88);
    node_add_last(&head, 99);

    printf("count: %d\n", head.count);

    // 遍历整个链表
    last = &head;
    for (; last != NULL; last = last->next)
    {
        printf("node: %d\n", last->index);
    }

    printf("------------------------------\n");

    // 删除表头
    while (1)
    {
        node_remove_first(&head, &node);
        if (node == NULL)
        {
            printf("empty\n");
            break;
        }
        else printf("peek: %d\n", node->index);
        free(node);
    }

    printf("------------------------------\n");

    printf("count: %d\n", head.count);

    last = &head;
    for (; last != NULL; last = last->next)
    {
        printf("node: %d\n", last->index);
    }

    printf("------------------------------\n");

    printf("head->next: %d\n", head.next);
    printf("head->last: %d\n", head.last);
    printf("done\n");

    // 再加一个
    node_add_last(&head, 314);

    // 再遍历一次
    last = &head;
    for (; last != NULL; last = last->next)
    {
        printf("node: %d\n", last->index);
    }

    return 0;
}
