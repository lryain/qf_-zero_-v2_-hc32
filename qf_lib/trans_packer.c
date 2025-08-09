#include "trans_packer.h"

#if trans_packer_compile_en

trans_packer_handle_t *trans_packer_creat_trans(size_t name_len_max, size_t data_len_max, trans_look_t en)
{
    trans_packer_handle_t *handle = trans_packer_malloc(sizeof(trans_packer_handle_t));
    if (handle == NULL)
        return NULL;

    memset(handle, 0, sizeof(trans_packer_handle_t));

#if cmd_monitor_en
    handle->rec_buffer_data = NULL;
    handle->rec_buffer_name = NULL;
    if (en & look_cmd)
    {
        if (name_len_max)
        {
            handle->rec_buffer_name = trans_packer_malloc(name_len_max + 1);
            if (handle->rec_buffer_name == NULL)
            {
                trans_packer_free(handle);
                return NULL;
            }
        }

        if (data_len_max)
        {
            handle->rec_buffer_data = trans_packer_malloc(data_len_max + 1);
            if (handle->rec_buffer_data == NULL)
            {
                trans_packer_free(handle);
                if (name_len_max)
                    trans_packer_free(handle->rec_buffer_name);
                return NULL;
            }
        }
    }
#endif

#if string_monitor_en
    handle->rec_buffer_data_s = NULL;
    handle->rec_buffer_name_s = NULL;
    if (en & look_string)
    {
        if (name_len_max)
        {
            handle->rec_buffer_name_s = trans_packer_malloc(name_len_max + 1);
            if (handle->rec_buffer_name_s == NULL)
            {
                trans_packer_free(handle);
#if cmd_monitor_en
                if (en & look_cmd)
                {
                    if (name_len_max)
                        trans_packer_free(handle->rec_buffer_name);
                    if (data_len_max)
                        trans_packer_free(handle->rec_buffer_data);
                }
#endif
                return NULL;
            }
        }

        if (data_len_max)
        {
            handle->rec_buffer_data_s = trans_packer_malloc(data_len_max + 1);
            if (handle->rec_buffer_data_s == NULL)
            {
                trans_packer_free(handle);
                if (name_len_max)
                    trans_packer_free(handle->rec_buffer_name_s);
#if cmd_monitor_en
                if (en & look_cmd)
                {
                    if (name_len_max)
                        trans_packer_free(handle->rec_buffer_name);
                    if (data_len_max)
                        trans_packer_free(handle->rec_buffer_data);
                }
#endif
                return NULL;
            }
        }
    }
#endif

    handle->_write_cb = NULL;
    handle->_name_len_max = name_len_max;
    handle->_data_len_max = data_len_max;

#if (cmd_monitor_en || string_monitor_en)
    handle->head = NULL;
    handle->tail = NULL;
    handle->look_at_en = en;
#endif

    return handle;
}

