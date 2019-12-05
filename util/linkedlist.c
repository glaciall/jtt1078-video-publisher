/**
 * simple linked-list implements
 */

struct node_t
{
    char data[1024];
    int dataLength;
    int count;
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
    head->count -= 1;
    return 0;
}

int node_add_last(struct node_t *head, char *val, int dataLength)
{
    struct node_t *node = (struct node_t *)malloc(sizeof(struct node_t));
    memset(node->data, 0, 1024);
    int i = 0;
    for (; i < dataLength; i++) node->data[i] = *(val++);
    node->dataLength = dataLength;
    node->next = NULL;

    if (head->last != NULL) head->last->next = node;
    else head->next = node;
    head->last = node;
    head->count += 1;
}