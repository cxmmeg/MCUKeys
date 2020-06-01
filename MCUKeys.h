/*
 * Copyright (c) 2018-2020
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-06-01     xph      the first version
 */
#ifndef _MCUKEYS__H
#define _MCUKEYS__H

#include <stdint.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* 
 * ����ѭ��ɨ������(ms) key_scan()�������ĸ��̶�ɨ�������� ��ֵ�͵��ڶ��� һ������Ϊ10ms
 */
#define KEY_TICKS 10
#define KEY_LIFT (0)  /* ����̧�� */
#define KEY_PRESS (1) /* �������� */

#define KEY_FILTER_TIME 10  // ��������ʱ�� 10ms
#define KEY_LONG_TIME 2000  // ����ʱ�� 2s
#define KEY_REPEAT_TIME 200 // �������200ms
#define KEY_DOUBLE_MIN 100  // ˫��������С��� 100ms
#define KEY_DOUBLE_MAX 300  // ˫����������� 300ms

#define KEY_FIFO_SIZE 50 //����FIFO�����С

//�ϱ��¼���־
#define KEY_REPORT_DOWN (1 << 0)   //�ϱ����������¼�
#define KEY_REPORT_UP (1 << 1)     //�ϱ�����̧���¼�
#define KEY_REPORT_LONG (1 << 2)   //�ϱ������¼�
#define KEY_REPORT_DOUBLE (1 << 3) //�ϱ�˫���¼�
#define KEY_REPORT_REPEAT (1 << 4) //�ϱ������¼�

/* �����¼��ļ���״̬ */
enum
{
    KEY_NONE = 0, /* û�а������� */
    KEY_DOWN,     /* ���������¼� */
    KEY_UP,       /* ����̧���¼� */
    KEY_LONG,     /* ���������¼� */
    KEY_DOUBLE,   /* ����˫���¼� */
};

typedef struct event_key_t
{
    uint8_t id : 5;    /* ���֧��32������ ������������������޸Ĵ�ֵ*/
    uint8_t state : 3; /* ��Ű���״̬ */
} event_key;

//����״̬��
typedef enum _e_key_machine
{
    KEY_MACHINE_NULL, //�ް�������
    KEY_MACHINE_DOWN,
    KEY_MACHINE_DOWN_RECHECK,
    KEY_MACHINE_UP,
    KEY_MACHINE_UP_RECHECK,
    KEY_MACHINE_LONG,
    KEY_MACHINE_REPEAT,

} e_key_machine;

//�����ṹ��
typedef struct _key_t
{
    event_key e_key;                                  /* ��¼����ID��״̬*/
    uint8_t (*get_key_status)(const event_key e_key); /*  �������µ��жϺ��� ��Ӳ����� */
    void (*key_event_cb)(const event_key e_key);      /* �����¼�����ص����� */
    uint16_t long_count;                              /* ���������� */
    uint16_t long_time;                               /* �������³���ʱ��, 0��ʾ����ⳤ�� */
    uint8_t state;                                    /* ������ǰ״̬�����»��ǵ��� */
    uint16_t repeat_speed;                            /* ������������ */
    uint16_t repeat_count;                            /* �������������� */
    uint16_t double_count;                            /* ˫��������*/
    uint8_t report_flag;                              /* �ϱ��¼���־*/
    e_key_machine key_machine;                        /* ����״̬��*/
    uint8_t prev_key_state;                           /* ��һ�ΰ�����״̬ */
    struct _key_t *next;                              /* �������� */

} key;

/* ����FIFO�õ����� */
typedef struct _t_keys_fifo
{
    event_key fifo_buffer[KEY_FIFO_SIZE]; /* ��ֵ������ */
    uint8_t read;                         /* ��������ָ��1 */
    uint8_t write;                        /* ������дָ�� */

} t_keys_fifo;

/***********************************************************************
* �������ƣ� key_scan
* �������ܣ� ����ɨ��
* ��������� ��
* �� ��  ֵ��  ��
* ����˵����  Ӧ�÷���һ��1ms�����ں�����
****************************************************************************/
void key_scan(void);

/***********************************************************************
* �������ƣ� key_add
* �������ܣ� ����ģ���ʼ��
* ���������  ��
* �� ��  ֵ�� ��
* ����˵���� ϵͳ��ʼ��ʱӦ�����ô˺���,�������
****************************************************************************/
int key_add(struct _key_t *curr);

/***********************************************************************
* �������ƣ� key_init
* �������ܣ� ��ʼ������
* ��������� �������
* �� ��  ֵ�� 0 ���ӳɹ� ���� ʧ��
* ����˵���� ������ʼ��ʱӦ�����ô˺���
****************************************************************************/
void key_init(struct _key_t *curr);

/***********************************************************************
* �������ƣ� key_out_fifo
* �������ܣ� �Ӱ���FIFO��ȡ��һ�������¼�
* ���������  ��
* �� ��  ֵ�� ��ǰFIFO�л���İ���ID���¼�
* ����˵������
****************************************************************************/
event_key key_out_fifo(void);

#endif