#if (cmd_monitor_en || string_monitor_en)
void trans_packer_set_look_en(trans_packer_handle_t *handle, trans_look_t en)
{
    if (handle->look_at_en == en)
        return;
#if cmd_monitor_en
    if ((handle->look_at_en & look_cmd) == 0 && (en & look_cmd) == look_cmd)
    {
        if (handle->_name_len_max)
        {
            handle->rec_buffer_name = trans_packer_malloc(handle->_name_len_max + 1);
            if (handle->rec_buffer_name == NULL)
                return;
        }
        if (handle->_data_len_max)
        {
            handle->rec_buffer_data = trans_packer_malloc(handle->_data_len_max + 1);
            if (handle->rec_buffer_data == NULL)
            {
                if (handle->_name_len_max)
                    trans_packer_free(handle->rec_buffer_name);
                return;
            }
        }
    }
    if ((handle->look_at_en & look_cmd) == look_cmd && (en & look_cmd) == 0)
    {
        if (handle->rec_buffer_name != NULL)
        {
            trans_packer_free(handle->rec_buffer_name);
            handle->rec_buffer_name = NULL;
        }
        if (handle->rec_buffer_data != NULL)
        {
            trans_packer_free(handle->rec_buffer_data);
            handle->rec_buffer_data = NULL;
        }
    }
#endif

#if string_monitor_en
    if ((handle->look_at_en & look_string) == 0 && (en & look_string) == look_string)
    {
        if (handle->_name_len_max)
        {
            handle->rec_buffer_name_s = trans_packer_malloc(handle->_name_len_max + 1);
            if (handle->rec_buffer_name_s == NULL)
                return;
        }
        if (handle->_data_len_max)
        {
            handle->rec_buffer_data_s = trans_packer_malloc(handle->_data_len_max + 1);
            if (handle->rec_buffer_data_s == NULL)
            {
                if (handle->_name_len_max)
                    trans_packer_free(handle->rec_buffer_name_s);
                return;
            }
        }
    }
    if ((handle->look_at_en & look_string) == look_string && (en & look_string) == 0)
    {
        if (handle->rec_buffer_name_s != NULL)
        {
            trans_packer_free(handle->rec_buffer_name_s);
            handle->rec_buffer_name_s = NULL;
        }
        if (handle->rec_buffer_data_s != NULL)
        {
            trans_packer_free(handle->rec_buffer_data_s);
            handle->rec_buffer_data_s = NULL;
        }
    }
#endif

    handle->look_at_en = en;
}
#endif

#if trans_packer_debug_log
void trans_packer_set_log_en(trans_packer_handle_t *handle, uint8_t en)
{
    handle->log_out_en = en;
}
#endif

void trans_packer_del_trans(trans_packer_handle_t *handle)
{
#if (cmd_monitor_en || string_monitor_en)
    while (trans_packer_get_pack_num(handle))
        trans_packer_get_pack(handle, NULL, NULL);
#endif
#if cmd_monitor_en
    if (handle->rec_buffer_data != NULL)
        trans_packer_free(handle->rec_buffer_data);
    if (handle->rec_buffer_name != NULL)
        trans_packer_free(handle->rec_buffer_name);
#endif
#if string_monitor_en
    if (handle->rec_buffer_data_s != NULL)
        trans_packer_free(handle->rec_buffer_data_s);
    if (handle->rec_buffer_name_s != NULL)
        trans_packer_free(handle->rec_buffer_name_s);
#endif
    trans_packer_free(handle);
}

void trans_packer_set_write_cb(trans_packer_handle_t *handle, trans_packer_write_cb_t _cb)
{
    handle->_write_cb = _cb;
}

void trans_packer_send_pack(trans_packer_handle_t *handle, const char *name, uint8_t *dat, size_t size)
{
    static uint8_t pack_head[3];
    uint8_t name_len;
    uint16_t crc = 0;
    size_t i;
    size_t _size = size;
    if (handle->_write_cb == NULL)
        return;

    pack_head[0] = trans_head_cmd_0;
    pack_head[1] = trans_head_cmd_1;
    pack_head[2] = trans_head_cmd_2;
    handle->_write_cb((const char *)pack_head, 3);

    name_len = strlen(name);
    handle->_write_cb((const char *)&name_len, 1);
    if (name_len)
        handle->_write_cb(name, name_len);

    if (size == pack_send_log)
        _size = strlen((const char *)dat) + 1;

    pack_head[0] = (_size >> 8) & 0xff;
    pack_head[1] = _size & 0xff;
    handle->_write_cb((const char *)pack_head, 2);

    if (size == pack_send_log)
    {
        if (_size > 0)
            _size--;
        if (_size && dat != NULL)
        {
            handle->_write_cb((const char *)dat, _size);
            handle->_write_cb((const char *)&crc, 1);
        }
    }
    else
    {
        if (_size)
            handle->_write_cb((const char *)dat, _size);
    }

    for (i = 0; i < name_len; i++)
    {
        crc += name[i];
    }
    for (i = 0; i < _size; i++)
    {
        crc += dat[i];
    }

    pack_head[0] = (crc >> 8) & 0xff;
    pack_head[1] = crc & 0xff;
    handle->_write_cb((const char *)pack_head, 2);
}

