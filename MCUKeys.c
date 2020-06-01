/*
 * Copyright (c) 2018-2020
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-06-01     xph      the first version
 */
#include "MCUKeys.h"

/*******************ȫ�ֱ�������***********************/
t_keys_fifo keys_fifo; //��������FIFO
static key *head = NULL;

/**********************************����ʵ��********************************************/

/***********************************************************************
* �������ƣ� key_in_fifo
* �������ܣ� ��һ������״̬����FIFO��
* ���������  ���尴��״̬
* �� ��  ֵ�� ��
* ����˵������
****************************************************************************/
void key_in_fifo(event_key e_key)
{
    keys_fifo.fifo_buffer[keys_fifo.write].id = e_key.id;
    keys_fifo.fifo_buffer[keys_fifo.write].state = e_key.state;
    if (++keys_fifo.write >= KEY_FIFO_SIZE)
    {
        keys_fifo.write = 0;
    }
}

/***********************************************************************
* �������ƣ� key_out_fifo
* �������ܣ� �Ӱ���FIFO��ȡ��һ�������¼�
* ���������  ��
* �� ��  ֵ��  ��ǰFIFO�л���İ����¼�
* ����˵������
****************************************************************************/
event_key key_out_fifo(void)
{
    event_key ret;

    if (keys_fifo.read == keys_fifo.write)
    {
        ret.id = KEY_NONE;
        ret.state = KEY_NONE;
    }
    else
    {
        ret = keys_fifo.fifo_buffer[keys_fifo.read];
        if (++keys_fifo.read >= KEY_FIFO_SIZE)
        {
            keys_fifo.read = 0;
        }
    }
		return ret;
}

/***********************************************************************
* �������ƣ� clear_keys_fifo
* �������ܣ� �������������
* ���������  ��
* �� ��  ֵ��  ��
* ����˵����   ��
****************************************************************************/
void clear_keys_fifo(void)
{
    keys_fifo.read = keys_fifo.write;
}

