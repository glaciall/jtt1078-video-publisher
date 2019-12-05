#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct node_t
{
    int index;
    struct node_t *next;
    struct node_t *last;
};

int node_remove_first(struct node_t *head, struct node_t **node)
{
    *node = head->next;
    if (head->next == NULL) return -1;

    head->next = (*node)->next;
    if ((*node)->next == NULL) head->last = NULL;
    (*node)->next = NULL;
    return 0;
}

int node_add_last(struct node_t *head, int val)
{
    struct node_t *node = (struct node_t *)malloc(sizeof(struct node_t));
    node->index = val;
    node->next = NULL;

    // 记录尾节点的下一个节点
    if (head->last != NULL) head->last->next = node;
    else head->next = node;
    head->last = node;
}

int main(void)
{
    int i = 0;
    struct node_t head = { index : 0, next : NULL, last : NULL };
    struct node_t *last = NULL;
    struct node_t *node = NULL;
    head.last = &head;

    for (; i < 10; i++)
    {
        node_add_last(&head, i + 1);
    }

    // 使用方法加几个
    node_add_last(&head, 77);
    node_add_last(&head, 88);
    node_add_last(&head, 99);

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