void trans_packer_send_pack_fmt(trans_packer_handle_t *handle, const char *name, const char *fmt, ...)
{
    va_list aptr;
    size_t num;

    char *printf_buffer = trans_packer_malloc(handle->_name_len_max + handle->_data_len_max);

    va_start(aptr, fmt);
    num = vsnprintf(printf_buffer, handle->_name_len_max + handle->_data_len_max - 1, fmt, aptr);
    va_end(aptr);

    printf_buffer[num++] = '\n';
    if (strcmp(name, "log") == 0)
        num = pack_send_log;

    trans_packer_send_pack(handle, name, (uint8_t *)printf_buffer, num);

    trans_packer_free(printf_buffer);
}

#if (cmd_monitor_en || string_monitor_en)

static void _trans_write_pack_cmd(trans_packer_handle_t *handle)
{
    //printf("c%d\n",sizeof(trans_get_pack_t));
    trans_get_pack_t *tmp = trans_packer_malloc(sizeof(trans_get_pack_t));
    size_t i;
    if (tmp == NULL)
    {
        handle->rec_sta = rec_head;
#if trans_packer_debug_log
        if (handle->log_out_en)
            trans_packer_log_printf_use("trans packer memory malloc faild\n");
        return;
#endif
    }
    if (handle->name_lenth)
    {
        handle->name_lenth += 1;
        //printf("a%d\n",handle->name_lenth);
        tmp->name_str = trans_packer_malloc(handle->name_lenth);
        if (tmp->name_str == NULL)
        {
            trans_packer_free(tmp);
            handle->rec_sta = rec_head;
            return;
        }
        for (i = 0; i < handle->name_lenth; i++)
        {
            tmp->name_str[i] = handle->rec_buffer_name[i];
        }
    }
    else
        tmp->name_str = NULL;
    tmp->_name_lenth = handle->name_lenth;

    if (handle->data_lenth)
    {
        //printf("b%d\n",handle->data_lenth);
        tmp->dat_buffer = trans_packer_malloc(handle->data_lenth);
        if (tmp->dat_buffer == NULL)
        {
            if (handle->name_lenth)
                trans_packer_free(tmp->name_str);
            trans_packer_free(tmp);
            handle->rec_sta = rec_head;
            return;
        }
        for (i = 0; i < handle->data_lenth; i++)
        {
            tmp->dat_buffer[i] = handle->rec_buffer_data[i];
        }
    }
    else
        tmp->dat_buffer = NULL;
    tmp->_data_lenth = handle->data_lenth;

    tmp->next = NULL;

    if (handle->head == NULL)
    {
        handle->head = tmp;
        handle->tail = tmp;
    }
    else
    {
        handle->tail->next = tmp;
        handle->tail = tmp;
    }
    handle->get_pack_size++;
}
#if string_monitor_en
static void _trans_write_pack_string(trans_packer_handle_t *handle)
{
    trans_get_pack_t *tmp = trans_packer_malloc(sizeof(trans_get_pack_t));
    size_t i;
    if (tmp == NULL)
    {
        handle->rec_sta_s = rec_head;
        return;
    }

    if (handle->name_lenth_s)
    {
        handle->name_lenth_s++;
        tmp->name_str = trans_packer_malloc(handle->name_lenth_s);
        if (tmp->name_str == NULL)
        {
            handle->rec_sta_s = rec_head;
            trans_packer_free(tmp);
            return;
        }
        for (i = 0; i < handle->name_lenth_s; i++)
        {
            tmp->name_str[i] = handle->rec_buffer_name_s[i];
        }
#if trans_packer_debug_log
        if (handle->log_out_en)
            trans_packer_log_printf_use("name:%s,over\n", tmp->name_str);
#endif
    }
    else
        tmp->name_str = NULL;
    tmp->_name_lenth = handle->name_lenth_s;

    if (handle->data_lenth_s)
    {
        tmp->dat_buffer = trans_packer_malloc(handle->data_lenth_s);
        if (tmp->dat_buffer == NULL)
        {
            handle->rec_sta_s = rec_head;
            trans_packer_free(tmp);
            if (handle->name_lenth_s)
                trans_packer_free(tmp->name_str);
            return;
        }
        for (i = 0; i < handle->data_lenth_s; i++)
        {
            tmp->dat_buffer[i] = handle->rec_buffer_data_s[i];
        }
    }
    else
        tmp->dat_buffer = NULL;
    tmp->_data_lenth = handle->data_lenth_s;

    tmp->next = NULL;

    if (handle->head == NULL)
    {
        handle->head = tmp;
        handle->tail = tmp;
    }
    else
    {
        handle->tail->next = tmp;
        handle->tail = tmp;
    }
    handle->get_pack_size++;
    handle->rec_sta_s = rec_head;
}

