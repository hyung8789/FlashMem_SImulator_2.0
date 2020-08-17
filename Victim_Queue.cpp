#include "Victim_Queue.h"

// Victim Block Queue ������ ���� ���� ť ����

Victim_Queue::Victim_Queue()
{
	this->queue_array = NULL;
	this->queue_size = 0;
	this->front = this->rear = 0;
}

Victim_Queue::Victim_Queue(unsigned int block_size)
{
	this->queue_array = NULL;
	this->queue_size = 0;
	this->init(block_size);
}

Victim_Queue::~Victim_Queue()
{
	if (this->queue_array != NULL)
	{
		delete[] this->queue_array;
		this->queue_array = NULL;
	}
}

void Victim_Queue::init(unsigned int block_size) //Victim Block ������ ���� ť �ʱ�ȭ
{
	if (this->queue_array == NULL)
	{
		this->queue_size = round(block_size * VICTIM_BLOCK_QUEUE_RATIO);
		this->queue_array = new victim_element[queue_size];
	}

	this->front = this->rear = 0;
}

//���� ���� ���� �Լ�
bool Victim_Queue::is_empty()
{
	return (this->front == this->rear); //front�� rear�� ���� ������ �������
}

//��ȭ ���� ���� �Լ�
bool Victim_Queue::is_full()
{
	/* front <- (front+1) % MAX_QUEUE_SIZE
	   rear <- (rear+1) % MAX_QUEUE_SIZE
	   MAX_QUEUE_SIZE�� 5�ϰ�� front�� rear�� ���� 0,1,2,3,4,0�� ���� ��ȭ 
	*/

	return ((this->rear + 1) % this->queue_size == this->front); //������ ��ġ���� front�� ������� ��ȭ����
}

//����ť ���
void Victim_Queue::print()
{
	printf("QUEUE(front = %u rear = %u)\n", this->front, this->rear);
	if (this->is_empty() != true) //ť�� ������� ������
	{
		unsigned int i = this->front;

		do {
			i = (i + 1) % (this->queue_size); //0,1,2,3,4,0�� ���� ��ȭ
			
			switch (this->queue_array[i].is_logical)
			{
			case true:
				std::cout << "victim_LBN : ";
				break;

			case false:
				std::cout << "victim_PBN : ";
				break;
			}
			std::cout << this->queue_array[i].victim_block_num << std::endl;
			std::cout << "victim_block_invalid_ratio : " << this->queue_array[i].victim_block_invalid_ratio << std::endl;
			std::cout << "----------------------------------" << std::endl;

			if (i == this->rear) //rear��ġ���� ���� �� ����
				break;
		} while (i != this->front); //ť�� �� ���� ��������
	}
	printf("\n");
	system("pause");
}

//���� �Լ�
int Victim_Queue::enqueue(victim_element src_data)
{
	if (this->is_full() == true) //���� á����
		return FAIL;

	//rear�� ���� ������Ų�� �����͸� �Է�
	this->rear = (this->rear + 1) % this->queue_size;
	this->queue_array[this->rear] = src_data; //copy value
	
	return SUCCESS;
}

//���� �Լ�
int Victim_Queue::dequeue(victim_element& dst_data)
{
	if (this->is_empty() == true) //���������
		return FAIL;

	//front�� ���� ������Ų�� front��ġ�� �����͸� ����
	this->front = (this->front + 1) % this->queue_size;
	
	dst_data = this->queue_array[this->front]; //dst_PBN���� �� ����

	return SUCCESS;
}

//ť�� �����ϴ� ����� ������ ��ȯ
unsigned int Victim_Queue::get_count()
{
	return (this->rear) - (this->front);
}