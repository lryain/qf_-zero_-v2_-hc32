#include "key_value_transation.h"

#if key_value_transation_compile_en

typedef struct _del_list
{
    key_value_handle_t del_handle;
    struct _del_list *next;
} del_list_t;

static key_value_register_t *register_head = NULL;
static key_value_register_t *register_tail = NULL;
// static uint8_t _busy = 0;

static del_list_t *del_head = NULL;
static del_list_t *del_tail = NULL;
static size_t del_cnt = 0;
static uint8_t _busy_del = 0;

static key_value_register_t *chek_has_register(key_value_handle_t handle);

static int key_cmp(const char *key1, const char *key2)
{
    for (; *key1 == *key2; ++key1, ++key2)
    {
        if (*key1 == '\0')
            return (0);
    }
    return 1;
}

static int get_key_sum(const char *key)
{
    int sum = 0, i;
    for (i = 0; i < strlen(key); i++)
    {
        sum += key[i];
    }
    return sum;
}

static void deL_handle(key_value_handle_t handle)
{
    key_value_register_t *tmp = chek_has_register(handle);
    if (tmp == NULL)
        return;

    if (tmp == register_head)
    {
        if (register_head == register_tail)
        {
            register_head = NULL;
            register_tail = NULL;
        }
        else
            register_head = register_head->next;
    }
    else if (tmp == register_tail)
    {
        register_tail = tmp->last;
        register_tail->next = NULL;
    }
    else
    {
        key_value_register_t *last = tmp->last;
        last->next = tmp->next;
    }

    key_value_free_func((void *)tmp->key);
    key_value_free_func(tmp);
}

static void del_handle_chek()
{
    del_list_t *tmp = del_head;
    del_list_t *move = tmp->next;
    if (del_cnt == 0)
        return;

    while (del_cnt--)
    {
        deL_handle(tmp->del_handle);
        key_value_free_func(tmp);
        tmp = move;
        move = move->next;
    }

    del_head = NULL;
    del_tail = NULL;
}

static key_value_register_t *chek_has_register(key_value_handle_t handle)
{
    key_value_handle_t move = (key_value_handle_t)register_head;

    if (register_head == NULL)
        return NULL;

    for (;;)
    {
        if (handle != move)
            goto _chek_next;

        return move;

    _chek_next:
        if (move->next == NULL)
            return NULL;
        move = move->next;
    }
}

int key_value_msg(const char *key, void *value, size_t lenth)
{
    int ret = 2;
    int sum = 0;
    key_value_register_t *move = register_head;

    // while (_busy)
    //     ;
    // _busy = 1;

    del_handle_chek();

    if (move == NULL)
    {
        //_busy = 0;
        return 1;
    }

    sum = get_key_sum(key);

    for (;;)
    {
        if (move->key_sum != sum)
        {
            goto chek_null;
        }

        if (key_cmp(key, move->key) == 0)
        {
            move->_cb(value, lenth);
            ret = 0;
            break;
        }

    chek_null:
        if (move->next == NULL)
        {
            //_busy = 0;
            return ret;
        }

        move = move->next;
    }

    del_handle_chek();
    //_busy = 0;
    return ret;
}

int key_value_register(key_value_handle_t *handle, const char *key, key_value_cb_t cb)
{
    size_t _len = 0;
    key_value_register_t *tmp = key_value_malloc_func(sizeof(key_value_register_t));

    if (tmp == NULL)
        return 1;

    _len = strlen(key) + 1;
    tmp->key = key_value_malloc_func(_len);
    if (tmp->key == NULL)
    {
        key_value_free_func(tmp);
        return 2;
    }
    memcpy((void *)tmp->key, key, _len);

    tmp->key_sum = get_key_sum(key);
    tmp->next = NULL;
    tmp->last = NULL;
    tmp->_cb = cb;

    // while (_busy)
    //     ;
    // _busy = 1;

    if (handle != NULL)
        *handle = tmp;

    if (register_head == NULL)
    {
        register_head = tmp;
        register_tail = tmp;
    }
    else
    {
        register_tail->next = tmp;
        tmp->last = register_tail;
        register_tail = tmp;
    }

   // _busy = 0;
    return 0;
}

int key_value_del(key_value_handle_t handle)
{
    del_list_t *tmp = key_value_malloc_func(sizeof(del_list_t));

    while (_busy_del)
        ;
    _busy_del = 1;

    if (tmp == NULL)
        return 1;
    tmp->del_handle = handle;
    tmp->next = NULL;

    if (del_tail != NULL)
        del_tail->next = tmp;
    if (del_head == NULL)
        del_head = tmp;
    del_tail = tmp;
    del_cnt++;

    _busy_del = 0;

    return 0;
}

#endif