/***********************************************************************
* �������ƣ� detect_key
* �������ܣ� ���һ��������״̬����״ֵ̬����FIFO��
* ��������� �������
* �� ��  ֵ��  ��
* ����˵����  
****************************************************************************/
static void detect_key(key *handle)
{
    uint8_t current_key_state = handle->get_key_status(handle->e_key); /* ��ȡ����״̬ */
    switch (handle->key_machine)
    {
    case KEY_MACHINE_NULL:
    {
        /* �������� */
        if (current_key_state == KEY_PRESS)
        {
            handle->key_machine = KEY_MACHINE_DOWN;
        }
        /* ˫���¼����� */
        handle->double_count += KEY_TICKS; 
        break;
    }
    case KEY_MACHINE_DOWN:
    {
        /* ���״̬���ڱ��� */
        if (current_key_state == handle->prev_key_state)
        {
            handle->key_machine = KEY_MACHINE_DOWN_RECHECK;
            handle->e_key.state = KEY_DOWN;
            /* ��������˰��������ϱ����� */
            if (handle->report_flag & KEY_REPORT_DOWN) 
            {
                /* ���밴�������¼� */
                key_in_fifo(handle->e_key); 
            }
            /* ���ע���˻ص����� ��ִ�� */
            if (handle->key_event_cb) 
            {
                handle->key_event_cb(handle->e_key);
            }
        }
        else
        {
            handle->key_machine = KEY_MACHINE_NULL;
        }
        break;
    }
    /* �����������Ͱ���̧���ж� */
    case KEY_MACHINE_DOWN_RECHECK:
    {
        /* �������ڱ��ְ���״̬ */
        if (current_key_state == handle->prev_key_state)
        {
            if (handle->long_time > 0)
            {
                if ((handle->long_count += KEY_TICKS) == handle->long_time)
                {
                    handle->e_key.state = KEY_LONG;
                    if (handle->report_flag & KEY_REPORT_LONG)
                    {
                        /* ��ֵ���밴��FIFO */
                        key_in_fifo(handle->e_key); 
                    }
                   /* ���ע���˻ص����� ��ִ�� */
                    if (handle->key_event_cb) 
                    {
                        handle->key_event_cb(handle->e_key);
                    }
                }
                else
                {
                    if (handle->repeat_speed > 0)
                    {
                        if ((handle->repeat_count += KEY_TICKS) >= handle->repeat_speed)
                        {
                            handle->repeat_count = 0;
                            handle->e_key.state = KEY_DOWN;
                            if (handle->report_flag & KEY_REPORT_REPEAT) 
                            {
                                /* ����������ÿ��repeat_speed����1������ */
                                key_in_fifo(handle->e_key);
                            }
                            /* ���ע���˻ص����� ��ִ�� */
                            if (handle->key_event_cb) 
                            {
                                handle->key_event_cb(handle->e_key);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            /* �����Ѿ�̧�� */
            handle->key_machine = KEY_MACHINE_UP;
        }
        break;
    }
    case KEY_MACHINE_UP:
    {
        if (current_key_state == handle->prev_key_state)
        {
            handle->key_machine = KEY_MACHINE_UP_RECHECK;
            handle->long_count = 0;   /* ������������ */
            handle->repeat_count = 0; /* �ظ����ͼ������� */
            handle->e_key.state = KEY_UP;
            if (handle->report_flag & KEY_REPORT_UP)
            {
                /* ���Ͱ�ť�������Ϣ */
                key_in_fifo(handle->e_key);
            }
            /* ���ע���˻ص����� ��ִ�� */
            if (handle->key_event_cb) 
            {
                handle->key_event_cb(handle->e_key);
            }
        }
        else
        {
            handle->key_machine = KEY_MACHINE_DOWN_RECHECK;
        }
        break;
    }
    case KEY_MACHINE_UP_RECHECK:
    {
        /* ����˫��Ҫ�� */
        if ((handle->double_count > KEY_DOUBLE_MIN) && (handle->double_count < KEY_DOUBLE_MAX))
        {
            handle->double_count = 0;
            handle->e_key.state = KEY_DOUBLE;
            if (handle->report_flag & KEY_REPORT_DOUBLE) 
            {
                key_in_fifo(handle->e_key); 
            }
            /* ���ע���˻ص����� ��ִ�� */
            if (handle->key_event_cb) 
            {
                handle->key_event_cb(handle->e_key);
            }
        }
        else
        {
            /* ������˫��Ҫ�� ��������� */
            handle->double_count = 0;
        }
        handle->key_machine = KEY_MACHINE_NULL;
    }
    default:
    {
        break;
    }
    }
    handle->prev_key_state = current_key_state;
}

/***********************************************************************
* �������ƣ� key_scan
* �������ܣ� ����ɨ��
* ��������� ��
* �� ��  ֵ��  ��
* ����˵����  Ӧ�÷���һ���̶������ں�����
****************************************************************************/
void key_scan(void)
{
    struct _key_t *now_key;
    for (now_key = head; now_key != NULL; now_key = now_key->next)
    {
        detect_key(now_key);
    }
}

/***********************************************************************
* �������ƣ� key_add
* �������ܣ� ���Ӱ���
* ��������� �������
* �� ��  ֵ�� 0 ���ӳɹ� ���� ʧ��
* ����˵���� ������ʼ��ʱӦ�����ô˺���
****************************************************************************/
int key_add(struct _key_t *curr)
{
    key *tail = head;
    
    while(tail) 
    {
        if(tail == curr) 
        {
            return -1; /* �Ѿ������� */
        }
        tail = tail->next;
    }
    
    curr->next = head;
    head = curr; /* ͷ�巨 */ 
    
    return 0;
}

/***********************************************************************
* �������ƣ� key_init
* �������ܣ� ��ʼ��������Ĭ�Ϲ���
* ��������� �������
* �� ��  ֵ�� 0 ���ӳɹ� ���� ʧ��
* ����˵���� ������ʼ��ʱӦ�����ô˺���
****************************************************************************/
void key_init(struct _key_t *curr)
{
    curr->e_key.id = 0;
    curr->e_key.state = 0;
    curr->double_count = 0;
    curr->get_key_status = NULL;
    curr->key_event_cb = NULL;
    curr->long_count = 0;
    curr->long_time = KEY_LONG_TIME;
    curr->repeat_count = 0;
    curr->repeat_speed = KEY_REPEAT_TIME;
    curr->state = KEY_LIFT;
    curr->next = NULL;
    curr->report_flag = KEY_REPORT_DOUBLE|KEY_REPORT_DOWN|KEY_REPORT_UP|KEY_REPORT_REPEAT|KEY_REPORT_LONG;
    curr->prev_key_state = KEY_LIFT;
    curr->key_machine = KEY_MACHINE_NULL;

}
/***********************************************************************
* �������ƣ� key_detach
* �������ܣ� ���ٰ���
* ��������� �������
* �� ��  ֵ�� 0 ���ӳɹ� ���� ʧ��
* ����˵���� ����ֻ�ǲ�������� �ڴ沢û���ͷ� �ʵ�ʱ���ǿ��Ե���key_add������ȵ�
*           �������ݷ������Ƿ�����
****************************************************************************/
int key_detach(struct _key_t *curr)
{
	return 0;
}