#endif

#if cmd_monitor_en

static void look_at_cmd_trans(trans_packer_handle_t *handle, uint8_t dat)
{

    // 6 134 247 name_len name len_h len_l data crc_h crc_l

    ///////////////////////head///////////////////

    if (dat == trans_head_cmd_0)
    {
        handle->dat_head[0] = dat;
        handle->head_count = 0;
    }
    else if (dat == trans_head_cmd_1)
    {
        if (handle->dat_head[0] == trans_head_cmd_0 && handle->head_count == 1)
            handle->dat_head[1] = dat;
        else
            handle->dat_head[0] = 0;
    }
    else if (dat == trans_head_cmd_2)
    {
        if (handle->head_count == 2 && handle->dat_head[1] == trans_head_cmd_1)
        {
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("uart trans get cmd head\n");
#endif
            handle->rec_sta = rec_name_len;
            return;
        }
        handle->dat_head[1] = 0;
        handle->dat_head[0] = 0;
    }
    else
    {
        handle->dat_head[1] = 0;
        handle->dat_head[0] = 0;
    }
    handle->head_count++;
    ///////////////////////name len///////////////////////////////
    if (handle->rec_sta == rec_name_len)
    {
        handle->name_lenth = dat;
        if (handle->name_lenth > handle->_name_len_max)
        {
            handle->rec_sta = rec_head;
        }
        else if (handle->name_lenth == 0)
        {
            handle->rec_sta = rec_data_len;
            handle->data_count = 4;
        }
        else
        {
            handle->rec_sta = rec_name;
            handle->rec_buffer_name[handle->name_lenth] = 0;
            handle->rec_p = 0;
        }
#if trans_packer_debug_log
        if (handle->log_out_en)
            trans_packer_log_printf_use("name len:%d\n", handle->name_lenth);
#endif
    }
    ///////////////////////name///////////////////////////////
    else if (handle->rec_sta == rec_name)
    {
        handle->rec_buffer_name[handle->rec_p++] = dat;
        if (handle->name_lenth == handle->rec_p)
        {
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("name:%s\n", handle->rec_buffer_name);
#endif
            handle->rec_sta = rec_data_len;
            handle->data_count = 4;
        }
    }
    ///////////////////////data len///////////////////////////////
    else if (handle->rec_sta == rec_data_len)
    {
        if (handle->data_count == 5)
            handle->data_lenth = dat << 8;
        else if (handle->data_count == 6)
        {
            handle->data_lenth |= dat;
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("data len:%d\n", handle->data_lenth);
#endif

            if (handle->data_lenth > handle->_data_len_max)
            {
                handle->rec_sta = rec_head;
            }
            else if (handle->data_lenth == 0)
            {
                handle->rec_sta = rec_crc;
                handle->data_count = 6;
            }
            else
            {
                handle->rec_sta = rec_data;
                handle->rec_p = 0;
            }
        }
    }
    ///////////////////////data///////////////////////////////
    else if (handle->rec_sta == rec_data)
    {
        handle->rec_buffer_data[handle->rec_p++] = dat;
        if (handle->data_lenth == handle->rec_p)
        {
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("data rec done\n");
#endif
            handle->rec_sta = rec_crc;
            handle->data_count = 6;
        }
    }
    ///////////////////////crc///////////////////////////////
    else if (handle->rec_sta == rec_crc)
    {
        if (handle->data_count == 7)
            handle->crc = dat << 8;
        else
        {
            uint16_t crc_tmp = 0;
            size_t i = 0;
            handle->crc |= dat;

            for (i = 0; i < handle->name_lenth; i++)
            {
                crc_tmp += handle->rec_buffer_name[i];
            }

            for (i = 0; i < handle->data_lenth; i++)
            {
                crc_tmp += handle->rec_buffer_data[i];
            }
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("chek crc:%d,%d\n", handle->crc, crc_tmp);
#endif
            if (handle->crc == crc_tmp) // crc ok
            {
                _trans_write_pack_cmd(handle);
#if trans_packer_debug_log
                if (handle->log_out_en)
                    trans_packer_log_printf_use("chek crc pass\n");
#endif
            }
            else // crc err
            {
#if trans_packer_debug_log
                if (handle->log_out_en)
                    trans_packer_log_printf_use("chek crc error\n");
#endif
            }
            handle->rec_sta = rec_head;
            return;
        }
    }

    handle->data_count++;
    return;
}

#endif

#if string_monitor_en

static void look_at_string_trans(trans_packer_handle_t *handle, uint8_t dat)
{
    // 6 134 247 name_len name len_h len_l data crc_h crc_l

    ///////////////////////head///////////////////

    if (dat == trans_head_string_0)
    {
        handle->dat_head_s[0] = dat;
        handle->head_count_s = 0;
    }
    else if (dat == trans_head_string_1)
    {
        if (handle->dat_head_s[0] == trans_head_string_0 && handle->head_count_s == 1)
            handle->dat_head_s[1] = dat;
        else
            handle->dat_head_s[0] = 0;
    }
    else if (dat == trans_head_string_2)
    {
        if (handle->head_count_s == 2 && handle->dat_head_s[1] == trans_head_string_1)
        {
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("uart trans get string head\n");
#endif
            handle->rec_sta_s = rec_name;
            handle->rec_p_s = 0;
            handle->name_lenth_s = 0;
            handle->data_lenth_s = 0;
            return;
        }
        handle->dat_head_s[1] = 0;
        handle->dat_head_s[0] = 0;
    }
    else
    {
        handle->dat_head_s[1] = 0;
        handle->dat_head_s[0] = 0;
    }
    handle->head_count_s++;

    ///////////////////////name///////////////////////////////
    if (handle->rec_sta_s == rec_name)
    {

        if (dat < 32)
        {
            if (dat != '\r' && dat != '\n')
            {
                handle->rec_sta_s = rec_head;
            }
        }

        if (dat == '=')
        {
            handle->rec_buffer_name_s[handle->rec_p_s] = 0;
#if trans_packer_debug_log
            if (handle->log_out_en)
                trans_packer_log_printf_use("name:%s,well rec datas\n", handle->rec_buffer_name_s);
#endif
            handle->rec_sta_s = rec_data;
            handle->rec_p_s = 0;
        }
        else if (dat == '\n' && handle->dat_head_s[2] == '\r')
        {
            handle->rec_buffer_name_s[handle->rec_p_s] = 0;
            _trans_write_pack_string(handle);
        }
        else if (dat != '\r')
        {
            handle->rec_buffer_name_s[handle->rec_p_s++] = dat;
            if (handle->rec_p_s == handle->_name_len_max)
            {
                handle->rec_sta_s = rec_head;
            }
            handle->name_lenth_s++;
        }
    }
    ///////////////////////data///////////////////////////////
    else if (handle->rec_sta_s == rec_data)
    {
        handle->rec_buffer_data_s[handle->rec_p_s] = 0;
        if (dat == '\n' && handle->dat_head_s[2] == '\r')
        {
            _trans_write_pack_string(handle);
        }
        else if (dat != '\r')
        {
            handle->rec_buffer_data_s[handle->rec_p_s++] = dat;
            if (handle->rec_p_s == handle->_data_len_max)
            {
                handle->rec_sta_s = rec_head;
            }
            handle->data_lenth_s++;
        }
    }
    handle->dat_head_s[2] = dat;
}

#endif

void trans_packer_push_byte(trans_packer_handle_t *handle, uint8_t dat)
{
#if cmd_monitor_en
    if (handle->look_at_en & look_cmd)
        look_at_cmd_trans(handle, dat);
#endif
#if string_monitor_en
    if (handle->look_at_en & look_string)
        look_at_string_trans(handle, dat);
#endif
}

size_t trans_packer_get_pack_num(trans_packer_handle_t *handle)
{
    return handle->get_pack_size;
}

size_t trans_packer_get_pack_str_lenth(trans_packer_handle_t *handle)
{
    if (handle->head == NULL)
        return 0;
    return handle->head->_name_lenth;
}

size_t trans_packer_get_pack_data_lenth(trans_packer_handle_t *handle)
{
    if (handle->head == NULL)
        return 0;
    return handle->head->_data_lenth;
}

size_t trans_packer_get_pack(trans_packer_handle_t *handle, const char *name, uint8_t *buffer)
{
    size_t i;
    char *name_s = (char *)name;

    if (handle->head == NULL)
        return 0;

    if (name != NULL)
    {
        for (i = 0; i < handle->head->_name_lenth; i++)
        {
            *name_s++ = handle->head->name_str[i];
        }
    }

    if (buffer != NULL)
    {
        for (i = 0; i < handle->head->_data_lenth; i++)
        {
            *buffer++ = handle->head->dat_buffer[i];
        }
    }
    if (handle->head->name_str != NULL)
        trans_packer_free(handle->head->name_str);
    if (handle->head->dat_buffer != NULL)
        trans_packer_free(handle->head->dat_buffer);

    if (handle->head->next != NULL)
    {
        trans_get_pack_t *tmp = handle->head->next;
        trans_packer_free(handle->head);
        handle->head = tmp;
    }
    else
    {
        trans_packer_free(handle->head);
        handle->head = NULL;
        handle->tail = NULL;
    }

    handle->get_pack_size--;
    return 1;
}
#endif

void trans_packer_string_to_number(char *dat, void *buffer, uint8_t bytes_width, uint8_t dat_width)
{
    uint8_t *bit8 = buffer;
    uint16_t *bit16 = buffer;
    uint32_t *bit32 = buffer;
    uint8_t fu_flg = 0;
    size_t count = 0;
    int32_t tmp = 0;

    if (bytes_width != 1 && bytes_width != 2 && bytes_width != 4)
        return;

    while (*dat)
    {
        if (*dat >= '0' && *dat <= '9')
            break;
        dat++;
    }

    while (*dat)
    {
        if (*dat > '9' || *dat < '0') // another
        {
            if (*dat == '-')
            {
                fu_flg = 1;
                dat++;
                continue;
            }
            else
                goto _save_num;
        }
        tmp *= 10;
        tmp += *dat - 0x30;
        count++;
        if (count == dat_width)
            goto _save_num;

        dat++;
        if (*dat == 0)
            goto _save_num;
        continue;

    _save_num:

        switch (bytes_width)
        {
        case 1:
            if (fu_flg)
                *(int8_t *)bit8 = -tmp;
            else
                *bit8 = tmp;
            bit8++;
            break;

        case 2:
            if (fu_flg)
                *(int16_t *)bit16 = -tmp;
            else
                *bit16 = tmp;
            bit16++;
            break;

        case 4:
            if (fu_flg)
                *(int32_t *)bit32 = -tmp;
            else
                *bit32 = tmp;
            bit32++;
            break;

        default:
            break;
        }
        tmp = 0;
        fu_flg = 0;
        count = 0;
        dat++;
    }
}

#endif
